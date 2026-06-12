corrosion_import_crate(
    MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml
)

target_sources(circuit
    PRIVATE
        solver_bridge.cpp
)

target_link_libraries(circuit
    PUBLIC
        solver
)