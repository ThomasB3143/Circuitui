#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>
#include "circuit/ecs/CircuitECS.hpp"
#include "circuit/ecs/Component.hpp"
#include "circuit/ecs/Types.hpp"

using namespace circuit::ecs;

TEST(CircuitECS, CreateResistor) {
    CircuitECS ecs;

    ComponentId r = ecs.create_component(ComponentType::Resistor, 100.0);

    auto& comp = ecs.get_component(r);
    EXPECT_TRUE(comp.alive);
    EXPECT_EQ(comp.type, ComponentType::Resistor);
}

TEST(CircuitECS, CreateCell) {
    CircuitECS ecs;

    ComponentId c = ecs.create_component(ComponentType::Cell, 50.0);

    auto& comp = ecs.get_component(c);
    EXPECT_TRUE(comp.alive);
    EXPECT_EQ(comp.type, ComponentType::Cell);
}

TEST(CircuitECS, CreateCapacitor) {
    CircuitECS ecs;

    ComponentId c = ecs.create_component(ComponentType::Capacitor, 25.0);

    auto& comp = ecs.get_component(c);
    EXPECT_TRUE(comp.alive);
    EXPECT_EQ(comp.type, ComponentType::Capacitor);
}

TEST(CircuitECS, CreateMultipleComponents) {
    CircuitECS ecs;

    ComponentId c0 = ecs.create_component(
        ComponentType::Cell , 10);
    ComponentId c1 = ecs.create_component(
        ComponentType::Capacitor , 10);
    ComponentId c2 = ecs.create_component(
        ComponentType::Resistor , 10);
    
    EXPECT_EQ(c0, 0);
    EXPECT_EQ(c1, 1);
    EXPECT_EQ(c2, 2);
}

TEST(CircuitECS, DeleteComponent) {
    CircuitECS ecs;

    ComponentId c0 = ecs.create_component(
        ComponentType::Cell , 10);
    ComponentId c1 = ecs.create_component(
        ComponentType::Capacitor , 10);
    ComponentId c2 = ecs.create_component(
        ComponentType::Resistor , 10);

    ecs.delete_component(c1);

    auto& dead_comp = ecs.get_component(c1);

    EXPECT_FALSE(dead_comp.alive);

    EXPECT_THROW(
        ecs.delete_component(c1);
    ,std::invalid_argument);

}

TEST(CircuitECS, CreateComponentWithTombstone) {
    CircuitECS ecs;

    ComponentId c0 = ecs.create_component(
        ComponentType::Cell , 10);
    ComponentId c1 = ecs.create_component(
        ComponentType::Capacitor , 10);
    ComponentId c2 = ecs.create_component(
        ComponentType::Resistor , 10);

    ecs.delete_component(c1);

    ComponentId c_replaced = ecs.create_component(ComponentType::Cell, 5);

    EXPECT_EQ(c_replaced, 1);

    auto comp = ecs.get_component(c_replaced);
    EXPECT_TRUE(comp.alive);
    EXPECT_EQ(comp.id, 1);
    
    ComponentId c3 = ecs.create_component(ComponentType::Resistor, 5);

    auto comp3 = ecs.get_component(c3);
    EXPECT_EQ(comp3.id, 3);
}

TEST(CircuitECS, BasicConnect) {
    CircuitECS ecs;

    ComponentId c0 = ecs.create_component(
        ComponentType::Cell , 10);
    ComponentId c1 = ecs.create_component(
        ComponentType::Capacitor , 10);

    ecs.connect_components(
        c0, c1, Terminal::Anode, Terminal::Anode);

    ecs.connect_components(
        c0, c1, Terminal::Cathode, Terminal::Cathode);

    ElectricalComponent comp0 = ecs.get_component(c0);
    ElectricalComponent comp1 = ecs.get_component(c1);

    EXPECT_EQ(comp0.anode, comp1.anode);
    EXPECT_EQ(comp0.cathode, comp1.cathode);

    EXPECT_EQ(comp0.anode, 0);
    EXPECT_EQ(comp0.cathode, 1);
}

TEST(CircuitECS, ConnectIntoExistingNode) {
    CircuitECS ecs;

    ComponentId a = ecs.create_component(ComponentType::Cell, 10);
    ComponentId b = ecs.create_component(ComponentType::Resistor, 10);
    ComponentId c = ecs.create_component(ComponentType::Capacitor, 10);

    // a.anode <-> b.anode
    ecs.connect_components(a, b, Terminal::Anode, Terminal::Anode);

    // c.anode joins the same node
    ecs.connect_components(c, a, Terminal::Anode, Terminal::Anode);

    auto& compA = ecs.get_component(a);
    auto& compB = ecs.get_component(b);
    auto& compC = ecs.get_component(c);

    EXPECT_NE(compA.anode, INVALID_NODE);
    EXPECT_EQ(compA.anode, compB.anode);
    EXPECT_EQ(compA.anode, compC.anode);
}

