#pragma once
#include "Snapshot.hpp"
#include "circuit/ecs/CircuitECS.hpp"

namespace circuit::snapshot {

class SnapshotBuilder {
public:
    Snapshot build_snapshot(ecs::CircuitECS& ecs);
    void free_snapshot(Snapshot snapshot);
};

}