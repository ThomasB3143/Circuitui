use crate::ffi::Snapshot;
use crate::snapshot::CircuitData;
// use crate::mna::build_system;

pub struct Solution {
    pub voltages: Vec<f64>,
}

/*#[no_mangle]
pub extern "C" fn solve_mna(snapshot: Snapshot) -> Solution {
    let data = unsafe { CircuitData::from_snapshot(snapshot) };

    Solution { 1 }
}*/

#[no_mangle]
pub extern "C" fn example() {
    println!("Hello!");
}