use crate::ffi::{Component, Snapshot};
use std::slice;

pub struct CircuitData {
    pub components: Vec<Component>,
    pub node_count: usize,
}

impl CircuitData {
    pub unsafe fn from_snapshot(snapshot: Snapshot) -> Self {
        let comps = slice::from_raw_parts(
            snapshot.components,
            snapshot.component_count as usize,
        );

        Self {
            components: comps.to_vec(),
            node_count: snapshot.node_count as usize,
        }
    }
}