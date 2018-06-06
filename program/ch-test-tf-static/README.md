# Test program for TensorFlow static ck package.

## Requirements

```bash
ck install package:lib-tensorflow-1.7.0-src-static
ck install package:lib-tensorflow-1.7.0-src-static --target_os=android23-arm64
```

## Build

```bash
ck compile program:ch-test-tf-static
ck compile program:ch-test-tf-static --target_os=android23-arm64
```

## Run

```bash
ck run program:ch-test-tf-static
ck run program:ch-test-tf-static --target_os=android23-arm64
```
