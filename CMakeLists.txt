cmake_minimum_required(VERSION 3.16.3)

project(Tonic)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_STANDARD 20)

add_executable(Tonic
        src/main.cpp
        src/Utils.cpp
        src/Tonic.cpp
        src/Tonic_FD.cpp
)


add_executable(BuildOracle
        src/main.cpp
        src/Utils.cpp
		src/Tonic.cpp
		src/Tonic_FD.cpp)

add_executable(DataPreprocessing
        src/main.cpp
        src/Utils.cpp
		src/Tonic.cpp
		src/Tonic_FD.cpp)

add_executable(RunExactAlgo
        src/main.cpp
		src/Tonic.cpp
		src/Tonic_FD.cpp
		src/Utils.cpp)

add_executable(CreateFDStream
        src/main.cpp
        src/Utils.cpp
	src/Tonic.cpp
	src/Tonic_FD.cpp)

target_include_directories(Tonic PRIVATE include)
target_include_directories(BuildOracle PRIVATE include)
target_include_directories(DataPreprocessing PRIVATE include)
target_include_directories(RunExactAlgo PRIVATE include)
target_include_directories(CreateFDStream PRIVATE include)
