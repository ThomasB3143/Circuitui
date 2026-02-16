#include "circuit/ecs/CircuitECS.hpp"
#include "circuit/ecs/Node.hpp"
#include "circuit/ecs/Types.hpp"

namespace circuit::ecs {

// Component creation
ComponentId CircuitECS::create_component(ComponentType type, double property) {

    ComponentId id = static_cast<ComponentId>(components_.size());

    components_.push_back(ElectricalComponent{
        .id = id,
        .type = type,
        .property = property
    });

    return id;
}

// Component manipulation

void CircuitECS::connect_components(ComponentId cId1, ComponentId cId2,
    Terminal t1, Terminal t2) {

    ElectricalComponent& comp1 = get_component(cId1);
    ElectricalComponent& comp2 = get_component(cId2);

    // Retrieve pointer to node at terminal t1 of comp1
    NodeId node1 = (t1 == Terminal::Anode) ? comp1.anode : comp1.cathode;

    // Retrieve pointer to node at terminal t2 of comp2
    NodeId node2 = (t2 == Terminal::Anode) ? comp2.anode : comp2.cathode;

    if (node1 == INVALID_NODE && node2 == INVALID_NODE) {
        // Neither are connected
        // Create a new node connected to both
        create_node(cId1, cId2, t1, t2);
    } else if (node1 != INVALID_NODE && node2 != INVALID_NODE) {
        // Both are connected
        // Merge nodes
        merge_nodes(node1, node2);
    } else if (node1 == INVALID_NODE) {
        // Only node2 exists
        // Connect comp1 to node2
        connect_node(cId1, t1, node2);
    } else {
        // Only node1 exists
        // Connect comp2 to node1
        connect_node(cId2, t2, node1);
    }
}
void CircuitECS::sever_terminal(ComponentId compID, Terminal term) {
    // TODO
    return;
}


// Accessors

ElectricalComponent& CircuitECS::get_component(ComponentId id) {
    return components_.at(id);
}

// Node manipulation

NodeId CircuitECS::get_node(ComponentId compID, Terminal term) {
    ElectricalComponent& comp = get_component(compID);
    return (term == Terminal::Anode) ? comp.anode : comp.cathode;
}

Node& CircuitECS::get_node(NodeId id) {
    return nodes_.at(id);
}

NodeId CircuitECS::create_node(ComponentId c1, ComponentId c2,
                            Terminal t1, Terminal t2) {

    // Create new node
    NodeId id = static_cast<NodeId>(nodes_.size());
    nodes_.push_back(Node{
        .id = id,
        .connected_components = {c1, c2}
    });

    // Connect components to node
    ElectricalComponent& comp1 = get_component(c1);
    ElectricalComponent& comp2 = get_component(c2);
    NodeId* node1 = (t1 == Terminal::Anode) ? &comp1.anode : &comp1.cathode;
    NodeId* node2 = (t2 == Terminal::Anode) ? &comp2.anode : &comp2.cathode;
    *node1 = id;
    *node2 = id;

    return id;
}

void connect_node(ComponentId comp, Terminal term, NodeId node) {
    
}

void CircuitECS::merge_nodes(NodeId node1, NodeId node2) {
    // TODO
    return;
}

} // namespace circuit::ecs
