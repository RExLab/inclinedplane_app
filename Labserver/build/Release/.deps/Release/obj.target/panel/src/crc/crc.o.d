cmd_Release/obj.target/panel/src/crc/crc.o := g++ '-DNODE_GYP_MODULE_NAME=panel' '-DUSING_UV_SHARED=1' '-DUSING_V8_SHARED=1' '-DV8_DEPRECATION_WARNINGS=1' '-D_LARGEFILE_SOURCE' '-D_FILE_OFFSET_BITS=64' '-DBUILDING_NODE_EXTENSION' -I/root/.node-gyp/0.12.6/include/node -I/root/.node-gyp/0.12.6/src -I/root/.node-gyp/0.12.6/deps/uv/include -I/root/.node-gyp/0.12.6/deps/v8/include -I../node_modules/nan -I../src -I../src/uart -I../src/modbus -I../src/crc -I../src/timer  -fPIC -pthread -Wall -Wextra -Wno-unused-parameter -O3 -ffunction-sections -fdata-sections -fno-tree-vrp -fno-omit-frame-pointer -fno-rtti -fno-exceptions -MMD -MF ./Release/.deps/Release/obj.target/panel/src/crc/crc.o.d.raw   -c -o Release/obj.target/panel/src/crc/crc.o ../src/crc/crc.cc
Release/obj.target/panel/src/crc/crc.o: ../src/crc/crc.cc \
 ../src/crc/crc.h ../src/crc/../_config_cpu_.h \
 ../src/crc/../_config_lib_.h ../src/crc/../uc_libdefs.h \
 ../src/crc/../arch.h ../src/crc/../raspberry.h
../src/crc/crc.cc:
../src/crc/crc.h:
../src/crc/../_config_cpu_.h:
../src/crc/../_config_lib_.h:
../src/crc/../uc_libdefs.h:
../src/crc/../arch.h:
../src/crc/../raspberry.h:
