# ch-hello-world-c

Simple C program to test compilation on various platform.

## Build

### Linux
Build program for Linux:
```
ck compile program:ch-hello-world-c
```

### Android
Build program for Android:
```
ck compile program:ch-hello-world-c --target_os=android23-arm
ck compile program:ch-hello-world-c --target_os=android23-arm64
ck compile program:ch-hello-world-c --target_os=android23-x86
ck compile program:ch-hello-world-c --target_os=android23-x86_64
ck compile program:ch-hello-world-c --target_os=android14-mips
ck compile program:ch-hello-world-c --target_os=android21-mips64
```
To list all available android targets use:
```
ck ls os:android*
```

## Run

```
ck run program:ch-hello-world-c
```
