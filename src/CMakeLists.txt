add_library(circuit)

target_include_directories(circuit
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_subdirectory(ecs)
add_subdirectory(snapshot)
add_subdirectory(solver)
