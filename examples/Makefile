# See LICENSE file for license and copyright information

include config.mk
include common.mk

DFLAGS ?= -g

ifneq (${DEBUG},0)
CFLAGS += ${DFLAGS}
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

PROJECT = example
OBJECTS = $(patsubst %.c, %.o,    $(EXAMPLES_SOURCE))
GCDA    = $(patsubst %.c, %.gcda, $(EXAMPLES_SOURCE))
GCNO    = $(patsubst %.c, %.gcno, $(EXAMPLES_SOURCE))

all: options ${PROJECT}

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

${PROJECT}: ${OBJECTS}
	$(ECHO) CC -o $@
	$(QUIET)${CC} ${SFLAGS} ${LDFLAGS} -o $@ ${OBJECTS} ${LIBS}

run:
	${QUIET}./${PROJECT}

clean:
	$(QUIET)rm -rf ${PROJECT} ${OBJECTS} ${GCNO} ${GCDA}

-include $(wildcard .depend/*.dep)

.PHONY: all options ${PROJECT} doc clean
