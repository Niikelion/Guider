# Guider - gui library for easier tool creation
Guider is modular, object oriented, cross-platform gui library. It focuses on ability to create beautiful and responsive user interface while minimizing need for writing code.

[TOC]



## Authors

- Michał Osiński — main developer

## Install

### Integrate in project

Guider exports cmake package(not yet):

```cmake
find_package(Guider CONFIG REQUIRED)

target_link_libraries(target PUBLIC Guider::Guider)
```

### Build Guider from sources

To build Guider with examples and tools:

```bash
mkdir build
cd build
cmake ../
cmake --build .
```

Note that Guider requires ParseLib.

Additional cmake options:

| Option         | Description                                                  |
| -------------- | ------------------------------------------------------------ |
| BUILD_TESTS    | Enables building tests. Requires Catch2.  Defaults to false. |
| BUILD_EXAMPLES | Enables building examples. Requires SFML. Defaults to true.  |
| BUILD_TOOLS    | Enables building tools. Requires SFML. Defaults to true.     |



## Concepts

TODO

## Basic use

TODO
