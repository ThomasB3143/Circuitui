use nalgebra::DVector;
use CircuitData;
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
        let circuit = CircuitData::from_snapshot(snap);

        // Build z vector: node current injections, then source voltages
        let mut z = DVector::<f64>::zeros(
            circuit.ungrounded.len() + circuit.components.iter().filter(|x| x.type_ == 2).count(),
        );

        // Bottom entries: voltage of each voltage source
        for (i, cell) in circuit
            .components
            .iter()
            .filter(|x| x.type_ == 2)
            .enumerate()
        {
            z[circuit.ungrounded.len() + i] = cell.property;
        }

        // Top entries: current injections at each ungrounded node (type 3 current sources)
        for (j, node) in circuit.ungrounded.iter().enumerate() {
            for comp in circuit.components.iter().filter(|x| x.type_ == 3) {
                if comp.anode == *node as u32 {
                    z[j] -= comp.property;
                } else if comp.cathode == *node as u32 {
                    z[j] += comp.property;
                }
            }
        }

        // Solve A * x = z via LU decomposition
        let x = circuit
            .a_matrix
            .full_piv_lu()
            .solve(&z)
            .expect("Circuit matrix is singular — check for floating nodes");

        // Extract node voltages (first ungrounded.len() entries of x)
        let mut voltages: Vec<f64> = x.iter().take(circuit.ungrounded.len()).cloned().collect();

        let solution_count = voltages.len() as u32;

        // Leak the vec into a raw pointer for the C++ caller to own
        let voltages_ptr = voltages.as_mut_ptr();
        std::mem::forget(voltages);

        Solution {
            voltages: voltages_ptr,
            solution_count,
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
