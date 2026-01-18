# Eucalyptus BIN/ELF Loader
## Demo
[![](https://img.youtube.com/vi/tjcYEm9Epho/maxresdefault.jpg)](https://www.youtube.com/watch?v=tjcYEm9Epho "Demo")

### Getting started 

#### Step 1
First, get a copy of the repo.
```
git clone https://github.com/Daniel224455/eucalyptus
```
then
```
./prepare.sh
```
This will initialize and update all the submodules.

#### Step 2
After that, you need to get valid payloads. <br>
You can use raw UEFI FDs for your device,
or you can use valid ELFs. <br>
Only EDK2 FDs and ELF converted ones were tested. <br>
The payloads need to be dropped off at `Payloads/stage2.bin|elf`.

#### Step 3
Set up the environment to build Eucalyptus
```
source env.sh
```

After that, you can build Eucalyptus for your device.
```
./build.sh [elf|bin] [device name]
```

#### Flashing
> [!WARNING]
> THIS SECTION MAY VARY FOR DIFFERENT DEVICES!

Assuming you have the Debian build for RUBIK Pi 3 installed, <br>
you can flash `eucalyptus.bin` to the partition `efi`.<br>

Example:
```
edl --loader=\\wsl.localhost\Ubuntu-22.04\home\daniel\rubikpi3-bsp\boot_images\boot\QcomPkg\SocPkg\Kodiak\Bin\WP\DEBUG\prog_firehose_ddr.elf \
    w efi \\wsl.localhost\Ubuntu-22.04\home\daniel\eucalyptus\eucalyptus.bin
```

If all goes well in the end, you should be greeted with messages from Eucalyptus<br>
and it loading your payload.

Example:
```
============================
Welcome to Eucalyptus
Universal BIN and ELF loader
============================
Binary build date:
14:12:20 on Jan 18 2026
============================
github.com/Daniel224455
============================
-> running in EL1
-> config: base=0x9FC00000 size=0x300000
-> stage1 mode is ELF
-> image handle found
-> filesystem found on boot device
-> opened root volume
-> stage1.elf found!
-> stage1 size: 67328 bytes (65 KB)
-> allocated buffer at 0x9CB25018
-> successfully read stage1!
-> first 16 bytes (hex): 7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00
-> stage1 buffer at 0x9CB25018
-> ELF entry: 0x9FC00000
-> ELF PHDRs: 1 at 0x40
-> PT_LOAD[0]: paddr=0x9FC00000 vaddr=0x9FC00000 off=0x10000 filesz=0x178 memsz=0x178
-> executing stage1 at 0x9FC00000
-> stage1 executed
-> stage2 mode is BIN
-> image handle found
-> filesystem found on boot device
-> opened root volume
-> stage2 found!
-> stage2 size: 3145728 bytes (3072 KB)
-> allocated buffer at 0x9C419018
-> successfully read stage2!
-> first 16 bytes (hex): 1C 0C 00 14 00 00 00 00 00 00 00 00 00 00 00 00
-> stage2 loaded successfully at 0x9C419018
-> stage2 relocated to 0x9FC00000
-> executing stage2 at 0x9FC00000, good night

Project Silicium for RUBIK Pi 3
Firmware Version 3.1 Build at 13:00:48 on Jan 18 2026

Eucalyptus Loader DEMO
You are running a binary outside of XBL :)
```