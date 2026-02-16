#pragma once

#include <vector>
#include "Node.hpp"
#include "Component.hpp"
#include "Types.hpp"

namespace circuit::ecs {

class CircuitECS {
public:
    CircuitECS() = default;

    // Component creation
    ComponentId create_component(ComponentType type, double property);

    // Component manipulation
    void connect_components(ComponentId cId1, ComponentId cId2,
                            Terminal t1, Terminal t2);
    void sever_terminal(ComponentId compID, Terminal term);

    // Accessors
    ElectricalComponent& get_component(ComponentId id);

private:

    // Node manipulation
    NodeId get_node(ComponentId compID, Terminal term);
    Node& get_node(NodeId cId1);
    NodeId create_node(ComponentId c1, ComponentId c2,
                            Terminal t1, Terminal t2);
    void connect_node(ComponentId comp, Terminal term, NodeId node);
    void merge_nodes(NodeId node1, NodeId node2);

    std::vector<Node> nodes_;
    std::vector<ElectricalComponent> components_;
};

} // namespace circuit::ecs
