
cmake_minimum_required(VERSION 3.4.1)


add_subdirectory(librtmp)

file(GLOB source ${CMAKE_SOURCE_DIR}/codec/*.cpp)

add_library(
        rtmpPush

        SHARED
        native-lib.cpp
        ${source}
)

include_directories(include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} _STDINT_H")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_STDINT_H")
#add_definitions(-D __FLOAT_WORD_ORDER)
target_link_libraries(
        rtmpPush
        rtmp
        x264
        faac
        log)