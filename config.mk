# See LICENSE file for license and copyright information

# libaries
SQLITE_INC ?= $(shell pkg-config --cflags sqlite3)
SQLITE_LIB ?= $(shell pkg-config --libs sqlite3)

# registry
#
# Make sure that none of the files referenced in REGISTRY_SOURCE contains a
# main function.
REGISTRY_SOURCE = registry/registry.c #$(wildcard registry/*.c) $(wildcard ../reference/registry/*.c)
REGISTRY_INCS   = -I registry
REGISTRY_LIBS   =

# server
#
# Make sure that none of the files referenced in SERVER_SOURCE contains a
# main function.
SERVER_SOURCE = server/database.c server/server.c #$(wildcard server/*.c) $(wildcard ../reference/server/*.c)
SERVER_INCS   = -I server $(SQLITE_INC)
SERVER_LIBS   = $(SQLITE_LIB)

# communication
#
# Make sure that none of the files referenced in COMMUNICATION_SOURCE contains a
# main function.
COMMUNICATION_SOURCE = communication/crypto/sha1.c communication/crypto/hmac.c communication/bpack.c communication/channel-endpoint-connector.c communication/channel-hmac.c communication/channel-with-server.c communication/channel.c communication/datastore.c communication/simple-memory-buffer.c memory.c #$(wildcard communication/*.c) $(wildcard ../reference/communication/*.c)
COMMUNICATION_INCS   = -I communication
COMMUNICATION_LIBS   = $(LIBS)

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
