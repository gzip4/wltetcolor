cmake_minimum_required (VERSION 3.8)

project ("wltetcolor" VERSION 1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory ("vendor")
add_subdirectory ("src")
