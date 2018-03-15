# ch-caffe-classification

Demo program showing how to use caffe library for ImageNet classification. Code is based on caffe example `${CK-TOOLS}/lib-caffe-*/src/examples/cpp_classification/classification.cpp`.

## Requirements

Caffe library:
```
ck install package --tags=lib,caffe
```

Caffe models:
```
ck install package --tags=caffemodel
```

ImageNet dataset
```
ck install package:imagenet-2012-aux
ck install package --tags=imagenet,val,raw
```

## Build
```
ck compile program:ch-caffe-classification
```

## Run
```
ck run program:ch-caffe-classification
```

Sample output:
```
ILSVRC2012_val_00000001.JPEG
0.4395 - n01697457 African crocodile, Nile crocodile, Crocodylus niloticus
0.2030 - n01664065 loggerhead, loggerhead turtle, Caretta caretta
0.1835 - n01665541 leatherback turtle, leatherback, leathery turtle, Dermochelys coriacea
0.0975 - n01698640 American alligator, Alligator mississipiensis
0.0157 - n01737021 water snake
```

## Parameters

### `CK_BATCH_COUNT`
Numbers batches to classify.

### `CK_BATCH_SIZE`
Number of images in every batch.

### `CK_SKIP_IMAGES`
Number of images to skip from beginning of dataset.

## TODO

- There is not batched operation currently implemented. All images are loaded and classified one by one.
- Check for prediction correctness.
