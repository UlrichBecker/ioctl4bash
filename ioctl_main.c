/*****************************************************************************/
/*                                                                           */
/*!  @brief Wrapper-program to perform of the C-function ioctl() via bash    */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*! @file     ioctl_main.c                                                   */
/*! @author   Ulrich Becker www.INKATRON.de                                  */
/*! @date     20.03.2017                                                     */
/*! Revision:                                                                */
/*****************************************************************************/
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "parse_opts.h"


#ifndef MAX_PIPE_SIZE_INFO
   #define MAX_PIPE_SIZE_INFO "/proc/sys/fs/pipe-max-size"
#endif

#define FD_IN  0
#define FD_OUT 1

#ifndef STATIC_ASSERT
  #define __STATIC_ASSERT__( condition, line ) \
       extern char static_assertion_on_line_##line[2*((condition)!=0)-1];

  #define STATIC_ASSERT( condition ) __STATIC_ASSERT__( condition, __LINE__)
#endif

typedef struct
{
   char*   filename;
   bool    verbose;
   ssize_t size;
   long    cmd;
   void*   pValue;
} IOCTL_T;

/*!----------------------------------------------------------------------------
*/
ssize_t getMaxPipeSize( void )
{
   ssize_t ret;
   int fd;
   char *pEnd;
   char buffer[16];
   const char* pipeSizeInfoFilename = MAX_PIPE_SIZE_INFO;

   fd = open( pipeSizeInfoFilename, O_RDONLY );
   if( fd < 0 )
   {
      fprintf( stderr,
               "ERROR: Could not open \"%s\" : %s\n",
               pipeSizeInfoFilename,
               strerror( errno ) );
      return -1;
   }

   ret = read( fd, &buffer, sizeof( buffer ) );
   close( fd );

   if( ret <= 0 )
   {
      fprintf( stderr,
               "ERROR: Could not correct read from \"%s\" : %s\n",
               pipeSizeInfoFilename,
               strerror( errno ) );
      return -1;
   }

   ret = strtoll( buffer, &pEnd, 10 );
   if( *pEnd != '\n' )
   {
      fprintf( stderr,
               "ERROR: Content of \"%s\" seems not to be a decimal number: \"%s\"\n",
               pipeSizeInfoFilename,
               buffer );
      return -1;
   }

   return ret;
}

/*!----------------------------------------------------------------------------
*/
static int readNumber( long* pNum, const char* str )
{
   char* pEnd;
   size_t l = strlen( str );

   if( l >= 2 && str[0] == '0' && str[1] == 'x' )
      *pNum = strtoll( &str[2], &pEnd, 16 );
   else
      *pNum = strtoll( str, &pEnd, 10 );

   return (*pEnd == '\0')? l : -1;
}

/*!----------------------------------------------------------------------------
*/
static void printHelp( FILE* pStream, const char* pName, struct OPTION_BLOCK_T pBl[] )
{
   fprintf( stdout,
            "Usage: %s [options] <device-file> <command> [parameter]\nOptions:\n",
            pName );
   printOptionList( pStream, pBl );
}

/*!----------------------------------------------------------------------------
*/
#define FSM_TRANSITION( newStade, attr... ) state = newStade
#define FSM_INIT_FSM( startState, attr... ) INPUT_FSM state = startState
#define FSM_DECLARE_STATE( state, attr... ) state

