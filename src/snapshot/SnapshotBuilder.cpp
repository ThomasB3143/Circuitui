#include "circuit/snapshot/SnapshotBuilder.hpp"
#include "circuit/ecs/Types.hpp"
#include "circuit/snapshot/SnapComponent.hpp"
#include <cstdint>
#include <stdexcept>
#include <unordered_map>

namespace circuit::snapshot {

Snapshot SnapshotBuilder::build_snapshot(ecs::CircuitECS& ecs) {
    Snapshot out{};

    std::vector<Component> comp_vec;

    // Set up hash map from NodeID to index in node_vec
    std::unordered_map<ecs::NodeId, uint32_t> node_map;


    // Iterate through nodes
    uint32_t i = 0;
    for (auto node : ecs.get_alive_nodes()) {
        
        // Map ID to index
        uint32_t index = i;
        node_map[node.id] = index;
        i ++;
    }

    // Iterate through components
    for (auto comp : ecs.get_alive_components()) {

        Component c{};

        // Get component type
        switch (comp.type) {
            case ecs::ComponentType::Cell: 
                c.type = SnapComponentType::Cell;
                break;
            case ecs::ComponentType::Resistor:
                c.type = SnapComponentType::Resistor;
                break;
            case ecs::ComponentType::Capacitor:
                c.type = SnapComponentType::Capacitor;
                break;
            default:
                throw std::runtime_error("Unknown component type");
        }

        // Get current electrical properties
        c.voltage = comp.voltage;
        c.current = comp.current;
        c.property = comp.property;

        // Get node indexes through map
        c.anode = node_map[comp.anode];
        c.cathode = node_map[comp.cathode];

        comp_vec.push_back(c);
    }

    // Allocate final arrays
    out.component_count = comp_vec.size();
    out.node_count = i;

    out.components = new Component[out.component_count];

    // Copy
    std::copy(comp_vec.begin(), comp_vec.end(), out.components);

    return out;
}

void SnapshotBuilder::free_snapshot(Snapshot snapshot) {
    delete[] snapshot.components;
}

}