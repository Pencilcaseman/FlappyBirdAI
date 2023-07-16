cmake_minimum_required(VERSION 3.23)
project(FlappyBirdAI)

set(CMAKE_CXX_STANDARD 17)

add_executable(FlappyBirdAI main.cpp)

add_subdirectory(Surge)
target_link_libraries(FlappyBirdAI PUBLIC surge)
