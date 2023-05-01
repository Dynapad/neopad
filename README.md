# NeoPad

A modern C17 rewrite of Pad++.

See below for:

  - [Get Started](#get-started)
  - [Usage](#usage)
  - [Developing](#developing)
    - [Directory Structure](#directory-structure)
  - 

## Get Started

Build using CMake:

```bash
cmake -B cmake-build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --target neopad_demo
```

Run the demo application:

```bash
./cmake-build-debug/demo/neopad
```

## Usage

### Controls

To exit, type `:q` and press `Enter`. There might be another way.

- `Click + Drag` - Pan
- `Mouse Wheel` - Zoom
- `Space` - Reset View

## Developing

### Directory Structure

- `src/` - Source code
  - `internal/` - Private headers
  - `shader/` - BGFX shader files
  - `renderer/` - Renderer _feature_ implementations.
  - `renderer.c` - Core renderer implementation.
  - `neopad.c` - Doesn't serve much purpose currently.
- `test/` - Unit tests
- `demo/` - Demo application
- `include/` - Public headers
- `cmake/` - CMake modules