static int parseCommandLine( IOCTL_T* pData, int argc, char** ppArgv )
{
   int i;
   bool cmdSet = false;
   int ret = 0;
   struct OPTION_BLOCK_T blockList[] =
   {
      {
         OPT_LAMBDA( pArg,
         {
            printHelp( stdout, pArg->ppAgv[0], pArg->pOptBlockList );
            exit( EXIT_SUCCESS );
            return 0;
         }),
         .shortOpt    = 'h',
         .longOpt     = "help",
         .helpText    = "Print this help and exit"
      },
      {
         OPT_LAMBDA( pArg,
         {
            if( ((IOCTL_T*)pArg->pUser)->size != 0 )
            {
               fprintf( stderr, "ERROR: Option " );
               printOption( stderr, pArg->pCurrentBlock );
               fprintf( stderr, " already set!\n" );
               return -1;
            }
            if( ((IOCTL_T*)pArg->pUser)->pValue != NULL )
            {
               fprintf( stderr,
                        "ERROR: Third parameter is given: 0x%0X, %d, "
                        "therefore the pipe-mode isn't permit!\n",
                        ((IOCTL_T*)pArg->pUser)->pValue,
                        ((IOCTL_T*)pArg->pUser)->pValue
                      );
               return -1;
            }
            if( pArg->optArg != NULL )
            {
               STATIC_ASSERT( sizeof( long ) <= sizeof( ((IOCTL_T*)pArg->pUser)->size ) );
               if( readNumber( (long*)&((IOCTL_T*)pArg->pUser)->size, pArg->optArg ) < 0 )
               {
                  fprintf( stderr, "ERROR: Invalid argument \"%s\"\n", pArg->optArg );
                  printOption( stderr, pArg->pCurrentBlock );
                  return -1;
               }
               return 0;
            }
            ((IOCTL_T*)pArg->pUser)->size = getMaxPipeSize();
            if( ((IOCTL_T*)pArg->pUser)->size < 0 )
               return -1;
            return 0;
         }),
         .hasArg      = OPTIONAL_ARG,
         .shortOpt    = 'p',
         .longOpt     = "pipe-size",
         .helpText    = "Enables the pipe-mode,\n"
                        "The optional argument PARAM determines the buffer-size in bytes.\n"
                        "If the argument is not given, so the buffer-size will be the\n"
                        "maximum size defined in \"" MAX_PIPE_SIZE_INFO "\"."
      },
      {
         OPT_LAMBDA( pArg,
         {
            ((IOCTL_T*)pArg->pUser)->verbose = true;
            return 0;
         }),
         .shortOpt    = 'v',
         .longOpt     = "verbose",
         .helpText    = "Be verbose"
      },
      OPTION_BLOCKLIST_END_MARKER
   };

   typedef enum
   {
      FSM_DECLARE_STATE( READ_FILE, label = 'read file name' ),
      FSM_DECLARE_STATE( READ_CMD,  label = 'read ioctl-command' ),
      FSM_DECLARE_STATE( READ_VALUE, label = 'read value of ioctl-command' ),
      FSM_DECLARE_STATE( READY )
   } INPUT_FSM; 

   FSM_INIT_FSM( READ_FILE );

   memset( pData, 0, sizeof(IOCTL_T) );
   for( i = 1; i < argc; i++ )
   {
      i = parseCommandLineOptionsAt( i, argc, ppArgv, blockList, pData );
      if( i < 0 )
         return -1;
      if( i == argc )
         break;

      switch( state )
      {
         case READ_FILE:
         {
            pData->filename = ppArgv[i];
            FSM_TRANSITION( READ_CMD );
            break;
         }
         case READ_CMD:
         {
            STATIC_ASSERT( sizeof( long ) <= sizeof( pData->cmd ) );
            if( readNumber( &pData->cmd, ppArgv[i] ) < 0 )
            {
               fprintf( stderr,
                        "ERROR: Can't read number of \"%s\"\n",
                        ppArgv[i] );
               printHelp( stderr, ppArgv[0], blockList );
               return -1;
            }
            cmdSet = true;
            FSM_TRANSITION( READ_VALUE );
            break;
         }
         case READ_VALUE:
         {
            if( pData->size != 0 )
            {
               fprintf( stderr,
                        "ERROR: Third parameter \"%s\" not allowed in pipe-mode!\n",
                        ppArgv[i] );
               return -1;
            }
            STATIC_ASSERT( sizeof( long ) <= sizeof( pData->pValue ) );
            if( readNumber( (long*)&pData->pValue, ppArgv[i] ) < 0 )
            {
               fprintf( stderr,
                        "ERROR: Can't read number of \"%s\"\n",
                        ppArgv[i] );
               printHelp( stderr, ppArgv[0], blockList );
               return -1;
            }
            FSM_TRANSITION( READY );
            break;
         }
         case READY:
         {
            fprintf( stderr,
                     "ERROR: To much parameters!\n" );
            printHelp( stderr, ppArgv[0], blockList );
            return -1;
         }
         default: assert( false ); break;
      };
   }

   if( pData->filename == NULL )
   {
      fprintf( stderr, "ERROR: Missing device-filename!\n" );
      return -1;
   }
   if( !cmdSet )
   {
      fprintf( stderr, "ERROR: Missing command!\n" );
      return -1;
   }
   return ret;
}

