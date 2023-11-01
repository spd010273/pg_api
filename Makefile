DEBUG           = -g -DDEBUG
CC              = gcc
PG_CONFIG       ?= pg_config
PG_INCLUDEDIR   = $(shell $(PG_CONFIG) --includedir)
PG_LIBDIR       = $(shell $(PG_CONFIG) --libdir)
INCLUDE         = -I. -I./lib/ -I$(PG_INCLUDEDIR)
LIBS            = -L$(PG_LIBDIR)
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
LDFLAGS         = -lm -lpq

pg_api: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJS) pg_api
