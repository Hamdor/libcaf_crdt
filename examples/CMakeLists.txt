cmake_minimum_required(VERSION 2.8)
project(caf_examples_replication CXX)

add_custom_target(replication_examples)
include_directories(${LIBCAF_INCLUDE_DIRS})
if(${CMAKE_SYSTEM_NAME} MATCHES "Window")
  set(WSLIB -lws2_32)
else()
  set(WSLIB)
endif()
macro(add name folder)
  add_executable(${name} ${folder}/${name}.cpp ${ARGN})
  target_link_libraries(${name}
                        ${LD_FLAGS}
                        ${CAF_LIBRARIES}
                        ${PTHREAD_LIBRARIES}
                        ${WSLIB})
  add_dependencies(${name} all_examples)
endmacro()
add(worker .)
add(parse_uri .)
