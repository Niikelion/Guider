# Guider - gui library for easier tool creation
Guider is modular, object oriented, cross-platform gui library. It focuses on ability to create beautiful and responsive user interface while minimizing need for writing code.

- [Authors](#authors)
- [Install](#install)
  - [Requirements](#requirements)
  - [Integrate in project](#integrate-in-project)
  - [Build Guider from sources](#build-guider-from-sources)
- [Concepts](#concepts)
- [Basic use](#basic-use)

## Authors

- Michał Osiński — main developer

## Install

### Requirements

| Target             | Requirements                                |
| ------------------ | ------------------------------------------- |
| Guider             | ParseLib(included as git submodule)         |
| SFML backend       | Guider, SFML                                |
| Guider Layout tool | Guider, SFML backend                        |
| Examples           | Guider, SFML, git(installed as application) |



### Integrate in project

Guider exports cmake package(not done yet):

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

Additional cmake options:

| Option            | Description                               | Default value |
| ----------------- | ----------------------------------------- | ------------- |
| BUILD_TESTS       | Enables building tests. Requires Catch2.  | false         |
| BUILD_EXAMPLES    | Enables building examples. Requires SFML. | true          |
| BUILD_TOOLS       | Enables building tools. Requires SFML.    | true          |
| DEV_BUILD         | For Guider developmnet                    | false         |
| INTERNAL_PARSELIB | Allows using parselib from submodule      | false         |

## Concepts

- Component - element that can be drawn and organized into hierarchy. Events are passed down the hierarchy, but some(position based) events may not be passed to some subtrees because they are outside their bounding boxes.
- Container - special type of component, that can contain other components. Containers are responsible for drawing their children, updating them and dispatching events to them. Additionally, children should receive events in order they are being drawn in.
- Event stealing - when containers are handling events, they dispatch them to their children sequentially until every child received event or one of them "stole it". Stealing prevents events from being passed to remaining children.

## Basic use

TODO
