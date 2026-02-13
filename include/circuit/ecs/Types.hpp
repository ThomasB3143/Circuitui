#pragma once

#include <cstdint>

namespace circuit::ecs {

using NodeId = std::uint32_t;
using ComponentId = std::uint32_t;

constexpr NodeId INVALID_NODE = UINT32_MAX;
constexpr ComponentId INVALID_COMPONENT = UINT32_MAX;

enum class ComponentType : std::uint8_t {
    Cell,
    Resistor,
    Capacitor
};

} // namespace circuit::ecs
