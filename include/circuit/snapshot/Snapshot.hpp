#pragma once
#include <cstdint>
#include "SnapComponent.hpp"

namespace circuit::snapshot {

struct Snapshot {
    Component* components;
    std::uint32_t component_count;

    std::uint32_t node_count;
};

}