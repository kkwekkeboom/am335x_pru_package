PRU_COMPILER_DIR=../../../utils/ti-cgt-pru
PRU_DRIVER_DIR=../../..
PRU_C_FLAGS=--silicon_version=3 --hardware_mac=on -i$(PRU_COMPILER_DIR)/share/ti/cgt-pru/include -i$(PRU_COMPILER_DIR)/share/ti/cgt-pru/lib
PRU_LD_FLAGS=-llibc.a

INCDIR_APP_LOADER=$(PRU_DRIVER_DIR)/pru_sw//app_loader/include
LIBDIR_APP_LOADER=$(PRU_DRIVER_DIR)/pru_sw/app_loader/lib
CROSS_COMPILE=arm-angstrom-linux-gnueabi-

HOST_C_FLAGS += -Wall -g -mtune=cortex-a8 -march=armv7-a -I/usr/local/include -I$(INCDIR_APP_LOADER)
#HOST_LD_FLAGS += $(PRU_DRIVER_DIR)/pru_sw/example_apps/lib/libprussdrv.a
HOST_LD_FLAGS += $(LIBDIR_APP_LOADER)/libprussdrv.a

FIND_ADDRESS_COMMAND=`$(PRU_COMPILER_DIR)/bin/dispru pru.elf| grep _c_int00 | cut -f1 -d\  `

.PHONY: all
all:
	# Compile pru.c into pro.obj
	$(PRU_COMPILER_DIR)/bin/clpru $(PRU_C_FLAGS) -c pru.c

	# Link pru.obj with libraries and output pru.map and pru.elf
	$(PRU_COMPILER_DIR)/bin/clpru $(PRU_C_FLAGS) -z pru.obj $(PRU_LD_FLAGS) \
		-m pru.map -o pru.elf AM3359_PRU.cmd

	# Convert pru.elf into text.bin and data.bin
	$(PRU_COMPILER_DIR)/bin/hexpru bin.cmd pru.elf

	# Find address of start of program and compile host program
	export START_ADDR=0x$(FIND_ADDRESS_COMMAND) && \
	echo "$$START_ADDR" && \
	${CROSS_COMPILE}gcc $(HOST_C_FLAGS) -DSTART_ADDR=`echo $$START_ADDR` -c -o host.o host.c && \
	${CROSS_COMPILE}gcc $(HOST_C_FLAGS) -o host host.o $(HOST_LD_FLAGS)

.PHONY: clean
clean:
	-rm *.obj
	-rm *.map
	-rm *.elf
	-rm *.bin
	-rm *.o
	-rm host