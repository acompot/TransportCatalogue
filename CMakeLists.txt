cmake_minimum_required(VERSION 3.5)

project(14FinalTransportCatalogue_p1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
set (proto
    "proto/transport_catalogue.proto"
    "proto/svg.proto"
    "proto/map_renderer.proto"
    "proto/graph.proto"
    "proto/transport_router.proto")

set (sources
    "src/json.cpp"
    "src/json_builder.cpp"
    "src/geo.cpp"
    "src/transport_catalogue.cpp"
    "src/request_handler.cpp"
    "src/svg.cpp"
    "src/map_renderer.cpp"
    "src/json_reader.cpp"
    "src/transport_catalogue.cpp"
    "src/transport_router.cpp"
    "src/serialization.cpp")

set (headers
    "inc/ranges.h"
    "inc/graph.h"
    "inc/json.h"
    "inc/json_builder.h"
    "inc/geo.h"
    "inc/transport_catalogue.h"
    "inc/request_handler.h"
    "inc/svg.h"
    "inc/map_renderer.h"
    "inc/json_reader.h"
    "inc/transport_catalogue.h"
    "inc/transport_router.h"
    "inc/serialization.h"
    "inc/router.h")

  protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto})

  add_executable(transport_catalogue  main.cpp ${sources} ${headers} ${proto} ${PROTO_SRCS} ${PROTO_HDRS})
  target_include_directories(transport_catalogue PRIVATE "inc")
  target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
  target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
  string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
  string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
  target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY_RELEASE}>" Threads::Threads)
include(CPack)
