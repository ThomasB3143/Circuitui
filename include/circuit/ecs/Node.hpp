#pragma once

#include <vector>
#include "Types.hpp"

namespace circuit::ecs {

struct Node {

    NodeId id = INVALID_NODE;

    bool ground = false;

    // All components connected to this node
    std::vector<ComponentId> connected_components;

    // Used for tombstone-based deletion
    bool alive = true;
};

} // namespace circuit::ecs
