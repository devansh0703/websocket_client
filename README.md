# WebSocket Client

A C++ WebSocket client implementation with SSL/TLS support using Boost.Beast and the GN build system.

## Features

- Secure WebSocket connections (WSS) support
- Command-line interface using CLI11
- Flexible message handling
- Comprehensive test suite
- Support for both GCC and Clang
- Debug and Release build configurations

## Prerequisites

- GN build system
- Ninja build system
- C++17 compatible compiler (GCC or Clang)
- Boost libraries (1.70.0 or newer)
- OpenSSL (1.1.1 or newer)
- CMake (for building dependencies)

## Building

1. Install dependencies:

```bash
# Ubuntu/Debian
sudo apt-get install build-essential libboost-all-dev libssl-dev cmake ninja-build

# Fedora
sudo dnf install gcc-c++ boost-devel openssl-devel cmake ninja-build
```

2. Configure the build:

```bash
# Debug build (default)
gn gen out/Debug

# Release build
gn gen out/Release --args="is_debug=false"

# Using Clang
gn gen out/Debug --args="is_clang=true"
```

3. Build the project:

```bash
ninja -C out/Debug
# or
ninja -C out/Release
```

4. Run tests:

```bash
./out/Debug/websocket_client_test
```

## Usage

```bash
# Basic usage with default settings (wss://echo.websocket.org:443)
./out/Debug/websocket_client

# Connect to a specific server
./out/Debug/websocket_client --host example.com --port 8080 --target /ws

# Use non-secure WebSocket
./out/Debug/websocket_client --no-secure --port 80
```

## Development

- Source code is in the `src/` directory
- Tests are in the `test/` directory
- Build configuration is in `build/` directory

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run the tests
5. Submit a pull request

## License

MIT License
