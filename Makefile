CFLAGS:=-Wall

all: main.c
	$(CC) $(CFLAGS) $^ -static -Ilib/ -Llib/ -liowkit -lpthread -lm -o hyt271
