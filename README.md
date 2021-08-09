OdinData
========

OdinData is a modular, scalable, high-throughput data acquisition framework consisting of two communicating applications; the FrameReceiver and FrameProcessor.
It acts as a data capture and processing pipeline for detector data streams transferred over a network connection.
The FrameReceiver constructs data frames in shared memory buffers and the FrameProcessor performs processing on the buffer and writes it to disk.

[![Code Climate](https://codeclimate.com/github/odin-detector/odin-data/badges/gpa.svg)](https://codeclimate.com/github/odin-detector/odin-data)
[![Test Coverage](https://codeclimate.com/github/odin-detector/odin-data/badges/coverage.svg)](https://codeclimate.com/github/odin-detector/odin-data/coverage)

External Software Dependencies
==============================

The following libraries and packages are required:

* [CMake](http://www.cmake.org) : build management system (version >= 2.8)
* [Boost](http://www.boost.org) : portable C++ utility libraries. The following components are used - program_options, unit_test_framework, date_time, interprocess, bimap (version >= 1.41)
* [ZeroMQ](http://zeromq.org) : high-performance asynchronous messaging library (version >= 4.1.4)
* [Log4CXX](http://logging.apache.org/log4cxx/): Configurable message logger (version >= 0.10.0)
* [HDF5](https://www.hdfgroup.org/HDF5): __Optional:__ if found, the FrameProcessor application will be built (version >= 1.8.14)
* [Blosc](http://blosc.org)/[c-blosc](https://github.com/blosc/c-blosc): __Optional:__ if found, the FrameProcessor plugin "BloscPlugin" will be built

Building
========

Instructions for building OdinData are given [here](docs/Building.md)

Usage
=====
Instructions for how to use each component of OdinData are given below:

* [FrameReceiver](docs/FrameReceiver.md)
* [FrameProcessor](docs/FrameProcessor.md)
* [FrameSimulator](docs/FrameSimulator.md)
* [Tools](docs/Tools.md)

