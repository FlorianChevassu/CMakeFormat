cmake_minimum_required(VERSION 3.14)

project(CMakeFormat LANGUAGES CXX)

option(BUILD_SUPERBUILD "Build CMakeFormat using the Superbuild." ON)
if(BUILD_SUPERBUILD)
  include(cmake/Superbuild.cmake)
  return()
endif()

add_subdirectory(src)

option(BUILD_TESTING "Build tests" ON)
if (BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

export(TARGETS CMakeFormat NAMESPACE CMakeFormat:: FILE CMakeFormatTargets.cmake)

install(TARGETS CMakeFormat
        EXPORT CMakeFormatTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

install(EXPORT CMakeFormatTargets
        FILE CMakeFormatTargets.cmake
        NAMESPACE CMakeFormat::
        DESTINATION lib/cmake/CMakeFormat
)

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/CMakeFormatConfig.cmake.in CMakeFormatConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/CMakeFormatConfig.cmake"
        DESTINATION lib/cmake/CMakeFormat
)
