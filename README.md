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

### Windows (MinGW)

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
   cmake -G "MinGW Makefiles" ..
   ```

4. Build the project:
   ```bash
   cmake --build .
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
