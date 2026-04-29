cmake_minimum_required(VERSION 3.20)
project(CircuitYard LANGUAGES CXX)

include(FetchContent)

fetchcontent_declare(
  corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
)

fetchcontent_makeavailable(corrosion)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)