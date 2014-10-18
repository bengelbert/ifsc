
# Example of a pre-build script to run - I'd like to integrate this into the 
# cmake scripts as much as possible (or at least the facility to run scripts)
#.././write_svn_revision.sh

AVR_TARGET_DEVICE=$1
SOURCE_DIR=$2

sed 's/my_device/'${AVR_TARGET_DEVICE}'/' $SOURCE_DIR/CMakeLists.txt.avr > $SOURCE_DIR/CMakeLists.txt

# Haven't exactly automated much yet...
cmake -DCMAKE_TOOLCHAIN_FILE=$SOURCE_DIR./avr-gcc.cmake $SOURCE_DIR
make

# Did the makefile complete?
BUILD_COMPLETE=$?
if [ "$BUILD_COMPLETE" -ne 0 ] ; then
    echo "Build Failed ($BUILD_COMPLETE)"
    exit 1
fi

# I'd really like this to be integrated into the above :)
ImageName=`basename *.elf .elf`
avr-objcopy -j .text -j .data -O ihex $ImageName.elf $ImageName.hex
avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex $ImageName.elf "$ImageName"_eeprom.hex

# Not required for the build process, but works well for an example of a post-build script that
# may need to be run
avr-size --mcu=${AVR_TARGET_DEVICE} -C "$ImageName".elf
