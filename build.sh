cd build/
cmake .. -DCMAKE_TOOLCHAIN_FILE=D:/Work/obrix/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_INSTALL_PREFIX=/d/libraries/
cmake --build .
cmake --install /d/Work/obrix/FFTWater/build --config Debug
