CFLAGS = -O3 -g

all:
	g++ $(CFLAGS) main.cc nvme.cc


setup:
	sudo modprobe uio_pci_generic
	echo $(PCIADDR) | sudo tee -a /sys/bus/pci/devices/$(PCIADDR)/driver/unbind
	echo uio_pci_generic | sudo tee -a /sys/bus/pci/devices/$(PCIADDR)/driver_override
	echo $(PCIADDR) | sudo tee -a /sys/bus/pci/drivers_probe
reset:
	echo $(PCIADDR) | sudo tee -a /sys/bus/pci/devices/$(PCIADDR)/driver/unbind
	echo nvme | sudo tee -a /sys/bus/pci/devices/$(PCIADDR)/driver_override
	echo $(PCIADDR) | sudo tee -a /sys/bus/pci/drivers_probe
