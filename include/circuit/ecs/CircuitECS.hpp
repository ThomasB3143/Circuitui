#pragma once

#include <vector>
#include "Node.hpp"
#include "Component.hpp"

namespace circuit::ecs {

class CircuitECS {
public:
    CircuitECS() = default;

    // Node creation
    NodeId create_node();

    // Component creation
    ComponentId create_resistor(NodeId a, NodeId b, double resistance);
    ComponentId create_capacitor(NodeId a, NodeId b, double capacitance);
    ComponentId create_cell(NodeId a, NodeId b, double emf);

    // Accessors
    const Node& get_node(NodeId id) const;
    Node& get_node(NodeId id);

    const ElectricalComponent& get_component(ComponentId id) const;
    ElectricalComponent& get_component(ComponentId id);

    const std::vector<Node>& nodes() const noexcept;
    const std::vector<ElectricalComponent>& components() const noexcept;

private:
    ComponentId create_component(ComponentType type,
        NodeId a,
        NodeId b,
        double resistance,
        double capacitance,
        double emf
    );

    std::vector<Node> nodes_;
    std::vector<ElectricalComponent> components_;
};

} // namespace circuit::ecs
