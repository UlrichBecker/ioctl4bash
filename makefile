###############################################################################
##                                                                           ##
##          Makefile for bash wrapper of the C-function ioctl()              ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:   ~/ioctl/src/makefile                                              ##
## Author: Ulrich Becker                                                     ##
## Date:   22.03.2016                                                        ##
###############################################################################
BASEDIR ?= .
EXE_NAME ?= ioctl
SOURCES = ioctl_main.c parse_opts.c

# We need some source files from a other git-repository...
GIT_REPOSITORY_URL = https://raw.githubusercontent.com/UlrichBecker/command_line_option_parser/master/src/

# At the moment this program doesn't have a commandline-option with
# needs obligatory argument, so we can reduce the size of the binary:
PRE_DEFINES = CONFIG_CLOP_NO_REQUIRED_ARG

VPATH  = $(BASEDIR)
INCDIR = $(BASEDIR)
CFLAGS ?= -g -O0

CC     ?= gcc
PREFIX ?= /usr/local/bin
_CFLAGS += $(CFLAGS)
_CFLAGS += $(addprefix -I,$(INCDIR))
_CFLAGS += $(addprefix -D,$(PRE_DEFINES))

OBJDIR=.obj


OBJ = $(addprefix $(OBJDIR)/,$(addsuffix .o,$(notdir $(basename $(SOURCES)))))

.PHONY: all 
all: $(EXE_NAME)

parse_opts.h:
	wget $(GIT_REPOSITORY_URL)parse_opts.h

parse_opts.c:
	wget $(GIT_REPOSITORY_URL)parse_opts.c

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c $(SOURCES) $(OBJDIR) parse_opts.h
	$(CC) -c -o $@ $< $(_CFLAGS)

$(EXE_NAME): $(OBJ)
	$(CC) -o $@ $^ $(_CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(EXE_NAME) core
	rmdir $(OBJDIR)

.PHONY: wipe
wipe: clean
	rm parse_opts.*

.PHONY: install
install:
	mkdir -p $(PREFIX)
	install -s -D -m 755 $(EXE_NAME) $(PREFIX)

#=================================== EOF ======================================
