# ch-hello-world-cpp

Simple C++ program to test compilation on various platform.

## Build

### Linux
Build program for Linux:
```
ck compile program:ch-hello-world-cpp
```

### Android
Build program for Android:
```
ck compile program:ch-hello-world-cpp --target_os=android23-arm64
```
To list all available android targets use:
```
ck ls os:android*
```

## Run

```
ck run program:ch-hello-world-cpp
```
