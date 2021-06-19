cmake_minimum_required(VERSION 3.5.1)

project(FlashLoggergRPCServer C CXX)

include(common.cmake)

# Proto file
get_filename_component(fl_proto "flog.proto" ABSOLUTE)
get_filename_component(fl_proto_path "${fl_proto}" PATH)

# Generated sources
set(fl_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/flog.pb.cc")
set(fl_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/flog.pb.h")
set(fl_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/flog.grpc.pb.cc")
set(fl_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/flog.grpc.pb.h")
add_custom_command(
      OUTPUT "${fl_proto_srcs}" "${fl_proto_hdrs}" "${fl_grpc_srcs}" "${fl_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${fl_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${fl_proto}"
      DEPENDS "${fl_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
# fl_grpc_proto
add_library(fl_grpc_proto
  ${fl_grpc_srcs}
  ${fl_grpc_hdrs}
  ${fl_proto_srcs}
  ${fl_proto_hdrs})
target_link_libraries(fl_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets greeter_[async_](client|server)
add_executable(${PROJECT_NAME}_EXE "FLogAsyncServer.cpp")
target_link_libraries(${PROJECT_NAME}_EXE
    fl_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
