#include "circuit/ecs/CircuitECS.hpp"
#include "circuit/ecs/Component.hpp"
#include "circuit/ecs/Node.hpp"
#include "circuit/ecs/Types.hpp"
#include <stdexcept>

namespace circuit::ecs {

// Component creation and deletion
ComponentId CircuitECS::create_component(ComponentType type, double property) {

	ComponentId id;

	// Check for tombstones
	if (!free_componentIds_.empty()) {
		// Tombstone exists!
		id = free_componentIds_.back();
		free_componentIds_.pop_back();
	} else {
		// No tombstones
		// ID is the size of components_ (just appended to the end)
		id = static_cast<ComponentId>(components_.size());
		components_.push_back(ElectricalComponent{});
	}

	components_[id] = ElectricalComponent{
	.id = id, .type = type, .property = property, .alive = true};

	return id;
}

void CircuitECS::delete_component(ComponentId id) {

	// Check if the component is not alive (already deleted)
	ElectricalComponent &to_delete = get_component(id);
	if (!to_delete.alive) {
		throw std::invalid_argument("Cannot delete a dead component");
	}
	// Mark as not alive
	to_delete.alive = false;

	// Sever terminals
	if (to_delete.anode != INVALID_NODE) {
		sever_terminal(id, Terminal::Anode);
	}
	if (to_delete.cathode != INVALID_NODE) {
		sever_terminal(id, Terminal::Cathode);
	}

	free_componentIds_.push_back(id);
}

// Component manipulation

void CircuitECS::connect_components(ComponentId cId1, ComponentId cId2,
	Terminal t1, Terminal t2) {

	ElectricalComponent &comp1 = get_component(cId1);
	ElectricalComponent &comp2 = get_component(cId2);

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
	// Retrieve component
	ElectricalComponent &comp = get_component(compID);

	// Retrieve node id
	NodeId *nodeID = (term == Terminal::Anode) ? &comp.anode : &comp.cathode;

	// Check if terminal actually has a node
	if (*nodeID == INVALID_NODE) {
		throw std::invalid_argument("Cannot sever a terminal with no node");
	}

	// Retrieve node
	Node &node = get_node(*nodeID);

	// Remove nodeID from component terminal
	*nodeID = INVALID_NODE;
	// Remove component from node's component list
	std::erase(node.connected_components, compID);

	// Check number of connected components post-sever:
	// > 1	 Just remove component from component list
	// <= 1 	Delete node (they require 2 connected components to exist)
	if (node.connected_components.size() <= 1) {
		ComponentId last_compID = node.connected_components.front();
		ElectricalComponent &last_comp = get_component(last_compID);
		NodeId *last_term =
		(last_comp.anode == *nodeID) ? &last_comp.anode : &last_comp.cathode;
		// YOU WERE HERE, REMOVE THE NODE FROM THE LAST COMPONENT AND DELETE THE
		// NODE
	}
	return;
}

// Accessors

ElectricalComponent &CircuitECS::get_component(ComponentId id) {
	return components_.at(id);
}

// Node creation and deletion

NodeId CircuitECS::create_node(ComponentId c1, ComponentId c2, Terminal t1,
	Terminal t2) {

	NodeId id;

	// Check for tombstones
	if (!free_nodeIds_.empty()) {
		// Tombstone exists!
		id = free_nodeIds_.back();
		free_nodeIds_.pop_back();
	} else {
		// No tombstones
		// ID is the size of nodes_ (just appended to the end)
		id = static_cast<ComponentId>(nodes_.size());
		nodes_.push_back(Node{});
	}

	nodes_[id] = Node{.id = id, .connected_components = {c1, c2}, .alive = true};

	// Connect components to node
	ElectricalComponent &comp1 = get_component(c1);
	ElectricalComponent &comp2 = get_component(c2);
	NodeId *node1 = (t1 == Terminal::Anode) ? &comp1.anode : &comp1.cathode;
	NodeId *node2 = (t2 == Terminal::Anode) ? &comp2.anode : &comp2.cathode;
	*node1 = id;
	*node2 = id;

	return id;
}

void CircuitECS::delete_node(NodeId id) {
	// Check if the node is not alive (already deleted)
	// Check if node has connected components
	Node &to_delete = get_node(id);
	if (!to_delete.alive) {
		throw std::invalid_argument("Cannot delete a dead node");
	} else if (!to_delete.connected_components.empty()) {
		throw std::invalid_argument("Cannot delete nodes with connected components");
	} else {
		// Mark as not alive
		to_delete.alive = false;
		free_nodeIds_.push_back(id);
	}
}

// Node manipulation

void connect_node(ComponentId comp, Terminal term, NodeId node) {}

void CircuitECS::merge_nodes(NodeId node1, NodeId node2) {
	// TODO
	return;
}

// Node Accessors

NodeId CircuitECS::get_node(ComponentId compID, Terminal term) {
	ElectricalComponent &comp = get_component(compID);
	return (term == Terminal::Anode) ? comp.anode : comp.cathode;
}

Node &CircuitECS::get_node(NodeId id) { return nodes_.at(id); }

} // namespace circuit::ecs
