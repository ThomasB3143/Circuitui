#[repr(C)]
pub struct Solution {
    pub voltages: *mut f64,
    pub solution_count: u32,
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

#[derive(Debug, PartialEq)]
pub struct CircuitData {
    pub components: Vec<Component>,
    pub node_count: usize,
}
