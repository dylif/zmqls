# ZMQ Live Stream (zmqls)

This project is based on https://github.com/chenxiaoqino/udp-image-streaming. Instead of raw UDP sockets, it uses ZMQ. This makes it more reliable and easier to scale. It retains negligible latency.

## Encoding

No video codec is used. Every frame is encoded to `jpeg` format by OpenCV to drastically reduce the bandwidth consumption.

## TODO

* Small performance tweaks (limit frame copying, etc.)
* Fix CMake
