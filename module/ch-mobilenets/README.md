# Helper module to manage MobileNet packages

This module is aided to manage bunch of `ck-tensorflow:tensorflowmodel-mobilenet-*-py` packages. 

Currently it works only with `v2` packages but can be easily extended to process `v1`.

[MobileNetV1](https://github.com/tensorflow/models/blob/master/research/slim/nets/mobilenet_v1.md)

[MobileNetV2](https://github.com/tensorflow/models/tree/master/research/slim/nets/mobilenet)

## Actions

### make_v2

Make all packages.

`ck add ck-tensorflow:package:ck-tensorflow:tensorflowmodel-mobilenet-v2-*-py`

### rm_v2

Remove all packages.

`ck rm ck-tensorflow:package:ck-tensorflow:tensorflowmodel-mobilenet-v2-*-py`

### update_meta_v2

Write common parameterized `meta.json` to all package dirs.
