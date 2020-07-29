cmake_minimum_required(VERSION 3.0)

project(panda-qt5-plugins)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(platformthemeplugin)
add_subdirectory(styleplugin)
