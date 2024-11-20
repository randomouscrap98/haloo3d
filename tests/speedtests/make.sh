# In the future, this may be modified to be a proper makefile
set -x

echo "Making library"
cd ../../
make
cd -

build() {
  gcc -O2 -flto $1.c ../../build/haloo3d.a -o $1.exe -lm
}

echo "Building tests"
build "simple2dtest"
build "simple3dtest"

echo "Done!"
