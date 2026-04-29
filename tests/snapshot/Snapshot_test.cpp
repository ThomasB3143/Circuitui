#include <cassert>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>
#include "circuit/ecs/CircuitECS.hpp"
#include "circuit/ecs/Component.hpp"
#include "circuit/ecs/Types.hpp"
#include "circuit/snapshot/SnapComponent.hpp"
#include "circuit/snapshot/SnapNode.hpp"
#include "circuit/snapshot/Snapshot.hpp"
#include "circuit/snapshot/SnapshotBuilder.hpp"

using namespace circuit::ecs;
using namespace circuit::snapshot;

TEST(Snapshot, StandardSnapshot) {
    CircuitECS ecs;

    ComponentId c0 = ecs.create_component(
        ComponentType::Cell , 10);
    ComponentId r0 = ecs.create_component(
        ComponentType::Resistor , 10);

    ecs.connect_components(
        c0, r0, Terminal::Anode, Terminal::Anode);

    ecs.connect_components(
        c0, r0, Terminal::Cathode, Terminal::Cathode);

    SnapshotBuilder builder;
    Snapshot snap = builder.build_snapshot(ecs);
    
    // Assert counts
    ASSERT_EQ(snap.component_count, 2);
    ASSERT_EQ(snap.node_count, 2);

    Component comp0 = snap.components[0];

    // Assert node to index mapping
    ASSERT_EQ(comp0.anode, 0);
    ASSERT_EQ(comp0.cathode, 1);

    // Assert component properties
    ASSERT_EQ(comp0.voltage, 0);
    ASSERT_EQ(comp0.current, 0);
    ASSERT_EQ(comp0.property, 10);
    ASSERT_EQ(comp0.type, SnapComponentType::Cell);


    Component comp1 = snap.components[1];

    // Assert node to index mapping
    ASSERT_EQ(comp1.anode, 0);
    ASSERT_EQ(comp1.cathode, 1);

    // Assert component properties
    ASSERT_EQ(comp1.voltage, 0);
    ASSERT_EQ(comp1.current, 0);
    ASSERT_EQ(comp1.property, 10);
    ASSERT_EQ(comp1.type, SnapComponentType::Resistor);

}

TEST(Snapshot, SnapshotDeletions) {
    CircuitECS ecs;

    ComponentId c0 = ecs.create_component(
        ComponentType::Cell , 10);
    ComponentId r0 = ecs.create_component(
        ComponentType::Resistor , 10);
    ComponentId c1 = ecs.create_component(
        ComponentType::Cell , 10);
    ComponentId r1 = ecs.create_component(
        ComponentType::Resistor , 10);

    ecs.connect_components(
        c0, r0, Terminal::Anode, Terminal::Anode);
    ecs.connect_components(
        c0, r0, Terminal::Cathode, Terminal::Cathode);
    ecs.connect_components(
        c0, c1, Terminal::Anode, Terminal::Anode);
    ecs.connect_components(
        c0, c1, Terminal::Cathode, Terminal::Cathode);
    ecs.connect_components(
    r0, r1, Terminal::Anode, Terminal::Anode);
    ecs.connect_components(
        r0, r1, Terminal::Cathode, Terminal::Cathode);

    ecs.delete_component(c0);
    ecs.delete_component(r1);

    SnapshotBuilder builder;
    Snapshot snap = builder.build_snapshot(ecs);
    
    // Assert counts
    ASSERT_EQ(snap.component_count, 2);
    ASSERT_EQ(snap.node_count, 2);

    Component comp0 = snap.components[0];

    // Assert node to index mapping
    ASSERT_EQ(comp0.anode, 0);
    ASSERT_EQ(comp0.cathode, 1);

    // Assert component properties
    ASSERT_EQ(comp0.voltage, 0);
    ASSERT_EQ(comp0.current, 0);
    ASSERT_EQ(comp0.property, 10);
    ASSERT_EQ(comp0.type, SnapComponentType::Resistor);


    Component comp1 = snap.components[1];

    // Assert node to index mapping
    ASSERT_EQ(comp1.anode, 0);
    ASSERT_EQ(comp1.cathode, 1);

    // Assert component properties
    ASSERT_EQ(comp1.voltage, 0);
    ASSERT_EQ(comp1.current, 0);
    ASSERT_EQ(comp1.property, 10);
    ASSERT_EQ(comp1.type, SnapComponentType::Cell);

}