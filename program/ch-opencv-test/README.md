# opencv-test

Simple demo for using OpenCV.

## Status

I've tried to build it using `lib-opencv-3.3.0`, but can't build currently.

`lib-opencv-3.3.0` is built as static lib by default, may be it's a problem.

1) Have to rebuild OpenCV lib with
```
-DWITH_WEBP=OFF -DWITH_ITT=OFF -DWITH_IPP=OFF
```
(I've just edited `PACKAGE_CONFIGURE_FLAGS` in meta)

2) A number of additional libs should be mentioned in program's meta, otherwise a lot of unresolved references are appeared when linking:
```
"extra_ld_vars": "-lopencv_imgcodecs -ldl -lz -lIlmImf -lHalf -lpng -ljasper -ljpeg -ltiff",
```

*BUT* both of those are needless when `lib-caffe` is in deps! Program is built an run smoothly!