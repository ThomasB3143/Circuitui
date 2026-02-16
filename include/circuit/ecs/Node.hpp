#pragma once

#include <vector>
#include "Types.hpp"

namespace circuit::ecs {

struct Node {

    NodeId id = INVALID_NODE;

    // All components connected to this node
    std::vector<ComponentId> connected_components;
};

} // namespace circuit::ecs
