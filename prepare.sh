set -e
git submodule init && git submodule update
cd edk2 && git submodule init && git submodule update && cd ..
source env.sh
make -C edk2/BaseTools
