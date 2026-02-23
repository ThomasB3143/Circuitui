#pragma once

#include "Types.hpp"

namespace circuit::ecs {

struct ElectricalComponent {

    ComponentId id = INVALID_COMPONENT;
    ComponentType type = ComponentType::Resistor;

    NodeId anode = INVALID_NODE;
    NodeId cathode = INVALID_NODE;

    // State variables (written by solver)
    double voltage = 0.0;
    double current = 0.0;

    // Property (used for resistance, emf, capacitance depending on type)
    double property = 0.0;

    // Used for tombstone-based deletion
    bool alive = true;
};

} // namespace circuit::ecs
