

N_TH ?= 1
URAND ?= 1
THETA ?= 0.99

CFLAGS = -O3 -g
CFLAGS += -DN_TH=$(N_TH)
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
