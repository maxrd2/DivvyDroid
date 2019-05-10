# DivvyDroid [![TravisCI build](https://travis-ci.org/maxrd2/DivvyDroid.svg?branch=master)](https://travis-ci.org/maxrd2/DivvyDroid)

`DivvyDroid` is an Qt/C++ application for remote controlling you Android device. It implements ADB TCP client. It was written and tested under Linux, but it should also compile under Windows and Mac OS X.

- Requires ADB to be installed, root might be needed on some devices
- Display Android device screen using screencap (JPEG, PNG, RAW) over ADB TCP
- Fast and accurate touch events by writing directly to /dev/input on android device
- Fallback to slow and inaccurate adb shell input command without /dev/input write permissions
- Keyboard support/emulation using adb shell input command
![screenshot](https://user-images.githubusercontent.com/1187381/57383580-5a513e00-71af-11e9-864b-68f31cb5e7b1.png)

## Build/Install

You will require git, Qt5 and gcc (or clang) compiler.
```shell
git clone https://github.com/maxrd2/DivvyDroid.git && cd DivvyDroid
mkdir build && cd build
qmake ..
make
sudo make install
```

## Contributing

Pull requests and patches are welcome. Please follow the [coding style](README.CodingStyle.md).

Feedback or ideas on how to make `DivvyDroid` even better are welcome.

## License

`DivvyDroid` is released under [GNU General Public License v3.0](LICENSE)
