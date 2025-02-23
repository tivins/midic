cmake_minimum_required(VERSION 3.28)
project(midic)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/libremidi)
add_subdirectory(libs/fmt-11.1.3)


include(FetchContent)
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0 # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_MakeAvailable(yaml-cpp)


set(SOURCES_LIBS
        src/lib/Midic.h
        src/lib/Util.h
        src/lib/Effects.h
        src/lib/Display.h
)

add_executable(midic src/main.cpp ${SOURCES_LIBS})
target_link_libraries(midic libremidi)

add_executable(midic_raster src/main_raster.cpp ${SOURCES_LIBS})
target_link_libraries(midic_raster PUBLIC libremidi fmt yaml-cpp::yaml-cpp)

