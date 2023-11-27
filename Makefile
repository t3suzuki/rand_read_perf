CFLAGS = -I /home/tomoya-s/hashmaps/libcuckoo

all:
	g++ -O3 -g main.cc nvme.cc $(CFLAGS)
