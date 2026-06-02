set -e
git submodule init && git submodule update
cd edk2 && git submodule init && git submodule update && cd ..
if [ -z "$EDK_TOOLS_PATH" ]; then
    exec bash -c "
        export PACKAGES_PATH=$PWD/edk2:$PWD
        export WORKSPACE=$PWD
        source $PWD/edk2/edksetup.sh
        exec bash '$0' '$1' '$2'
    " _ "$PAYLOAD_TYPE" "$PLATFORM"
fi
make -C edk2/BaseTools
