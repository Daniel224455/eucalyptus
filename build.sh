#!/bin/bash
set -e

PAYLOAD_TYPE=$1
PLATFORM=$2

if [ -z "$EDK_TOOLS_PATH" ]; then
    exec bash -c "
        export PACKAGES_PATH=$PWD/edk2:$PWD
        export WORKSPACE=$PWD
        source $PWD/edk2/edksetup.sh
        exec bash '$0' '$1' '$2'
    " _ "$PAYLOAD_TYPE" "$PLATFORM"
fi

build -s -n 0 -a AARCH64 -t CLANGPDB -p EucalyptusPkg/EucalyptusPkg.dsc || exit 1

rm -rf EucalyptusFs
mkdir -p EucalyptusFs/EFI/BOOT
cp Build/EucalyptusPkg/DEBUG_CLANGPDB/AARCH64/Eucalyptus.efi EucalyptusFs/EFI/BOOT/BOOTAA64.EFI
cp Payloads/payload.$PAYLOAD_TYPE EucalyptusFs/
cp Configs/$PLATFORM/eucalyptus.cfg EucalyptusFs/

echo "Build complete for $PLATFORM with $PAYLOAD_TYPE payload"
