SOURCE_DIR=$1

cp -a $SOURCE_DIR/CMakeLists.txt.unix $SOURCE_DIR/CMakeLists.txt

# Haven't exactly automated much yet...
cmake $SOURCE_DIR
make

# Did the makefile complete?
BUILD_COMPLETE=$?
if [ "$BUILD_COMPLETE" -ne 0 ] ; then
    echo "Build Failed ($BUILD_COMPLETE)"
    exit 1
fi

