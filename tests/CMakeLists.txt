find_package(Catch2 REQUIRED)

add_executable(UnitTests
  main.cpp
)
target_link_libraries(UnitTests PRIVATE Catch2::Catch2)