TEST(CircuitECS, MergeNodesThroughConnection) {
    CircuitECS ecs;

    ComponentId a = ecs.create_component(ComponentType::Cell, 10);
    ComponentId b = ecs.create_component(ComponentType::Resistor, 10);
    ComponentId c = ecs.create_component(ComponentType::Capacitor, 10);
    ComponentId d = ecs.create_component(ComponentType::Resistor, 10);

    // Two independent nodes
    ecs.connect_components(a, b, Terminal::Anode, Terminal::Anode);
    ecs.connect_components(c, d, Terminal::Anode, Terminal::Anode);

    NodeId node1 = ecs.get_component(a).anode;
    NodeId node2 = ecs.get_component(c).anode;

    ASSERT_NE(node1, node2);

    // Merge nodes
    ecs.connect_components(b, c, Terminal::Anode, Terminal::Anode);

    auto& compA = ecs.get_component(a);
    auto& compB = ecs.get_component(b);
    auto& compC = ecs.get_component(c);
    auto& compD = ecs.get_component(d);

    EXPECT_EQ(compA.anode, compB.anode);
    EXPECT_EQ(compA.anode, compC.anode);
    EXPECT_EQ(compA.anode, compD.anode);
}

TEST(CircuitECS, SeverSingleTerminal) {
    CircuitECS ecs;

    ComponentId a = ecs.create_component(ComponentType::Cell, 10);
    ComponentId b = ecs.create_component(ComponentType::Resistor, 10);

    ecs.connect_components(a, b, Terminal::Anode, Terminal::Anode);

    ecs.sever_terminal(a, Terminal::Anode);

    auto& compA = ecs.get_component(a);
    auto& compB = ecs.get_component(b);

    EXPECT_EQ(compA.anode, INVALID_NODE);
    EXPECT_EQ(compB.anode, INVALID_NODE);
}

TEST(CircuitECS, SeverOneOfManyConnections) {
    CircuitECS ecs;

    ComponentId a = ecs.create_component(ComponentType::Cell, 10);
    ComponentId b = ecs.create_component(ComponentType::Resistor, 10);
    ComponentId c = ecs.create_component(ComponentType::Capacitor, 10);

    ecs.connect_components(a, b, Terminal::Anode, Terminal::Anode);
    ecs.connect_components(c, a, Terminal::Anode, Terminal::Anode);

    NodeId sharedNode = ecs.get_component(a).anode;

    ecs.sever_terminal(a, Terminal::Anode);

    auto& compA = ecs.get_component(a);
    auto& compB = ecs.get_component(b);
    auto& compC = ecs.get_component(c);

    EXPECT_EQ(compA.anode, INVALID_NODE);
    EXPECT_EQ(compB.anode, sharedNode);
    EXPECT_EQ(compC.anode, sharedNode);
}

TEST(CircuitECS, DeleteComponentSeversConnections) {
    CircuitECS ecs;

    ComponentId a = ecs.create_component(ComponentType::Cell, 10);
    ComponentId b = ecs.create_component(ComponentType::Resistor, 10);

    ecs.connect_components(a, b, Terminal::Cathode, Terminal::Cathode);

    ecs.delete_component(a);

    auto& compA = ecs.get_component(a);
    auto& compB = ecs.get_component(b);

    EXPECT_FALSE(compA.alive);
    EXPECT_EQ(compB.cathode, INVALID_NODE);
}

TEST(CircuitECS, TestAliveAccessors) {
    CircuitECS ecs;

    ComponentId a = ecs.create_component(ComponentType::Cell, 10);
    ComponentId b = ecs.create_component(ComponentType::Resistor, 10);
    ComponentId c = ecs.create_component(ComponentType::Capacitor, 10);

    ecs.connect_components(a, b, Terminal::Anode, Terminal::Anode);
    ecs.connect_components(c, a, Terminal::Anode, Terminal::Anode);

    ecs.delete_component(a);
    ecs.delete_component(b);

    std::vector<ElectricalComponent> alive_comp = ecs.get_alive_components();
    std::vector<Node> alive_node = ecs.get_alive_nodes();

    EXPECT_EQ(alive_comp.size(), 1);
    EXPECT_EQ(alive_node.size(), 0);
    EXPECT_EQ(alive_comp[0].id, 2);
}