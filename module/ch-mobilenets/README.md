# Helper module to manage MobileNet packages

This module is aided to manage bunch of MobileNet packages. 

There are different sets of MobileNet packages:

`ck-tensorflow:tensorflowmodel-mobilenet-v1-*-py`
version V1 2017-06-14.

`ck-tensorflow:package:tensorflowmodel-mobilenet-v1-*-2018_02_22-py`
version [V1 2018-02-22](https://github.com/tensorflow/models/blob/master/research/slim/nets/mobilenet_v1.md).

`ck-tensorflow:package:tensorflowmodel-mobilenet-v2-*-py`
version [V2](https://github.com/tensorflow/models/tree/master/research/slim/nets/mobilenet).

`ck-request-asplos18-mobilenets-armcl-opencl:package:weights-mobilenet-v1-*-npy`
version V1 2017-06-14.

## Actions

### `make_*`
Make all packages.
```bash
ck make_v1 ch-mobilenets
ck make_v2 ch-mobilenets
```

### `rm_v2`
Remove all packages.
```bash
ck rm_v2 ch-mobilenets
```        
It just does set of calls for `ck rm` for each package found by its name.

### update_meta_*
Write common parameterized `meta.json` to all package dirs.
```bash
ck update_meta_v1 ch-mobilenets
ck update_meta_v1_2018 ch-mobilenets
ck update_meta_v2 ch-mobilenets
``` 
Next version writes `meta.json` to packages in `ck-request-asplos18-mobilenets-armcl-opencl` repo:
```bash
ck update_meta_v1_npy ch-mobilenets
``` 

### update_meta_v2
Write common parameterized `meta.json` to all package dirs.
```bash
ck update_meta_v2 ch-mobilenets
```    