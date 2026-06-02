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
After that, you need to get a valid payload. <br>
You can use raw UEFI FDs for your device,
or you can use valid ELFs. <br>
Only EDK2 FDs and ELF converted ones were tested. <br>
The payloads need to be dropped off at `Payloads/payload.bin|elf`.

#### Step 3

After that, you can build Eucalyptus for your device.
```
./build.sh [elf|bin] [platform]
```

#### Usage
Copy the contents of `EucalyptusFs` into a FAT32-formatted (or other FS, depending on
your driver situation) storage device
