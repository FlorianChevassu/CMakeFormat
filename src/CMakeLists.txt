find_package(CMakeAST REQUIRED)
find_package(polymorphic_value REQUIRED)

add_executable(CMakeFormat main.cpp)

target_include_directories(CMakeFormat PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_compile_features(CMakeFormat PUBLIC cxx_std_17)

target_link_libraries(CMakeFormat
  PUBLIC
    CMakeAST::CMakeAST
)
