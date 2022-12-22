cmake_minimum_required(VERSION 3.5)
project(rucfs_test)

include_directories(
  ${CMAKE_SOURCE_DIR}
)

add_executable(${PROJECT_NAME}
  ../src/rucfs.c
  main.c
)
