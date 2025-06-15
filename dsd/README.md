## How to build
First of all, if you're on Windows:
```sh
export CARGO_BUILD_TARGET=x86_64-pc-windows-gnu
```

Run this at least once:
```sh
cmake -B build -DBUILD_STATIC=ON -DUSE_QT6=OFF -DCMAKE_PREFIX_PATH=$MSYSTEM_PREFIX/qt5-static
```

Then run this to build `melonDS.exe` + `dsd_melonds`:
```
cmake --build build
```

## Credits

C++/Rust bridge with cross-language LTO: https://github.com/XiangpengHao/cxx-cmake-example
