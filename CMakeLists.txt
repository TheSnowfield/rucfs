cmake_minimum_required(VERSION 3.5)
project(rucfs)

include_directories(
  ${CMAKE_SOURCE_DIR}/src
)

add_library(${PROJECT_NAME}
  src/rucfs.c
)
