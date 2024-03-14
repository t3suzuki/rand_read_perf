

N_CORE ?= 1
URAND ?= 0
THETA ?= 0.99

CFLAGS = -O3 -g
CFLAGS += -DN_CORE=$(N_CORE)
CFLAGS += -DURAND=$(URAND)
CFLAGS += -DTHETA=$(THETA)

all:
	g++ $(CFLAGS) main.cc nvme.cc -o my
write:
	g++ $(CFLAGS) write.cc nvme.cc -o write


setup:
	sudo modprobe uio_pci_generic
	echo $(PCIADDR) | sudo tee -a /sys/bus/pci/devices/$(PCIADDR)/driver/unbind
	echo uio_pci_generic | sudo tee -a /sys/bus/pci/devices/$(PCIADDR)/driver_override
	echo $(PCIADDR) | sudo tee -a /sys/bus/pci/drivers_probe
reset:
	echo $(PCIADDR) | sudo tee -a /sys/bus/pci/devices/$(PCIADDR)/driver/unbind
	echo nvme | sudo tee -a /sys/bus/pci/devices/$(PCIADDR)/driver_override
	echo $(PCIADDR) | sudo tee -a /sys/bus/pci/drivers_probe

clean:
	rm -f write *.o my *~
