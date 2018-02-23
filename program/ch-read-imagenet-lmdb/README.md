# ch-read-imagenet-lmdb

Example of reading Caffe ImageNet LMDB database and example of deserialization protobuf binary value without using of Protobuf library.

## Requirements

LMDB library:
```
ck install package --tags=lib,lmdb
```

ImageNet val package, it is required for LMDB packages:
```
ck install package --tags=dataset,imagenet,val
```

ImageNet LMDB package:
```
ck install package --tags=dataset,imagenet,val-lmdb
```
Installed Caffe package is also required for LMDB packages, because of they use Caffe's program `convert_imageset` for preparation of LMDB databases.

## Compile
```
ck compile program:ch-read-imagenet-lmdb
```

## Run
```
ck run program:ch-read-imagenet-lmdb --env.CK_IMG_COUNT=50
```

## Notes
Each value in Caffe ImageNet LMDB database is binary serialized Caffe Datum protobuf object. Its protobuf definition is:
```
message Datum {
  optional int32 channels = 1;
  optional int32 height = 2;
  optional int32 width = 3;
  // the actual image data, in bytes
  optional bytes data = 4;
  optional int32 label = 5;
  // Optionally, the datum could also hold float data.
  repeated float float_data = 6;
  // If true data contains an encoded image that need to be decoded
  optional bool encoded = 7 [default = false];
}
````
Binary representation of protobuf messages is described in [official documentation](https://developers.google.com/protocol-buffers/docs/encoding).