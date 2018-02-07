# opencv-test

Simple demo for using OpenCV.

## Status

I've tried to build it using `lib-opencv-3.3.0`, but **can't build** currently.

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

But even after that it complains about `undefined reference to png_set_longjmp_fn`, because of
> The "png_set_longjmp_fn()" API was introduced in libpng-1.4.x.

but `sudo apt-get install libpng-dev` on Ubuntu 16.04 tells us `libpng12-dev is already the newest version (1.2.54-1ubuntu1)` and there is no `libpng-dev` package in `ck`.

**BUT** both of those (1, 2) are needless when `lib-caffe` is in deps, program is built an run smoothly!