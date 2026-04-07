cmake_minimum_required(VERSION 4.2)
project(Gaming_Posture_Fixer)

set(CMAKE_CXX_STANDARD 17)

enable_language(RC)

add_executable(Gaming_Posture_Fixer #WIN32
        src/main.cpp
        src/resources.rc
)

target_include_directories(Gaming_Posture_Fixer PRIVATE include)

target_compile_definitions(Gaming_Posture_Fixer PRIVATE UNICODE _UNICODE)

target_link_libraries(Gaming_Posture_Fixer PRIVATE
    user32
    gdi32
    winmm
)

target_link_options(Gaming_Posture_Fixer PRIVATE
    -static
    -static-libgcc
    -static-libstdc++
)
