# A qrcode image decoder in C++ and python with opencv qrcode decoder module.

## Compile:

```
g++ -o qrcodereader qrcodereader.cpp $(pkg-config --cflags --libs opencv4 libheif) -std=c++11
```



The sample heic and png files are included, and for verify the heic library, heic2png is to convert heic file to png format.
most of the code are written by cursor, the XX_copilot.cpp is written by copilot, which is not working.
