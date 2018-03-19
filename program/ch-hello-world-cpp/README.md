# ch-hello-world-cpp

Simple C++ program to test compilation on various platform.

## Notes
These meta entries
```
  "compiler_add_include_as_env_from_deps": [
    "CK_ENV_LIB_STDCPP_INCLUDE", 
    "CK_ENV_LIB_STDCPP_INCLUDE_EXTRA"
  ], 
  "linker_add_lib_as_env": [
    "CK_CXX_EXTRA", 
    "CK_ENV_LIB_STDCPP_STATIC"
  ],
```
are not necessary on Linux as `g++` can found C++ headers and libs at standard locations, but they should be here due to cross-compilation for Android.

## Build

### Linux
Build program for Linux:
```
ck compile program:ch-hello-world-cpp
```

### Android
Build program for Android:
```
ck compile program:ch-hello-world-cpp --target_os=android23-arm
ck compile program:ch-hello-world-cpp --target_os=android23-arm64
ck compile program:ch-hello-world-cpp --target_os=android23-x86
ck compile program:ch-hello-world-cpp --target_os=android23-x86_64
ck compile program:ch-hello-world-cpp --target_os=android14-mips
ck compile program:ch-hello-world-cpp --target_os=android21-mips64
```
To list all available android targets use:
```
ck ls os:android*
```

### Windows
**TODO:** Check on Windows.

## Run

```
ck run program:ch-hello-world-cpp
```
