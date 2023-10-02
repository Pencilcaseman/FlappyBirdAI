# Set the minimum CMake version and
# the project's name
cmake_minimum_required(VERSION 3.16)
project(FlappyBirdAI)

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)

# Create the executable and add main.cpp
# as the source file
add_executable(FlappyBirdAI main.cpp)

# Customise LibRapid. See more options at
# https://librapid.rtfd.io/en/latest/cmakeIntegration.html
set(LIBRAPID_OPTIMISE_SMALL_ARRAYS ON)
set(LIBRAPID_USE_BLAS ON)
set(LIBRAPID_USE_OPENCL ON)
set(LIBRAPID_USE_CUDA ON)
set(LIBRAPID_USE_OMP ON)
set(LIBRAPID_FAST_MATH ON)
set(LIBRAPID_OPTIMISE_SMALL_ARRAYS ON)
set(LIBRAPID_CUDA_FLOAT_VECTOR_WIDTH 4)
set(LIBRAPID_CUDA_DOUBLE_VECTOR_WIDTH 4)
set(LIBRAPID_GET_FFTW OFF)
set(LIBRAPID_GET_BLAS OFF)

# Add surge as a subdirectory and link it
add_subdirectory(surge)
target_link_libraries(FlappyBirdAI PUBLIC surge)
