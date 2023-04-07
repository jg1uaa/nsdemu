# nsdemu (Nostr Signing Device EMUlator)

## Description

Seek possibility of porting [nostr-signing-device](https://github.com/lnbits/nostr-signing-device) (NSD) to other SoC (not ESP32), emulate NSD on OpenBSD and Linux (arc4random() supported glibc is required).

## Build procedure for OpenBSD/Linux

```
$ git clone --recursive https://github.com/jg1uaa/nsdemu
$ cd nsdemu
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Build procedure for RP2040

```
$ git clone --recursive https://github.com/jg1uaa/nsdemu
$ cd nsdemu
$ git clone --recursive https://github.com/raspberrypi/pico-sdk
$ mkdir build
$ cd build
$ cmake -DPLATFORM=rp2040 -DKEYSTR=nsec1-your-secret-key ..
$ make
$ cp nsdemu.uf2 /path/to/rp2boot-device
$ sync
```

## Usage for OpenBSD/Linux

```
$ nsdemu -l [serial device] -k [secret key]
```

Secret key accepts both bech32 format (nsec1...) and 64digit-hex format.

## Supported command

Only `/ping`, `/public-key`, `/sign-message` and `/shared-secret` is implemented. Enough to work with [horse](https://github.com/fiatjaf/horse).

## Note

*For test purpose only*, argument of nsdemu is shown by ps command.

## License

MIT License

## Issue

Currently nsdemu uses [secp256k1](https://github.com/bitcoin-core/secp256k1) library, it contains precomputed table. The smallest size of the table is 32k bytes, replacing more small library is required to fit in small-storage MCU.
