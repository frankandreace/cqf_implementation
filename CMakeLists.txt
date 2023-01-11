cmake_minimum_required(VERSION 3.19)

project(CQF)

include(CTest)
enable_testing()

add_compile_options(-Wall -Ofast -g -fpermissive) #-Werror

add_subdirectory(cqf)
add_subdirectory(test)
