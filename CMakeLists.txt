cmake_minimum_required(VERSION 4.0)

project(GIScriptEditor LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_subdirectory(GIScript)
add_subdirectory(GIScriptEditor)
