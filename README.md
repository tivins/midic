# MIDIC 
C++ MIDI recorder and visualizer

![preview.png](data/preview.png)


Install:

Requirements:

* `ffmpeg` to be installed.

```shell
mkdir build
cd build
cmake ..
make
```

Usage:

* Record the instrument
  ```shell
  ./midic
  ```
* Generate video
  ```shell
  ./midic_raster <raw file> <config file>
  ``` 