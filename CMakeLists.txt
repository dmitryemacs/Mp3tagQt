cmake_minimum_required(VERSION 3.16)

project(Mp3tagQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Add TagLib for MP3 tag editing
# Use TagLib from source as subproject
add_subdirectory(taglib-2.1.1)
set(TAGLIB_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/taglib-2.1.1/taglib"
    "${CMAKE_CURRENT_SOURCE_DIR}/taglib-2.1.1/taglib/toolkit"
    "${CMAKE_CURRENT_SOURCE_DIR}/taglib-2.1.1/taglib/mpeg"
    "${CMAKE_CURRENT_SOURCE_DIR}/taglib-2.1.1/taglib/mpeg/id3v2"
    "${CMAKE_CURRENT_SOURCE_DIR}/taglib-2.1.1/taglib/mpeg/id3v2/frames"
    "${CMAKE_CURRENT_BINARY_DIR}/taglib-2.1.1"
)
set(TAGLIB_LIBRARIES taglib)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(mp3tag
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET mp3tag APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(mp3tag SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(mp3tag
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(mp3tag PRIVATE Qt${QT_VERSION_MAJOR}::Widgets "${CMAKE_CURRENT_BINARY_DIR}/taglib-2.1.1/taglib/libtag.a")
target_include_directories(mp3tag PRIVATE ${TAGLIB_INCLUDE_DIRS})
target_compile_definitions(mp3tag PRIVATE TAGLIB_STATIC)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.mp3tag)
endif()
set_target_properties(mp3tag PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS mp3tag
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(mp3tag)
endif()
