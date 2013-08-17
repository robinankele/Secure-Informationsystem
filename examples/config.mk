# See LICENSE file for license and copyright information

INCS = -I . -I..
LIBS = -lm ../libregistry.a ../libcommunication.a ../libserver.a -lsqlite3

# compiler
CC ?= gcc

# flags
CFLAGS += -std=c99 -pedantic -Wall -Wextra $(INCS)

# linker flags
LDFLAGS +=

# List all source files here that contain your own test code.
EXAMPLES_SOURCE  = test.c #$(wildcard *.c)

# Set to something != 0 to enable a debug build
DEBUG ?= 1

# Set to something != 0 to enable a coverage analysis support
COVERAGE ?= 0

# Set to something != 0 if you want verbose build output
VERBOSE ?= 0

# Set the target platform. Possible values are i386 (for a 32 bit
# environment) and x86_64 (for a 64 bit environment)
TARGET ?= x86_64

# Please note that if you change COVERAGE or TARGET you have to rebuild
# everything!
