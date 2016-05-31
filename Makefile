CC=gcc
CFLAGS+=-Wall -fPIC -std=c99
LDFLAGS+=-shared
SOURCES=lang.c
OBJECTS=$(SOURCES:.c=.o)

OUT=lang.so

all: $(OUT)

$(OUT): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(OUT)
