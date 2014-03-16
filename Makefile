CFLAGS:=-Wall

all: main.c
	$(CC) $(CFLAGS) $^ -static -Ilib/ -Llib/ -liowkit -lpthread -o hyt271
