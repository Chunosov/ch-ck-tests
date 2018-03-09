# ch-armcl-inception-v3

`ck` implementation of ArmCL example `graph_inception_v3.cpp`
https://github.com/ARM-software/ML-ArmNN-collaboration/blob/master/examples/graph_inception_v3.cpp

## Requirements

ArmCL compiled with Graph API:
```
ck install package:lib-armcl-opencl-private --env.USE_GRAPH=ON --env.USE_NEON=ON
# or
ck install package:lib-armcl-opencl-18.01 --env.USE_GRAPH=ON --env.USE_NEON=ON
```
To access `lib-armcl-*` package you have to pull `ck-math` repo:
```
ck pull ck-math
```

## Build
```
ck compile program:ch-armcl-inception-v3
```

## Run
```
ck run program:ch-armcl-inception-v3
ck run program:ch-armcl-inception-v3 --env.CK_IMG=../ILSVRC2012_val_00000002.ppm
ck run program:ch-armcl-inception-v3 --env.CK_IMG=../ILSVRC2012_val_00000003.ppm
ck run program:ch-armcl-inception-v3 --env.CK_IMG=../ILSVRC2012_val_00000004.ppm
```

## TODO
- Prepare weights and get download link.
- Pass image and labels files like `ch-armcl-mobilenet` does.