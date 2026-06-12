#pragma once

#include "circuit/snapshot/Snapshot.hpp"

extern "C" {
    void solve_mna(circuit::snapshot::Snapshot snapshot);
}