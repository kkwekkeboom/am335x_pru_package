#canbus pinmux
echo 32 > /sys/kernel/debug/omap_mux/uart1_rtsn
echo 32 > /sys/kernel/debug/omap_mux/uart1_ctsn
cat /sys/kernel/debug/omap_mux/uart1_rtsn
cat /sys/kernel/debug/omap_mux/uart1_ctsn

###############################################################################
#								UART 1 pinmuxing 							  #
###############################################################################

echo 45 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio45/direction

###############################################################################

###############################################################################
#								UART 2 pinmuxing 							  #
###############################################################################

echo 26 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio26/direction

###############################################################################

###############################################################################
#								UART 4 pinmuxing 							  #
###############################################################################


echo 46 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio46/direction

###############################################################################

###############################################################################
#								UART 5 pinmuxing 							  #
###############################################################################

# Set UART2 (RTSN)
echo 65 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio65/direction

###############################################################################
#								SPI 5 pinmuxing 							  #
###############################################################################
## NOTE: pinmuxing must be set in the /boot/am335x-bone.dtb file already

## Export pins as GPIO
echo 86 > /sys/class/gpio/export

#slave select
echo 60 > /sys/class/gpio/export
echo 51 > /sys/class/gpio/export
echo 66 > /sys/class/gpio/export
echo 69 > /sys/class/gpio/export

#backplane select lines
echo 50 > /sys/class/gpio/export
echo 44 > /sys/class/gpio/export
echo 67 > /sys/class/gpio/export
echo 68 > /sys/class/gpio/export

ls /sys/class/gpio/ | grep '86\|60\|51\|66\|69\|50\|44\|67\|68'

## Set pin directions
echo out > /sys/class/gpio/gpio86/direction
echo out > /sys/class/gpio/gpio60/direction
echo out > /sys/class/gpio/gpio51/direction
echo out > /sys/class/gpio/gpio66/direction
echo out > /sys/class/gpio/gpio69/direction
echo out > /sys/class/gpio/gpio50/direction
echo out > /sys/class/gpio/gpio44/direction
echo out > /sys/class/gpio/gpio67/direction
echo out > /sys/class/gpio/gpio68/direction

###############################################################################
# Slow PulseCounter IO
###############################################################################

echo 48 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio48/direction
