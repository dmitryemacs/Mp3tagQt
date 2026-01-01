# Mp3tagQt

A Qt-based application for editing MP3 tags using the TagLib library.

## Description

Mp3tagQt is a graphical user interface application built with Qt framework that allows users to view and edit metadata tags in MP3 files. The application uses the TagLib library for reading and writing audio file metadata.

## Features

- View MP3 file tags (ID3v1, ID3v2)
- Edit artist, album, title, genre, year, and other metadata
- Batch editing of multiple files
- Support for various audio formats through TagLib

## Requirements

- Qt 6.10.1 or higher
- CMake 3.30.5 or higher
- TagLib 2.1.1 (included in the repository)

## Building

### Using Makefile (Recommended)

The project includes a Makefile for convenient building and running:

```bash
# Generate CMake configuration files
make generate

# Build the project
make build

# Build and run the application
make run

# Clean build directory
make clean

# Generate, build and run (all in one)
make all
```

### Windows (Qt 6.10.1 with MinGW)

1. Clone the repository:
   ```bash
   git clone git@github.com:dmitryemacs/Mp3tagQt.git
   cd Mp3tagQt
   ```

2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Configure the project with CMake:
   ```bash
   cmake -S D:/QtProjects/Mp3tagQt -B D:/QtProjects/Mp3tagQt/build/Desktop_Qt_6_10_1_llvm_mingw_64_bit-Debug "-DCMAKE_COLOR_DIAGNOSTICS:BOOL=ON" "-DCMAKE_PREFIX_PATH:PATH=D:/Programms/Qt/6.10.1/llvm-mingw_64" "-DCMAKE_C_COMPILER:FILEPATH=D:/Programms/Qt/Tools/llvm-mingw1706_64/bin/clang.exe" "-DCMAKE_CXX_FLAGS_INIT:STRING=-DQT_QML_DEBUG" "-DCMAKE_GENERATOR:STRING=Ninja" "-DCMAKE_CXX_COMPILER:FILEPATH=D:/Programms/Qt/Tools/llvm-mingw1706_64/bin/clang++.exe" "-DCMAKE_BUILD_TYPE:STRING=Debug"
   ```

4. Build the project:
   ```bash
   cmake --build D:/QtProjects/Mp3tagQt/build/Desktop_Qt_6_10_1_llvm_mingw_64_bit-Debug --target all
   ```

### Linux/macOS

1. Clone the repository:
   ```bash
   git clone https://github.com/dmitryemacs/Mp3tagQt.git
   cd Mp3tagQt
   ```

2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Configure the project with CMake:
   ```bash
   cmake ..
   ```

4. Build the project:
   ```bash
   cmake --build .
   ```

## Usage

After building, you can run the application:

```bash
./mp3tag
```

Or on Windows:

```bash
mp3tag.exe
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [Qt Framework](https://www.qt.io/)
- [TagLib](https://taglib.org/)
