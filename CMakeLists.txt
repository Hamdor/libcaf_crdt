cmake_minimum_required(VERSION 2.8)
project(caf_crdt C CXX)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB LIBCAF_CRDT_HDRS "caf/crdt/*.hpp")
file(GLOB LIBCAF_CRDT_HDRS "caf/crdt/types/*.hpp")
file(GLOB LIBCAF_CRDT_HDRS "caf/crdt/detail/*.hpp")

add_custom_target(libcaf_crdt)

# list cpp files excluding platform-dependent files
set (LIBCAF_CRDT_SRCS
     src/replicator.cpp
     src/replicator_actor.cpp
     src/replicator_hooks.cpp)

# build shared library if not compiling static only
if(NOT CAF_BUILD_STATIC_ONLY)
  add_library(libcaf_crdt_shared SHARED ${LIBCAF_CRDT_SRCS}
              ${LIBCAF_CRDT_HDRS})
  target_link_libraries(libcaf_crdt_shared ${LD_FLAGS}
                                           ${CAF_LIBRARY_CORE}
                                           ${CAF_LIBRARY_IO})
  set_target_properties(libcaf_crdt_shared
                        PROPERTIES
                        SOVERSION "${CAF_VERSION}"
                        VERSION "${CAF_VERSION}"
                        OUTPUT_NAME caf_crdt)
  if(NOT WIN32)
    install(TARGETS libcaf_crdt_shared LIBRARY DESTINATION lib)
  endif()
endif()
# build static library only if --build-static or --build-static-only was set
if(CAF_BUILD_STATIC_ONLY OR CAF_BUILD_STATIC)
  add_library(libcaf_crdt_static STATIC ${LIBCAF_CRDT_HDRS}
                                        ${LIBCAF_CRDT_SRCS})
  target_link_libraries(libcaf_crdt_static ${LD_FLAGS}
                                                  ${CAF_LIBRARY_CORE_STATIC}
                                                  ${CAF_LIBRARY_IO_STATIC})
  set_target_properties(libcaf_crdt_static PROPERTIES OUTPUT_NAME
                        caf_crdt_static)
  install(TARGETS libcaf_crdt_static ARCHIVE DESTINATION lib)
endif()
link_directories(${LD_DIRS})
include_directories(. ${INCLUDE_DIRS})
# install includes
install(DIRECTORY caf/ DESTINATION include/caf FILES_MATCHING PATTERN "*.hpp")
