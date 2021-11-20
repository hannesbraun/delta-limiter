# Delta Limiter

A weird distortion and lowpass plugin

- Plugin format: LV2
- Supported platforms: Linux, macOS, Windows

## Building

```bash
mkdir build
cd build
cmake ..
make
```

### Windows

It is recommended to use MSYS2 to build this plugin on Windows. Install LV2 by getting the repository from https://gitlab.com/lv2/lv2 and install it with the following commands.

```bash
./waf configure --prefix=/usr/local --no-plugins --lv2dir=lib
./waf build
./waf install
```

When running CMake, you may need to set the `PKG_CONFIG_PATH` environment variable to `/usr/local/lib/pkgconfig` (or whereever you installed lv2 previously). This is needed if this directory isn't already in the pkg-config search path.

## Documentation

```bash
pandoc -s user_manual.md -o user_manual.html
pandoc -s user_manual.md -o user_manual.pdf
```

## License

Copyright Hannes Braun 2021.

Distributed under the Boost Software License, Version 1.0.

(See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
