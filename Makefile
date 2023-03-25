# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2023 SASANO Takayoshi <uaa@uaa.org.uk>

TARGET = nsdemu
OBJS = command.o random.o serial.o secure.o nostr-key.o main.o
CFLAGS = -fdata-sections -ffunction-sections -Wall -Os

SECP256K1_PATH = ./secp256k1
SECP256K1_OBJS = secp256k1.o precomputed_ecmult.o precomputed_ecmult_gen.o
SECP256K1_SRC_PATH = $(SECP256K1_PATH)/src
SECP256K1_INCLUDE_PATH = $(SECP256K1_PATH)/include

# reduce secp256k1_ecmult_gen_prec_table to 32k, ECMULT_GEN_PREC_BITS=2
# (default 4)
# secp256k1_pre_g and secp256k1_pre_g_128 is not used, ECMULT_WINDOW_SIZE
# is minimal (default 15)
SECP256K1_CFLAGS = -DECMULT_GEN_PREC_BITS=2 -DECMULT_WINDOW_SIZE=2 \
	-DENABLE_MODULE_ECDH=1 -DENABLE_MODULE_EXTRAKEYS=1 \
	-DENABLE_MODULE_SCHNORRSIG=1 \
	-Wno-unused-function

all: $(TARGET)

$(TARGET): $(SECP256K1_OBJS) $(OBJS) 
	$(CC) -Wl,--gc-sections $(SECP256K1_OBJS) $(OBJS) -o $(TARGET)

secp256k1.o: $(SECP256K1_SRC_PATH)/secp256k1.c
	$(CC) $(CFLAGS) $(SECP256K1_CFLAGS) -c $< -o $@

precomputed_ecmult.o: $(SECP256K1_SRC_PATH)/precomputed_ecmult.c
	$(CC) $(CFLAGS) $(SECP256K1_CFLAGS) -c $< -o $@

precomputed_ecmult_gen.o: $(SECP256K1_SRC_PATH)/precomputed_ecmult_gen.c
	$(CC) $(CFLAGS) $(SECP256K1_CFLAGS) -c $< -o $@

command.o: command.c
	$(CC) $(CFLAGS) -c $< -o $@

random.o: random.c
	$(CC) $(CFLAGS) -c $< -o $@

serial.o: serial.c
	$(CC) $(CFLAGS) -c $< -o $@

secure.o: secure.c
	$(CC) $(CFLAGS) -I$(SECP256K1_INCLUDE_PATH) -c $< -o $@

nostr-key.o: nostr-key.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(SECP256K1_OBJS) $(TARGET)
