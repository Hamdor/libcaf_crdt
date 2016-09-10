cmake_minimum_required(VERSION 2.8)
project(caf_replication C CXX)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB LIBCAF_REPLICATION_HDRS "caf/replication/*.hpp")
file(GLOB LIBCAF_REPLICATION_HDRS "caf/replication/cmrdt/*.hpp")
file(GLOB LIBCAF_REPLICATION_HDRS "caf/replication/detail/*.hpp")

add_custom_target(libcaf_replication)

# list cpp files excluding platform-dependent files
set (LIBCAF_REPLICATION_SRCS
     src/replicator.cpp
     src/replicator_actor.cpp
     src/replicator_hooks.cpp)

# build shared library if not compiling static only
if(NOT CAF_BUILD_STATIC_ONLY)
  add_library(libcaf_replication_shared SHARED ${LIBCAF_REPLICATION_SRCS}
              ${LIBCAF_REPLICATION_HDRS})
  target_link_libraries(libcaf_replication_shared ${LD_FLAGS}
                                                  ${CAF_LIBRARY_CORE}
                                                  ${CAF_LIBRARY_IO})
  set_target_properties(libcaf_replication_shared
                        PROPERTIES
                        SOVERSION "${CAF_VERSION}"
                        VERSION "${CAF_VERSION}"
                        OUTPUT_NAME caf_replication)
  if(NOT WIN32)
    install(TARGETS libcaf_replication_shared LIBRARY DESTINATION lib)
  endif()
endif()
# build static library only if --build-static or --build-static-only was set
if(CAF_BUILD_STATIC_ONLY OR CAF_BUILD_STATIC)
  add_library(libcaf_replication_static STATIC ${LIBCAF_REPLICATION_HDRS}
                                               ${LIBCAF_REPLICATION_SRCS})
  target_link_libraries(libcaf_replication_static ${LD_FLAGS}
                                                  ${CAF_LIBRARY_CORE_STATIC}
                                                  ${CAF_LIBRARY_IO_STATIC})
  set_target_properties(libcaf_replication_static PROPERTIES OUTPUT_NAME
                        caf_replication_static)
  install(TARGETS libcaf_replication_static ARCHIVE DESTINATION lib)
endif()
link_directories(${LD_DIRS})
include_directories(. ${INCLUDE_DIRS})
# install includes
install(DIRECTORY caf/ DESTINATION include/caf FILES_MATCHING PATTERN "*.hpp")
