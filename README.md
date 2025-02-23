# MIDIC 
C++ MIDI recorder and visualizer

![preview.png](data/preview-2.png)


### Install:

Requirements:

* `ffmpeg` to be installed.

```shell
mkdir build
cd build
cmake ..
make
```

### Usage:

* Record an instrument
  ```shell
  ./midic [<raw file>]
  ```
* Generate video
  ```shell
  ./midic_raster <raw file> <config file>
  ``` 
* Tools (planned)
  ```shell
  #./midic_trim <input raw file> [<output raw file>]
  #./midic_quantize <input raw file> [<output raw file>]
  ```
  
-----

### TODO

* [ ] display velocity (alpha + glow)
* [ ] create midic_trim
* [ ] create midic_midi
* [ ] create midic_quantize  