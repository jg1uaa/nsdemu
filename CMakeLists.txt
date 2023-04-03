cmake_minimum_required(VERSION 3.13)
project(nsdemu C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

set(SECP256K1_PATH secp256k1)
set(SECP256K1_INCLUDE ../${SECP256K1_PATH}/include)
set(SECP256K1_SRC ${SECP256K1_PATH}/src)

add_library(common STATIC
		   main.c command.c nostr-key.c secure.c)
target_compile_options(common PRIVATE -I${SECP256K1_INCLUDE})

add_library(secp256k1 STATIC
		      ${SECP256K1_SRC}/secp256k1.c
		      ${SECP256K1_SRC}/precomputed_ecmult.c
		      ${SECP256K1_SRC}/precomputed_ecmult_gen.c)
set(SECP256K1_CFLAGS
	-DECMULT_GEN_PREC_BITS=2 -DECMULT_WINDOW_SIZE=2 -DENABLE_MODULE_ECDH=1
	-DENABLE_MODULE_EXTRAKEYS=1 -DENABLE_MODULE_SCHNORRSIG=1
	-Wno-unused-function)
target_compile_options(secp256k1 PRIVATE ${SECP256K1_CFLAGS})

add_executable(nsdemu platform-unix.c)
target_link_libraries(nsdemu common secp256k1)
