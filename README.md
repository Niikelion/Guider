<p align="center"><img src="assets/project-logo.png" alt="guider-logo" width="256" height="256" /></p>

# Guider - GUI library for easier tool creation

Guider is a modular, object-oriented, cross-platform GUI library. It focuses on the ability to create a beautiful and responsive user interface while minimizing the need for writing code.

- [Authors](#authors)
- [Features](#features)
- [Install](#install)
  - [Requirements](#requirements)
  - [Integrate in project](#integrate-in-project)
  - [Build Guider from sources](#build-guider-from-sources)
- [Concepts](#concepts)
- [Examples](#examples)
- [License](#license)

## Authors

- Michał Osiński — main developer

## Features

- Cross-platform
- Loading hierarchy from XML files 
- Styling elements from XML files & code
- Ease of embedding into the application
- Intuitive layout types
- Optimized for(but not limited to) static GUI

Planned features:

- Animations
- Panel utility
- Optimized binary format and JSON support
- Inputfield, Scrollbar and Switch implementations
- Focus system
- More events

## Install

### Requirements

| Target             | Requirements                          |
| ------------------ | ------------------------------------- |
| Guider             | ParseLib(included as a git submodule) |
| SFML backend       | Guider, SFML                          |
| Guider Layout tool | Guider, SFML backend                  |
| Examples           | Guider, SFML backend                  |



### Integrate in project

Guider exports CMake package(WIP):

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

Additional CMake options:

| Option            | Description                                                  | Default value |
| ----------------- | ------------------------------------------------------------ | ------------- |
| BUILD_TESTS       | Enables building tests. Requires Catch2.                     | false         |
| BUILD_EXAMPLES    | Enables building examples. Requires SFML.                    | true          |
| BUILD_TOOLS       | Enables building tools. Requires SFML.                       | true          |
| DEV_BUILD         | Used for Guider development. When enabled, Guider uses resource files from the source directory, instead of copying them to the build directory and using them from there. | false         |
| INTERNAL_PARSELIB | Allows using ParseLib from the submodule.                    | true          |

## Concepts

- **Component** - element that can be drawn and organized into a hierarchy. Events are passed down the hierarchy, but some(position-based) events may not be passed to some subtrees because they are outside their bounding boxes.
- **Container** - a special type of component, that can contain other components. Containers are responsible for drawing their children, updating them and dispatching events to them. Additionally, children should receive events in order they are being drawn in.
- **Event stealing** - when containers are handling events, they dispatch them to their children sequentially until every child received event or one of them "stole it". Stealing prevents events from being passed to the remaining children.
- **Backend** - implementation of rendering and resource management. All backends are required to implement every platform-specific feature that Guider requires to run. Because of this approach, every component that is implemented using purely Guider API is guaranteed to work on every platform that have proper backend implementation.
- **Gravity** - vertical/horizontal alignment.

## Components

List of components provided by default can be found [here](ELEMENTS.md).

## Examples

Examples for using Guider can be found [here](examples/README.md).

## License

Guider is available under [MIT](LICENSE) license.
