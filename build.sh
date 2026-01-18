#!/bin/bash
set -e

if [ -z "$EUC_ENV_READY" ]; then
    echo "ERROR: env.sh has not been sourced."
    echo "Run:  source env.sh"
    exit 1
fi

PAYLOAD_TYPE=$1
BOARD=$2

if [ "$PAYLOAD_TYPE" == "bin" ]; then
    PAYLOAD_FILE="Payloads/stage2.bin"
else
    PAYLOAD_FILE="Payloads/stage2.elf"
fi

build -s -n 0 -a AARCH64 -t CLANGPDB -p EucalyptusPkg/EucalyptusPkg.dsc || exit 1

SIZE=50
dd if=/dev/zero of=eucalyptus.bin bs=1M count=$SIZE
mkfs.vfat -F 32 -n "EUCALYPTUS" eucalyptus.bin

sudo mount -o loop eucalyptus.bin /mnt
sudo mkdir -p /mnt/EFI/BOOT
sudo cp Build/EucalyptusPkg/DEBUG_CLANGPDB/AARCH64/Eucalyptus.efi /mnt/EFI/BOOT/BOOTAA64.EFI
sudo cp "$PAYLOAD_FILE" /mnt

sudo sh -c "cat \"Configs/$BOARD/eucalyptus.cfg\" > \"/mnt/eucalyptus.cfg\""

cd Stage1/$BOARD
make
cd ../..
if [ "$PAYLOAD_TYPE" == "bin" ]; then
    sudo cp "Stage1/$BOARD/stage1.bin" /mnt
else
    sudo cp "Stage1/$BOARD/stage1.elf" /mnt    
fi    
sudo umount /mnt

echo "Build complete for $BOARD with $PAYLOAD_TYPE payload"
