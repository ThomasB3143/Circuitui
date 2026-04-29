#pragma once
#include <cstdint>

namespace circuit::snapshot {

enum class SnapComponentType : std::uint8_t {
    Cell = 0,
    Resistor = 1,
    Capacitor = 2
};

struct Component {
    SnapComponentType type;

    double voltage;
    double current;
    double property;

    // Indices instead of pointers
    std::uint32_t anode;
    std::uint32_t cathode;
};

}