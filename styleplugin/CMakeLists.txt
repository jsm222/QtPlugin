cmake_minimum_required(VERSION 3.5)
project(pstyleplugin)

set(TARGET pstyleplugin)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(QT Core Gui Widgets DBus Multimedia)
find_package(Qt5 REQUIRED ${QT})
find_package(PkgConfig REQUIRED)
find_package(KF5 REQUIRED WindowSystem)

include(GNUInstallDirs)

set (SRCS
    pstyleplugin.cpp
    pstyleplugin.h
    basestyle.h
    basestyle.cpp
    phantomcolor.h
    phantomcolor.cpp
    shadowhelper.h
    shadowhelper.cpp
    tileset.h
    tileset.cpp
    boxshadowrenderer.h
    boxshadowrenderer.cpp
    sound.h
    sound.cpp
)

add_library(${TARGET} MODULE ${SRCS})
target_link_libraries(${TARGET}
    Qt5::GuiPrivate
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    KF5::WindowSystem
    Qt5::Multimedia
    )

set(SOUND_FILES
    ../sounds/ping.wav
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_QTPLUGINDIR}/styles/)

install(FILES ${SOUND_FILES} DESTINATION "${CMAKE_INSTALL_DATADIR}/panda/sounds")
