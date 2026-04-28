#pragma once

#include <vector>
#include "Node.hpp"
#include "Component.hpp"
#include "Types.hpp"

namespace circuit::ecs {

class CircuitECS {
public:
    CircuitECS() = default;

    // Component creation and deletion
    ComponentId create_component(ComponentType type, double property);
    void delete_component(ComponentId id);

    // Component manipulation
    void connect_components(ComponentId cId1, ComponentId cId2,
                            Terminal t1, Terminal t2);
    void sever_terminal(ComponentId compID, Terminal term);

    // Component Accessors
    ElectricalComponent& get_component(ComponentId id);

    // Snapshot accessors
    std::vector<ElectricalComponent> get_alive_components();
    std::vector<Node> get_alive_nodes();

private:

    // Node creation and deletion
    NodeId create_node(ComponentId c1, ComponentId c2,
                            Terminal t1, Terminal t2);
    void delete_node(NodeId id);


    // Node manipulation    
    void connect_node(ComponentId compID, Terminal term, NodeId nodeID);
    void merge_nodes(NodeId nodeID1, NodeId nodeID2);

    // Node Accessors
    NodeId get_node(ComponentId compID, Terminal term);
    Node& get_node(NodeId cId1);

    std::vector<Node> nodes_; // list of nodes
    std::vector<NodeId> free_nodeIds_; // list of free node Ids (previously deleted)

    std::vector<ElectricalComponent> components_; // list of components
    std::vector<ComponentId> free_componentIds_; // list of free component Ids (previously deleted)

};

} // namespace circuit::ecs
