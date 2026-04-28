#pragma once
#include <cstdint>

namespace circuit::snapshot {

enum class ComponentType : std::uint8_t {
    Cell = 0,
    Resistor = 1,
    Capacitor = 2
};

struct Component {
    ComponentType type;

    double voltage;
    double current;
    double property;

    // Indices instead of pointers
    std::uint32_t anode;
    std::uint32_t cathode;
};

}