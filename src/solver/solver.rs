use crate::ffi::Snapshot;
use crate::snapshot::CircuitData;
use crate::mna::build_system;

pub struct Solution {
    pub voltages: Vec<f64>,
}

pub fn solve_mna(snapshot: Snapshot) -> Solution {
    let data = unsafe { CircuitData::from_snapshot(snapshot) };

    let (g, b) = build_system(&data);

    let voltages = gaussian_elimination(g, b);

    Solution { voltages }
}