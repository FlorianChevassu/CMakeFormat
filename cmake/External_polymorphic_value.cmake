ExternalProject_Add(polymorphic_value
  GIT_REPOSITORY https://github.com/jbcoe/polymorphic_value.git
  GIT_TAG master # version 1.3.0 does not work with CMake
  CMAKE_CACHE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/polymorphic_value-install
)
set(polymorphic_value_DIR ${CMAKE_CURRENT_BINARY_DIR}/polymorphic_value-install/lib/cmake/polymorphic_value)
