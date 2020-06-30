include(ExternalProject)

include(${CMAKE_CURRENT_LIST_DIR}/External_CMakeAST.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/External_Catch.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/External_Expected.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/External_Fmt.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/External_polymorphic_value.cmake)

ExternalProject_Add(${PROJECT_NAME}
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
  CMAKE_ARGS
    -DBUILD_SUPERBUILD=OFF
    -DCatch2_DIR=${Catch2_DIR}
    -Dtl-expected_DIR=${tl-expected_DIR}
    -Dfmt_DIR=${fmt_DIR}
    -DCMakeAST_DIR=${CMakeAST_DIR}
    -Dpolymorphic_value_DIR=${polymorphic_value_DIR}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  SOURCE_DIR ${${PROJECT_NAME}_SOURCE_DIR}
  BINARY_DIR ${${PROJECT_NAME}_BINARY_DIR}/${PROJECT_NAME}-build
  DEPENDS CMakeAST Catch expected fmt polymorphic_value
)
