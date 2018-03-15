# ch-caffe-detection

Demo program showing how to use caffe library for object detection. Code is based on caffe example `${CK-TOOLS}/lib-caffe-*-ssd-*/src/examples/ssd/ssd_detect.cpp`

## Requirements

Caffe library:
```
ck install package --tags=lib,caffe,ssd
```

Caffe models:
```
ck install package --tags=caffemodel,ssd
```

Detection dataset
```
ck install package --tags=dataset,images,object-detection
```

## Build
```
ck compile program:ch-caffe-detection
```

## Run
```
ck run program:ch-caffe-detection
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
- Postprocess detection output, draw annotated boxes over source image.