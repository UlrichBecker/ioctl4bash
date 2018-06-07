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
SOURCES = ioctl_main.c parse_opts.c
EXE_NAME = ioctl

# We need some source files from a other git-repository...
GIT_REPOSITORY_URL = https://raw.githubusercontent.com/UlrichBecker/command_line_option_parser/master/src/

VPATH= $(BASEDIR)
INCDIR = $(BASEDIR)
CFLAGS ?= -g -O0

CC     ?=gcc
PREFIX ?= /usr/local/bin
CFLAGS += $(addprefix -I,$(INCDIR))

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
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE_NAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(EXE_NAME) core
	rmdir $(OBJDIR)

.PHONY: wipe
wipe: clean
	rm parse_opts.*

.PHONY: install
install:
	install -s -D -m 755 $(EXE_NAME) $(PREFIX)

#=================================== EOF ======================================
