cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc
    GIT_TAG        v1.38.0
)
FetchContent_MakeAvailable(grpc)

set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP grpc++)
if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif()
