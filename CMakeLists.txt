cmake_minimum_required(VERSION 3.0)

project(ExifStat)

find_package(OpenMP REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Crée des variables avec les fichiers à compiler
set(SRCS
        cpp_exiftool/src/ExifTool.cpp
        cpp_exiftool/src/ExifToolPipe.cpp
        cpp_exiftool/src/TagInfo.cpp
        src/main.cpp src/FileManager.cpp)

set(HEADERS
        cpp_exiftool/inc/ExifTool.h
        cpp_exiftool/inc/ExifToolPipe.h
        cpp_exiftool/inc/TagInfo.h
        src/FileManager.h)

# On indique que l'on veut un exécutable "hello" compilé à partir des fichiers décrits par les variables SRCS et HEADERS
add_executable(ExifStat ${SRCS} ${HEADERS})