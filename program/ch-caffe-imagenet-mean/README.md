# ch-caffe-imagenet-mean

Generates `mean.binaryproto` file for ImageNet LMDB database.

## Requirements

Caffe library:
```
ck install package:lib-caffe-bvlc-master-cpu-universal
```
or
```
ck install package:lib-caffe-intel-master-cpu
```
or another one.


LMDB ImageNet dataset for which the mean should be calcuated:
```
ck install package:imagenet-2012-val-lmdb-224
ck install package:imagenet-2012-val-lmdb-227
ck install package:imagenet-2012-val-lmdb-256
```

## Run
```
ck run program:ch-caffe-imagenet-mean
```
Output file is in `tmp` directory