/*!----------------------------------------------------------------------------
*/
static int doIoctl( const IOCTL_T* pData )
{
   int ret= 0;
   int fd;

   fd = open( pData->filename, O_RDWR );
   if( fd < 0 )
   {
      if( errno == EPERM )
      {
         if( pData->verbose )
            fprintf( stdout,
                     "WARNING: File %s not writable, trying to open as read-only.\n",
                     pData->filename
                   );
         fd = open( pData->filename, O_RDONLY );
      }
      if( fd < 0 )
      {
         fprintf( stderr,
                  "ERROR: Can not open file %s : %s\n",
                  pData->filename,
                  strerror( errno ) );
         return -1;
      }
   }

   ret = ioctl( fd, pData->cmd, pData->pValue );
   close( fd );

   if( ret != 0 )
   {
      fprintf( stderr,
               "ERROR: ioctl returns with %d -> %s\n",
               ret,
               strerror( errno ) );
      return ret;
   }

   if( pData->verbose )
      fprintf( stdout, "Success!\n" );

   return 0;
}

/*!----------------------------------------------------------------------------
*/
int main( int argc, char** ppArgv )
{
   IOCTL_T oIoctl;
   int ret;
   fd_set  readFds;
   struct timeval oTimeout = { 0, 0 };

   if( parseCommandLine( &oIoctl, argc, ppArgv ) < 0 )
      return EXIT_FAILURE;

   if( oIoctl.size != 0 )
   {
      oIoctl.pValue = malloc( oIoctl.size );
      if( oIoctl.pValue == NULL )
      {
         fprintf( stderr,
                  "ERROR: Unable to allocate %d bytes: %s\n",
                  oIoctl.size,
                  strerror( errno ) );
         return EXIT_FAILURE;
      }
      memset( oIoctl.pValue, 0, oIoctl.size );
   }

   if( oIoctl.verbose )
   {
      printf( "File:        %s\n", oIoctl.filename );
      printf( "Command:     0x%08X %d\n", oIoctl.cmd, oIoctl.cmd );
      if( oIoctl.size != 0 )
         printf( "Buffer-size: %d bytes, address: %p\n", oIoctl.size, oIoctl.pValue );
      else
         printf( "Value:       0x%08X %d\n", oIoctl.pValue, oIoctl.pValue );
   }

   if( oIoctl.size != 0 )
   {
      FD_ZERO( &readFds );
      FD_SET( FD_IN, &readFds );
      if( select( FD_IN+1, &readFds, NULL, NULL, &oTimeout ) > 0 )
         read( FD_IN, oIoctl.pValue, oIoctl.size );
   }

   ret = doIoctl( &oIoctl );

   if( oIoctl.size != 0 )
   {
      if( ret >= 0 )
         write( FD_OUT, oIoctl.pValue, oIoctl.size );

      free( oIoctl.pValue );
   }

   return (ret < 0)? EXIT_FAILURE : EXIT_SUCCESS;
}
/*================================== EOF ====================================*/
