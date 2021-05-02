file(GLOB imagenetSources *.cpp)
file(GLOB imagenetIncludes *.h )

# require CMake 2.8 or greater
cmake_minimum_required(VERSION 2.8)
include_directories("/olle_test1/home/jetson/jetson-inference/utils/image")
include_directories("/olle_test1/home/jetson/jetson-inference/utils/video")
include_directories("/olle_test1/home/jetson/jetson-inference/utils/cuda")
include_directories("/olle_test1/home/jetson/jetson-inference/utils")
include_directories("/olle_test1/home/jetson/jetson-inference/c")

# declare my-recognition project
project(imagenet2)

# import jetson-inference and jetson-utils packages.
# note that if you didn't do "sudo make install"
# while building jetson-inference, this will error.
find_package(jetson-utils)
find_package(jetson-inference)

# CUDA is required
find_package(CUDA)

# compile the my-recognition program
cuda_add_executable(imagenet2 imagenet2.cpp)

# link my-recognition to jetson-inference library
target_link_libraries(imagenet2 jetson-inference)
