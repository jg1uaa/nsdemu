cmake_minimum_required(VERSION 3.13)

option(PLATFORM "target platform")
if(NOT PLATFORM)
  set(PLATFORM unix) # as default
endif()

option (SECURE_LIBRARY "secure library")
if(NOT SECURE_LIBRARY)
  set(SECURE_LIBRARY secp256k1) # as default
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

set(UBITCOIN_PATH uBitcoin)
set(UBITCOIN_INCLUDE ../${UBITCOIN_PATH}/src)
set(UBITCOIN_SRC ${UBITCOIN_PATH}/src)
set(UBITCOIN_UTILITY_SRC ${UBITCOIN_SRC}/utility)
set(UBITCOIN_TREZOR_SRC ${UBITCOIN_UTILITY_SRC}/trezor)

set(SECP256K1_PATH secp256k1)
set(SECP256K1_INCLUDE ../${SECP256K1_PATH}/include)
set(SECP256K1_SRC ${SECP256K1_PATH}/src)

project(nsdemu)

if(USE_PICO_SDK)
  pico_sdk_init()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} -std=c++17 -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

add_library(common STATIC main.c command.c nostr-key.c)
target_compile_options(common PRIVATE ${EXTRA_FLAG})

# compile both library (not smart, I know)
add_library(secp256k1 STATIC
		      secure-secp256k1.c
		      ${SECP256K1_SRC}/secp256k1.c
		      ${SECP256K1_SRC}/precomputed_ecmult.c
		      ${SECP256K1_SRC}/precomputed_ecmult_gen.c)
set(SECP256K1_CFLAGS
	-DECMULT_GEN_PREC_BITS=4 -DECMULT_WINDOW_SIZE=2 -DENABLE_MODULE_ECDH=1
	-DENABLE_MODULE_EXTRAKEYS=1 -DENABLE_MODULE_SCHNORRSIG=1
	-Wno-unused-function -I${SECP256K1_INCLUDE})
target_compile_options(secp256k1 PRIVATE ${SECP256K1_CFLAGS})

add_library(ubitcoin STATIC
		     secure-ubitcoin.cpp
		     ${UBITCOIN_TREZOR_SRC}/bignum.c
		     ${UBITCOIN_TREZOR_SRC}/ecdsa.c
		     ${UBITCOIN_TREZOR_SRC}/memzero.c
		     ${UBITCOIN_TREZOR_SRC}/ripemd160.c
		     ${UBITCOIN_TREZOR_SRC}/sha2.c
		     ${UBITCOIN_TREZOR_SRC}/secp256k1.c
		     ${UBITCOIN_UTILITY_SRC}/segwit_addr.c
		     ${UBITCOIN_SRC}/BaseClasses.cpp
		     ${UBITCOIN_SRC}/Bitcoin.cpp
		     ${UBITCOIN_SRC}/BitcoinCurve.cpp
		     ${UBITCOIN_SRC}/Conversion.cpp
		     ${UBITCOIN_SRC}/Hash.cpp
		     ${UBITCOIN_SRC}/Networks.cpp)
set(UBITCOIN_CXX_FLAGS -DUSE_STDONLY -I${UBITCOIN_INCLUDE})
target_compile_options(ubitcoin PRIVATE ${UBITCOIN_CXX_FLAGS})

add_executable(nsdemu ${PLATFORM_C})
if(USE_PICO_SDK)
  target_compile_options(nsdemu PRIVATE -DKEYSTR="${KEYSTR}")
  pico_enable_stdio_usb(nsdemu 1)
  pico_enable_stdio_uart(nsdemu 0)
  pico_add_extra_outputs(nsdemu)
# pico_set_binary_type(nsdemu copy_to_ram)
endif()
target_link_libraries(nsdemu common ${SECURE_LIBRARY} ${EXTRA_LIBRARY})
