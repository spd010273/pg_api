EXTENSION       = pgapi
EXTVERSION      = $(shell grep default_version $(EXTENSION).control | \
                  sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/" )
DEBUG           = -g -DDEBUG
CC              = gcc
PG_CONFIG       ?= pg_config
PG_INCLUDEDIR   = $(shell $(PG_CONFIG) --includedir)
PG_LIBDIR       = $(shell $(PG_CONFIG) --libdir)
INCLUDE         = -I. -I./lib/ -I$(PG_INCLUDEDIR)
LIBS            = -L$(PG_LIBDIR) -lm -lmicrohttpd -lpq
FLAGS           = $(DEBUG) -O2 -pipe
STRICTURES		= -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector-strong \
			      --param=ssp-buffer-size=4 -grecord-gcc-switches \
				  -Wmissing-prototypes -Wpointer-arith -Wdeclaration-after-statement \
				  -Wendif-labels -Wmissing-format-attribute -Wformat-security \
				  -fno-strict-aliasing -fwrapv -fexcess-precision=standard
ARCHFLAGS		= -m64 -march=native -mtune=native
CFLAGS          = $(INCLUDE) $(LIBS) $(FLAGS) $(STRICTURES) $(ARCHFLAGS)
SRCS            = $(wildcard src/*.c) $(wildcard src/lib/*.c)
OBJS            = $(SRCS:.c=.o)
DATA            = $(wildcard sql/$(EXTENSION)--$(EXTVERSION).sql) $(wildcard sql/$(EXTENSION)--*--$(EXTVERSION).sql)
PG_CPPFLAGS     = $(INCLUDE) $(LIBS) $(DEBUG)
EXTRA_CLEAN     = pg_api
LDFLAGS         = -lm -lpq -lmicrohttpd
PGXS           := $(shell $(PG_CONFIG) --pgxs)

pg_api: $(OBJS)
	$(CC) -o $@ $^ $(STRICTURES) $(DEBUG) -lpq -lmicrohttpd -lm

#.PHONY: clean
#clean:
#	rm -f $(OBJS) pg_api

include $(PGXS)
