project(wavy)

include_directories(./src/)
include_directories(../cmd/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FALGS} -g -O3 -std=c++20")

add_executable(wavy 
    src/frame_generator.cpp
    main.cpp
)
