CFLAGS = -O3 -g

all:
	g++ $(CFLAGS) main.cc nvme.cc
