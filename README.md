# obsidian

obsidian is a lightweight [ClassiCube](https://www.classicube.net/) server written in C.

**Note 2023-03-20**: I originally worked on this around 2018-2019, and haven't touched it since. I'm uploading it to GitHub, as the Git server hosting it has been dead about as long as that and I'd like it somewhere other than my possibly-failing hard drive.

## Build on Linux
### Debian dependencies
```sh
$ sudo apt install build-essential ninja zlib libcurl4-openssl-dev python3-pip
```

### Install `meson`
Packages from e.g. Debian and Ubuntu are probably out of date, so install `meson` using `pip`:
```sh
$ sudo pip3 install meson
```

### Build
```sh
$ git clone https://dev2.allfearthesentinel.net/csnxs/obsidian.git
$ cd obsidian
$ mkdir build
$ meson build
$ cd build
$ ninja
```

## Build on Windows
You'll need MSYS2. Open a Mingw64 console and run:
```sh
$ pacman -S git mingw-w64-x86_64-python3 mingw-w64-x86_64-python3-pip mingw-w64-x86_64-zlib mingw-w64-x86_64-curl mingw-w64-x86_64-ninja mingw-w64-x86_64-meson mingw-w64-x86_64-gcc mingw-w64-x86_64-pkg-config
$ git clone https://dev2.allfearthesentinel.net/csnxs/obsidian.git
$ cd obsidian
$ meson . build
$ cd build
$ ninja
```