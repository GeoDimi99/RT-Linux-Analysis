CC=gcc
CFLAGS=-O2 -Wall -Wextra
LDFLAGS=-lcurl -lpthread -lrt

all: rt_client

rt_client: rt_client.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f rt_client
