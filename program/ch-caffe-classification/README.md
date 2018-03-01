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

## Parameters

### `CK_BATCH_COUNT`
Numbers batches to classify.

### `CK_BATCH_SIZE`
Number of images in every batch.

### `CK_SKIP_IMAGES`
Number of images to skip from beginning of dataset.