#pragma once

#include "circuit/snapshot/Snapshot.hpp"

extern "C" {
    void solve(circuit::snapshot::Snapshot snapshot);
    void example();
}