PREFIX = /usr/local

CC = gcc
LD = gcc

INCLUDE = -I/opt/etherlab/include
LIB_PATH = -L/opt/etherlab/lib

LIBRARY = -lethercat

CFLAGS = -Wall --std=c99 -g -O2 $(INCLUDE) -D_XOPEN_SOURCE
LDFLAGS = -static -Wall -Wextra $(LIB_PATH) $(LIBRARY)

TARGET = somanet_com_ethercat_red_blue_pill_master_example

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

all: somanet_com_ethercat_red_blue_pill_master_example

$(TARGET): somanet_com_ethercat_red_blue_pill_master_example.o
	$(LD) -o $@ $^ $(LDFLAGS)

.PHONY: clean install

install:
	install $(TARGET) $(PREFIX)/bin

clean:
	rm -rf *.o $(TARGET)
