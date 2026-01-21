cmake_minimum_required(VERSION 3.28)
project(MIREA_AES)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "D:/qt/6.8.3/msvc2022_64")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(MIREA_AES WIN32
        main.cpp
        mainwindow.cpp
        mainwindow.hpp
        resources.qrc
)

target_link_libraries(MIREA_AES PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

if(WIN32)
    find_program(WINDEPLOYQT
            NAMES windeployqt
            PATHS "D:/qt/6.8.3/msvc2022_64/bin"
            NO_DEFAULT_PATH
    )

    if(WINDEPLOYQT AND CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(TARGET MIREA_AES POST_BUILD
                COMMAND "${WINDEPLOYQT}"
                --release
                --compiler-runtime
                --dir "$<TARGET_FILE_DIR:MIREA_AES>"
                "$<TARGET_FILE:MIREA_AES>"
                COMMENT "windeployqt: Копирование всех DLL и плагинов..."
        )
    endif()
endif()

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    find_program(ENIGMA_VB "EnigmaVirtualBox.exe" PATHS "C:/Program Files (x86)/Enigma Virtual Box")
    if(ENIGMA_VB)
        add_custom_command(TARGET MIREA_AES POST_BUILD
                COMMAND "${ENIGMA_VB}"
                -mode silent
                -outputfile "MIREA_AES_Portable.exe"
                -files "MIREA_AES.exe;.;."
                -iconfile "${CMAKE_SOURCE_DIR}/resources.qrc"
                WORKING_DIRECTORY "$<TARGET_FILE_DIR:MIREA_AES>"
        )
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release" AND MSVC)
    target_compile_options(MIREA_AES PRIVATE /O2)
    target_link_options(MIREA_AES PRIVATE /LTCG)
endif()
