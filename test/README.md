# nsdemu-test

## Description

Endlessly send `/ping`, `/public-key`, `/sign-message` and `/shared-secret` command to check nostr-signing-device stability.

Response from nostr-signing-device is not checked.

## Build procedure for OpenBSD/Linux

GNU Make and arc4random() required, simply `make` (or `gmake`).

## Usage

```
$ nsdemu-test -l [serial device]
```

## License

MIT License
