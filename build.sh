

rm -rf output
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make install
