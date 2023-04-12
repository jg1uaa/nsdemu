cmake_minimum_required(VERSION 3.13)

option(PLATFORM "target platform")
if(NOT PLATFORM)
  set(PLATFORM unix) # as default
endif()

if(${PLATFORM} STREQUAL "rp2040")
  set(USE_PICO_SDK ON)
  set(PLATFORM_C platform-rp2040.c)
  set(EXTRA_LIBRARY pico_stdlib pico_rand)
  include(pico-sdk/pico_sdk_init.cmake)
  option(KEYSTR "secret key")
  if(NOT KEYSTR)
    message(FATAL_ERROR "set secret key with -DKEYSTR=nsec1...")
  endif()
elseif(${PLATFORM} STREQUAL "unix")
  set(USE_PICO_SDK OFF)
  set(PLATFORM_C platform-unix.c)
  set(EXTRA_FLAG -DDIAGNOSE)
else()
  message(FATAL_ERROR "set platform with -DPLATFORM=...")
endif()

project(nsdemu)

if(USE_PICO_SDK)
  pico_sdk_init()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

set(SECP256K1_PATH secp256k1)
set(SECP256K1_INCLUDE ../${SECP256K1_PATH}/include)
set(SECP256K1_SRC ${SECP256K1_PATH}/src)

add_library(common STATIC
		   main.c command.c nostr-key.c secure.c)
target_compile_options(common PRIVATE -I${SECP256K1_INCLUDE} ${EXTRA_FLAG})

add_library(secp256k1 STATIC
		      ${SECP256K1_SRC}/secp256k1.c
		      ${SECP256K1_SRC}/precomputed_ecmult.c
		      ${SECP256K1_SRC}/precomputed_ecmult_gen.c)
set(SECP256K1_CFLAGS
	-DECMULT_GEN_PREC_BITS=4 -DECMULT_WINDOW_SIZE=2 -DENABLE_MODULE_ECDH=1
	-DENABLE_MODULE_EXTRAKEYS=1 -DENABLE_MODULE_SCHNORRSIG=1
	-Wno-unused-function)
target_compile_options(secp256k1 PRIVATE ${SECP256K1_CFLAGS})

add_executable(nsdemu ${PLATFORM_C})
if(USE_PICO_SDK)
  target_compile_options(nsdemu PRIVATE -DKEYSTR="${KEYSTR}")
  pico_enable_stdio_usb(nsdemu 1)
  pico_enable_stdio_uart(nsdemu 0)
  pico_add_extra_outputs(nsdemu)
# pico_set_binary_type(nsdemu copy_to_ram)
endif()
target_link_libraries(nsdemu common secp256k1 ${EXTRA_LIBRARY})
