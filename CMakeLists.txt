cmake_minimum_required(VERSION 3.28)
project(midic)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/libremidi)

set(SOURCES_LIBS
        src/Midic.h
        src/Util.h
        src/Effects.h
)

add_executable(midic main.cpp ${SOURCES_LIBS})
target_link_libraries(midic libremidi)

add_executable(midic_rater main_raster.cpp ${SOURCES_LIBS})
target_link_libraries(midic_rater libremidi)
