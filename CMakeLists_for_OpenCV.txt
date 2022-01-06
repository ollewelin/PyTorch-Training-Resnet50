#file(GLOB imagenetSources *.cpp)
#file(GLOB imagenetIncludes *.h )

# require CMake 2.8 or greater
cmake_minimum_required(VERSION 2.8)



#------ Used when inside Docker container ----------
include_directories("/olle_test1/home/jetson/jetson-inference/utils/image")
include_directories("/olle_test1/home/jetson/jetson-inference/utils/video")
include_directories("/olle_test1/home/jetson/jetson-inference/utils/cuda")
include_directories("/olle_test1/home/jetson/jetson-inference/utils")
include_directories("/olle_test1/home/jetson/jetson-inference/c")
#---------------------------------------------------

#======= Used when build directly from source wihtout Docker image =========
include_directories("~/jetson-inference/build/aarch64/include")
include_directories("~/jetson-inference/build/aarch64/include/jetson-inference")
include_directories("~/jetson-inference/build/aarch64/include/jetson-utils")
#===========================================================================

# declare my-recognition project
project(imagenet2)
project(imagenet3)

if(MSVC)
    message(STATUS "MSVC DEFINED")
    message(STATUS $ENV{OPENCV_MSVC_DIR})
    find_package(OpenCV REQUIRED PATHS $ENV{OPENCV_MSVC_DIR} NO_DEFAULT_PATH)
else()
    find_package(OpenCV REQUIRED)
endif()

include_directories( ${OpenCV_INCLUDE_DIRS} .)
#add_library(gst_src gst_src.cpp)
#add_executable(main main.cpp)

if(MSVC)
    target_link_libraries( ${OpenCV_MSVC_LIBS})
    message(STATUS "MSVC LIBS LINKED.")
else()
 #   target_link_libraries(main ${OpenCV_LIBS} gst_src)
 #   target_link_libraries(gst_src ${OpenCV_LIBS})

endif()


# import jetson-inference and jetson-utils packages.
# note that if you didn't do "sudo make install"
# while building jetson-inference, this will error.
find_package(jetson-utils)
find_package(jetson-inference)
find_package(OpenCV REQUIRED)
# CUDA is required
find_package(CUDA)
# add directory for libnvbuf-utils to program
link_directories(/usr/lib/aarch64-linux-gnu/tegra)
# compile the my-recognition program
cuda_add_executable(imagenet2 imagenet2.cpp)
cuda_add_executable(imagenet3 imagenet3.cpp)
# link my-recognition to jetson-inference library
target_link_libraries(imagenet2 jetson-inference)
target_link_libraries(imagenet3 jetson-inference)
target_link_libraries(imagenet3 ${OpenCV_LIBS})
