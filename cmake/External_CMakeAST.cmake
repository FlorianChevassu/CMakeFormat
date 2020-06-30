ExternalProject_Add(CMakeAST
  #GIT_REPOSITORY https://github.com/FlorianChevassu/CMakeAST.git
  #GIT_TAG master
  SOURCE_DIR C:/Work/Perso/CMakeTools/CMakeAST/CMakeAST
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND "" # installation is done via the superbuild.
  CMAKE_CACHE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/CMakeAST-install
    -DBUILD_TESTING:BOOL=OFF
)
set(CMakeAST_DIR ${CMAKE_CURRENT_BINARY_DIR}/CMakeAST-install/lib/cmake/CMakeAST)
