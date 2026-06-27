#[no_mangle]
pub extern "C" fn solution_from_snapshot(snap: Snapshot) -> Solution {
    Solution::from_snapshot(snap)
}

#[repr(C)]
pub struct Solution {
    pub voltages: *mut f64,
    pub solution_count: u32,
}

impl Solution {
    pub fn from_snapshot(snap: Snapshot) -> Solution {
        // ENTER LOGIC
        // Fake solution so it compiles
        let mut voltage = 1.0;
        Solution {
            voltages: &mut voltage,
            solution_count: 1,
        }
    }
}

#[repr(C)]
#[derive(Clone, Debug, PartialEq)]
pub struct Component {
    pub type_: u8,
    pub voltage: f64,
    pub current: f64,
    pub property: f64,
    pub anode: u32,
    pub cathode: u32,
}

#[repr(C)]
pub struct Snapshot {
    pub components: *mut Component,
    pub component_count: u32,
    pub node_count: u32,
}
