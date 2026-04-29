add_library(circuit)

target_compile_options(circuit PRIVATE -g -O0)

target_include_directories(circuit
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_subdirectory(ecs)
add_subdirectory(snapshot)
