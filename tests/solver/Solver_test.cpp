#include <cassert>
#include <gtest/gtest.h>
#include "circuit/solver/solver_bridge.hpp"
#include "circuit/snapshot/SnapshotBuilder.hpp"
#include "circuit/ecs/CircuitECS.hpp"

using namespace circuit::snapshot;
using namespace circuit::ecs;

TEST(Solver, basicTest) {

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

    // Run rust function
    solve_mna(snap);

}