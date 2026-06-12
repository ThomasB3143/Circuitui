pub mod ffi;

use ffi::{CircuitData, Snapshot, Solution};
use std::slice;
extern crate queues;
use queues::*;

#[no_mangle]
pub extern "C" fn solve_mna(snap: Snapshot) -> Solution {
    // Convert to CircuitData
    let circuit = circuitdata_from_snapshot(snap);

    //
    let ground_mask = ground_nodes(circuit);

    // MOCK RETURN
    let mut voltage = 1.0;
    let voltages: *mut f64 = &mut voltage;
    let solution_count: u32 = 2;
    Solution {
        voltages,
        solution_count,
    }
}

fn circuitdata_from_snapshot(snapshot: Snapshot) -> CircuitData {
    unsafe {
        let comps = slice::from_raw_parts(snapshot.components, snapshot.component_count as usize); // Turns a pointer and size into a full list

        CircuitData {
            components: comps.to_vec(),
            node_count: snapshot.node_count as usize,
        }
    }
}

fn ground_nodes(circuit: CircuitData) -> Vec<bool> {
    let mut visited = vec![false; circuit.node_count];
    let mut grounded = vec![false; circuit.node_count];

    // Repeatedly start BFS from an unvisited node
    while !visited.iter().all(|&x| x == true) {
        let mut bfs_queue = queue![visited.iter().position(|&x| x == false).unwrap()];
        grounded[bfs_queue.peek().unwrap()] = true;
        while let Ok(current_node) = bfs_queue.remove() {
            visited[current_node] = true;
            println!("CURRENT NODE: {}", current_node);

            // Go through every component where the anode or cathode is *current_node*
            for comp in circuit
                .components
                .iter()
                .filter(|x| x.anode == current_node as u32)
            {
                if !visited[comp.cathode as usize] {
                    bfs_queue.add(comp.cathode as usize).unwrap();
                }
            }
            for comp in circuit
                .components
                .iter()
                .filter(|x| x.cathode == current_node as u32)
            {
                if !visited[comp.anode as usize] {
                    bfs_queue.add(comp.anode as usize).unwrap();
                }
            }
        }
    }

    grounded
}

// =============================
// TESTING BEGINS HERE
// =============================

#[cfg(test)]
mod tests {
    use crate::*;
    use ffi::*;

    fn make_disconnected_snapshot() -> Snapshot {
        let components = vec![
            Component {
                type_: 1,
                voltage: 5.0,
                current: 0.1,
                property: 100.0,
                anode: 1,
                cathode: 0,
            },
            Component {
                type_: 2,
                voltage: 3.3,
                current: 0.05,
                property: 220.0,
                anode: 1,
                cathode: 0,
            },
            Component {
                type_: 1,
                voltage: 5.0,
                current: 0.1,
                property: 100.0,
                anode: 2,
                cathode: 3,
            },
            Component {
                type_: 2,
                voltage: 3.3,
                current: 0.05,
                property: 220.0,
                anode: 2,
                cathode: 3,
            },
        ];

        let boxed = components.into_boxed_slice();

        Snapshot {
            component_count: boxed.len() as u32,
            node_count: 4,
            components: Box::into_raw(boxed) as *mut Component,
        }
    }

    fn make_basic_snapshot() -> Snapshot {
        let components = vec![
            Component {
                type_: 1,
                voltage: 5.0,
                current: 0.1,
                property: 100.0,
                anode: 1,
                cathode: 0,
            },
            Component {
                type_: 2,
                voltage: 3.3,
                current: 0.05,
                property: 220.0,
                anode: 1,
                cathode: 0,
            },
        ];

        let boxed = components.into_boxed_slice();

        Snapshot {
            component_count: boxed.len() as u32,
            node_count: 2,
            components: Box::into_raw(boxed) as *mut Component,
        }
    }

    #[test]
    fn circuitdata_from_snapshot_works() {
        let test_snapshot = make_basic_snapshot();
        let circuit_data = circuitdata_from_snapshot(test_snapshot);
        let _expected_circuit_data = CircuitData {
            components: vec![
                Component {
                    type_: 1,
                    voltage: 5.0,
                    current: 0.1,
                    property: 100.0,
                    anode: 1,
                    cathode: 0,
                },
                Component {
                    type_: 2,
                    voltage: 3.3,
                    current: 0.05,
                    property: 220.0,
                    anode: 1,
                    cathode: 0,
                },
            ],
            node_count: 2,
        };
        assert_eq!(circuit_data, _expected_circuit_data);
    }

    #[test]
    fn grounding_works() {
        let test_snapshot = make_disconnected_snapshot();
        let circuit_data = circuitdata_from_snapshot(test_snapshot);
        let grounded_mask = ground_nodes(circuit_data);
        assert_eq!(grounded_mask, vec![true, false, true, false]);
    }

    #[test]
    fn main_solve_mna_works() {
        let test_snapshot = make_disconnected_snapshot();
        let solutions = solve_mna(test_snapshot);
        unsafe {
            assert_eq!(*solutions.voltages, 1.0);
            assert_eq!(solutions.solution_count, 2);
        }
    }
}
