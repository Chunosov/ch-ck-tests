# ch-armcl-mobilenet

`ck` implementation of ArmCL example `graph_mobilenet.cpp`
https://github.com/ARM-software/ML-ArmNN-collaboration/blob/master/examples/graph_mobilenet.cpp

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

Download and unpack weights package into `tmp` directory:
```
mkdir cnn_data
cd cnn_data
wget https://www.dropbox.com/s/bzi2lwy2t1mfxoq/arm_mobilenet_v1_1_224_model.zip
unzip arm_mobilenet_v1_1_224_model.zip
```
Weights are prepared by ARM.

## Build
```
ck compile program:ch-armcl-mobilenet
```

## Run
```
ck run program:ch-armcl-mobilenet
ck run program:ch-armcl-mobilenet --env.CK_IMG=../ILSVRC2012_val_00000002.ppm
ck run program:ch-armcl-mobilenet --env.CK_IMG=../ILSVRC2012_val_00000003.ppm
ck run program:ch-armcl-mobilenet --env.CK_IMG=../ILSVRC2012_val_00000004.ppm
```
