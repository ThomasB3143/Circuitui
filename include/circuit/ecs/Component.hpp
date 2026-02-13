#pragma once

#include "Types.hpp"

namespace circuit::ecs {

struct ElectricalComponent {
    ComponentId id = INVALID_COMPONENT;
    ComponentType type = ComponentType::Resistor;

    NodeId node_a = INVALID_NODE;
    NodeId node_b = INVALID_NODE;

    // State variables (written by solver)
    double voltage = 0.0;
    double current = 0.0;

    // Properties (used depending on type)
    double resistance = 0.0;   // for resistor
    double capacitance = 0.0;  // for capacitor
    double emf = 0.0;          // for cell
};

} // namespace circuit::ecs
