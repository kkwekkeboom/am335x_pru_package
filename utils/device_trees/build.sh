dtc -O dtb -o am335x-bone.dtb -b 0 -@ am335x-bone.dts
dtc -O dtb -o am335x-boneblack.dtb -b 0 -@ am335x-boneblack.dts
cp am335x-bone.dtb am335x-boneblack.dtb /boot
