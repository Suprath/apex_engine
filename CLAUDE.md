# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**apex_engine** is a C++20 project using CMake for builds. The project is intended to be built and run within Docker containers.

## Build System

- **Build tool**: CMake (version 4.2+)
- **C++ standard**: C++20
- **Primary executable**: `apex_engine`

### Building

The project uses CMake. The typical workflow is:

1. Configure: `cmake -B build`
2. Build: `cmake --build build`
3. Run: `./build/apex_engine`

**Note**: When Docker is set up, builds should be executed within the Docker container rather than locally.

### CMakeLists.txt Structure

The CMakeLists.txt is intentionally minimal:
- Sets C++20 as the required standard
- Defines a single executable target (`apex_engine`) from `main.cpp`

As the project grows, you'll likely add:
- Additional source files via `add_executable()` or `add_library()`
- External dependencies via `find_package()` or `fetch_content()`
- Subdirectories with `add_subdirectory()`
- Compiler flags and optimizations

## Docker Setup

Docker support is planned but not yet configured. When setting up Docker:
1. Create a `Dockerfile` that includes a C++20 compiler (clang or GCC) and CMake
2. Optionally create a `docker-compose.yml` for development workflows
3. Add a `.dockerignore` to exclude build artifacts and IDE files

## Code Structure

Currently the project has:
- `main.cpp` - Entry point with a simple starter example

As development progresses, organize code into:
- `src/` - Implementation files (.cpp)
- `include/` or `src/` - Header files (.h, .hpp)
- `tests/` - Unit tests (if applicable)

## Development Workflow

Since nothing is installed locally yet:
- Use Docker for all builds and development
- Build commands should be run inside the container
- The IDE (.idea directory suggests CLion) can be configured to build via Docker

## Project Purpose

The project name "apex_engine" suggests this may become an engine or core library, but the current implementation is a minimal starter. Future development will define its actual purpose and scope.
