modprobe uio_pruss
echo "BB-SPIDEV9" > /sys/devices/bone_capemgr.9/slots
echo "BB-BONE-PRU-01" > /sys/devices/bone_capemgr.9/slots
cat /sys/devices/bone_capemgr.9/slots
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/root/am335x_pru_package/pru_sw/app_loader/lib