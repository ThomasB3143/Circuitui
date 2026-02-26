#include "circuit/ecs/CircuitECS.hpp"
#include "circuit/ecs/Component.hpp"
#include "circuit/ecs/Node.hpp"
#include "circuit/ecs/Types.hpp"
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

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
    ElectricalComponent& comp = get_component(compID);

    // Get pointer to the terminal's node
    NodeId* termNode =
        (term == Terminal::Anode) ? &comp.anode : &comp.cathode;

    // Nothing to sever
    if (*termNode == INVALID_NODE)
        return;

    // Preserve node ID before modifying component
    NodeId nodeId = *termNode;
    Node& node = get_node(nodeId);

    // Disconnect component from node
    *termNode = INVALID_NODE;
    std::erase(node.connected_components, compID);

    // Node collapse logic
    if (node.connected_components.size() == 1) {
        // Only one component left -> node is invalid
        ComponentId lastCompId = node.connected_components.front();
        ElectricalComponent& lastComp = get_component(lastCompId);

        // Remove node from remaining component
        if (lastComp.anode == nodeId)
            lastComp.anode = INVALID_NODE;
        if (lastComp.cathode == nodeId)
            lastComp.cathode = INVALID_NODE;

        node.connected_components.clear();
        delete_node(nodeId);
    }
    else if (node.connected_components.empty()) {
        // Fully disconnected node
        delete_node(nodeId);
    }
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

void CircuitECS::connect_node(ComponentId compID, Terminal term, NodeId nodeID) {

	// Get component from comp
	ElectricalComponent& component = get_component(compID);

	// Get node from node
	Node& node_to_connect = get_node(nodeID);

	// Get pointer to node at term of comp
	NodeId* node_at_term = 
        (term == Terminal::Anode) ? &component.anode : &component.cathode;
	
	// Check if terminal is already connected
	if (*node_at_term != INVALID_NODE) {
		throw std::invalid_argument("Cannot connect a pre-connected terminal to a node");
	}

	// Set terminal value to node ID
	*node_at_term = nodeID;

	// Push component ID onto node's connected list
	node_to_connect.connected_components.push_back(compID);

	return;
}

void CircuitECS::merge_nodes(NodeId nodeID1, NodeId nodeID2) {

	// Check for merging of same node
	if (nodeID1 == nodeID2) {
		throw std::invalid_argument("Cannot merge a node with itself");
	}

	// Get nodes, connected_components lists and size
	// Start by assuming node1 is to grow and node2 is to merge
	NodeId growId = nodeID1;
	NodeId mergeId = nodeID2;

	Node* to_grow = &get_node(nodeID1);
	Node* to_merge = &get_node(nodeID2);

	std::vector<ComponentId>* grow_list = &to_grow->connected_components;
	std::vector<ComponentId>* merge_list = &to_merge->connected_components;

	size_t grow_size = grow_list->size();
	size_t merge_size = merge_list->size();

	if (grow_size < merge_size) {
		// Node1 has less components
		// Swap merge and grow nodes
		std::swap(growId, mergeId);
		std::swap(to_grow, to_merge);
		std::swap(grow_list, merge_list);
		std::swap(grow_size, merge_size);
	}

	// TODO
	// Change all instances of mergeId in components to growId
	for (const auto& compId : *merge_list) {
		ElectricalComponent& comp = get_component(compId);

		if (comp.anode == mergeId) comp.anode = growId;
		else if (comp.cathode == mergeId) comp.cathode = growId;
		else throw std::logic_error("Inconsistent component terminals and node lists");

	}

	// Transfer all components from merge_list to grow_list
	grow_list->reserve(grow_size + merge_size);
	grow_list->insert(grow_list->end(), 
		std::make_move_iterator(merge_list->begin()),
		std::make_move_iterator(merge_list->end()));

	// Delete merge_list
	merge_list->clear();
	delete_node(mergeId);

	return;
}

// Node Accessors

NodeId CircuitECS::get_node(ComponentId compID, Terminal term) {
	ElectricalComponent &comp = get_component(compID);
	return (term == Terminal::Anode) ? comp.anode : comp.cathode;
}

Node &CircuitECS::get_node(NodeId id) {
	return nodes_.at(id);
}

} // namespace circuit::ecs
