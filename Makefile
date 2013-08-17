# See LICENSE file for license and copyright information

include config.mk
include common.mk

INCS = -I .
LIBS = -lm
CFLAGS += -std=c99 -pedantic -Wall -Wno-format-zero-length -Wextra $(INCS)
DFLAGS ?= -g

ifneq (${DEBUG},0)
CFLAGS += ${DFLAGS}
else
CPPFLAGS += -DNDEBUG
endif

ifneq ($(COVERAGE),0)
CFLAGS  += -fprofile-arcs -ftest-coverage
LDFLAGS += -fprofile-arcs -ftest-coverage
LIBS    += -lgcov
endif

ifeq ($(TARGET),i386)
CFLAGS  += -m32
LDFLAGS += -m32
else
ifeq ($(TARGET),x86_64)
CFLAGS  += -m64
LDFLAGS += -m64
else
$(error "Unkown target platform")
endif
endif

ifeq ($(strip $(REGISTRY_SOURCE)), )
$(error "No registry source files specified")
endif

ifeq ($(strip $(SERVER_SOURCE)), )
$(error "No server source files specified")
endif

ifeq ($(strip $(COMMUNICATION_SOURCE)), )
$(error "No communication source files specified")
endif

REGISTRY = registry
REGISTRY_OBJECTS = $(patsubst %.c, %.o, $(REGISTRY_SOURCE))
REGISTRY_GCDA = $(patsubst %.c, %.gcda, $(REGISTRY_SOURCE))
REGISTRY_GCNO = $(patsubst %.c, %.gcno, $(REGISTRY_SOURCE))

SERVER = server
SERVER_OBJECTS = $(patsubst %.c, %.o, $(SERVER_SOURCE))
SERVER_GCDA = $(patsubst %.c, %.gcda, $(SERVER_SOURCE))
SERVER_GCNO = $(patsubst %.c, %.gcno, $(SERVER_SOURCE))

COMMUNICATION = communication
COMMUNICATION_OBJECTS = $(patsubst %.c, %.o, $(COMMUNICATION_SOURCE))
COMMUNICATION_GCDA = $(patsubst %.c, %.gcda, $(COMMUNICATION_SOURCE))
COMMUNICATION_GCNO = $(patsubst %.c, %.gcno, $(COMMUNICATION_SOURCE))

all: options ${REGISTRY} ${SERVER} ${COMMUNICATION}

options:
	@echo build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LIBS    = ${LIBS}"
	@echo "CC      = ${CC}"
	@echo "DEBUG   = ${DEBUG}"
	@echo "COVERGE = ${COVERAGE}"
	@echo "TARGET  = ${TARGET}"

%.o: %.c
	$(ECHO) CC $<
	@mkdir -p .depend/$(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF .depend/$(abspath $@).dep

${SERVER_OBJECTS}: config.mk
${REGISTRY_OBJECTS}: config.mk
${COMMUNICATION_OBJECTS}: config.mk

${REGISTRY}: lib${REGISTRY}.a
${SERVER}: lib${SERVER}.a
${COMMUNICATION}: lib${COMMUNICATION}.a

lib${REGISTRY}.a: ${REGISTRY_OBJECTS}
	$(ECHO) AR rcs $@
	$(QUIET)ar rcs $@ ${REGISTRY_OBJECTS}

lib${SERVER}.a: ${SERVER_OBJECTS}
	$(ECHO) AR rcs $@
	$(QUIET)ar rcs $@ ${SERVER_OBJECTS}

lib${COMMUNICATION}.a: ${COMMUNICATION_OBJECTS}
	$(ECHO) AR rcs $@
	$(QUIET)ar rcs $@ ${COMMUNICATION_OBJECTS}

doc:
	$(QUIET)doxygen Doxyfile

clean:
	$(QUIET)rm -rf ${REGISTRY_OBJECTS} ${SERVER_OBJECTS} ${COMMUNICATION_OBJECTS} \
		lib${REGISTRY}.a lib${SERVER}.a lib${COMMUNICATION}.a doc \
		${COMMUNICATION_GCDA} ${COMMUNICATION_GCNO} \
		${SERVER_GCDA} ${SERVER_GCNO} \
		${REGISTRY_GCDA} ${REGISTRY_GCNO}

-include $(wildcard .depend/*.dep)

.PHONY: all options ${REGISTRY} ${SERVER} ${COMMUNICATION} doc clean
