pub mod ffi;
use ffi::*;
use std::slice;
extern crate nalgebra;
extern crate queues;
use nalgebra::DMatrix;
use queues::*;

use crate::ffi::Component;
#[derive(Debug, PartialEq)]
pub struct CircuitData {
    pub components: Vec<Component>,
    pub node_count: usize,
    pub ungrounded: Vec<usize>,
    pub a_matrix_duplicates: DMatrix<f64>,
    pub a_matrix: DMatrix<f64>,
    pub row_copies: Vec<usize>,
}

impl CircuitData {
    pub fn from_snapshot(snap: Snapshot) -> CircuitData {
        unsafe {
            let comps = slice::from_raw_parts(snap.components, snap.component_count as usize); // Turns a pointer and size into a full list

            let mut circuit = CircuitData {
                components: comps.to_vec(),
                node_count: snap.node_count as usize,
                ungrounded: Vec::new(),
                a_matrix_duplicates: DMatrix::zeros(0, 0),
                a_matrix: DMatrix::zeros(0, 0),
                row_copies: Vec::new(),
            };

            circuit.ground_nodes();
            circuit.populate_a_matrix();

            // (circuit.a_matrix, circuit.row_copies) = circuit.reduce_and_compute_row_copies();

            circuit
        }
    }

    fn ground_nodes(&mut self) {
        let mut visited = vec![false; self.node_count];
        let mut grounded = vec![false; self.node_count];

        // Repeatedly start BFS from an unvisited node
        while !visited.iter().all(|&x| x == true) {
            let mut bfs_queue = queue![visited.iter().position(|&x| x == false).unwrap()];
            grounded[bfs_queue.peek().unwrap()] = true;
            while let Ok(current_node) = bfs_queue.remove() {
                visited[current_node] = true;

                // Go through every component where the anode or cathode is *current_node*
                for comp in self
                    .components
                    .iter()
                    .filter(|x| x.anode == current_node as u32)
                {
                    if !visited[comp.cathode as usize] {
                        bfs_queue.add(comp.cathode as usize).unwrap();
                    }
                }
                for comp in self
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

        // Apply grounded mask to enumerated nodes
        // to get a list of ungrounded nodes
        self.ungrounded = (0..self.node_count)
            .zip(&grounded)
            .filter(|(_, &m)| !&m)
            .map(|(x, _)| x)
            .collect();
    }

    fn populate_a_matrix(&mut self) {
        // Compute G matrix
        let g_matrix = self.compute_g_matrix();

        // Compute B and C matrices
        let (b_matrix, c_matrix) = self.compute_bc_matrix();

        // D matrix is all zeros
        let d_matrix = DMatrix::<f64>::zeros(b_matrix.ncols(), b_matrix.ncols());

        // Dimensions of the final matrix
        let rows = g_matrix.nrows() + c_matrix.nrows();
        let cols = g_matrix.ncols() + b_matrix.ncols();

        let mut a_matrix = DMatrix::<f64>::zeros(rows, cols);

        // Top-left: G
        a_matrix
            .view_mut((0, 0), (g_matrix.nrows(), g_matrix.ncols()))
            .copy_from(&g_matrix);

        // Top-right: B
        a_matrix
            .view_mut((0, g_matrix.ncols()), (b_matrix.nrows(), b_matrix.ncols()))
            .copy_from(&b_matrix);

        // Bottom-left: C
        a_matrix
            .view_mut((g_matrix.nrows(), 0), (c_matrix.nrows(), c_matrix.ncols()))
            .copy_from(&c_matrix);

        // Bottom-right: D
        a_matrix
            .view_mut(
                (g_matrix.nrows(), g_matrix.ncols()),
                (d_matrix.nrows(), d_matrix.ncols()),
            )
            .copy_from(&d_matrix);

        self.a_matrix = a_matrix;
    }

    fn reduce_and_compute_row_copies(&self) -> (DMatrix<f64>, Vec<usize>) {
        let rows = self.a_matrix.nrows();
        let cols = self.a_matrix.ncols();

        // usize::MAX means that row is not a duplicate
        let mut duplicates = vec![usize::MAX; rows];

        // Detect duplicate rows
        for i in 0..rows {
            if duplicates[i] != usize::MAX {
                continue;
            }

            for j in (i + 1)..rows {
                if duplicates[j] != usize::MAX {
                    continue;
                }

                let identical = (0..cols).all(|c| self.a_matrix[(i, c)] == self.a_matrix[(j, c)]);

                if identical {
                    duplicates[j] = i;
                }
            }
        }

        // Collect unique rows
        let unique_rows: Vec<usize> = (0..rows).filter(|&r| duplicates[r] == usize::MAX).collect();

        let mut reduced = DMatrix::zeros(unique_rows.len(), cols);

        for (new_row, &old_row) in unique_rows.iter().enumerate() {
            for c in 0..cols {
                reduced[(new_row, c)] = self.a_matrix[(old_row, c)];
            }
        }

        (reduced, duplicates)
    }

    fn compute_g_matrix(&self) -> DMatrix<f64> {
        let mut matrix: DMatrix<f64> = DMatrix::zeros(self.ungrounded.len(), self.ungrounded.len());
        for i in 0..self.ungrounded.len() {
            let ui = self.ungrounded[i];
            // Get conductance of all surrounding nodes
            for comp in self
                .components
                .iter()
                .filter(|x| x.type_ == 1 && (x.anode == ui as u32 || x.cathode == ui as u32))
            {
                matrix[(i, i)] += comp.property.recip();
            }
            for j in (i + 1)..self.ungrounded.len() {
                let uj = self.ungrounded[j];
                // Get conductance of all components between the two nodes
                for comp in self.components.iter().filter(|x| x.type_ == 1) {
                    if comp.anode == ui as u32 && comp.cathode == uj as u32
                        || comp.cathode == ui as u32 && comp.anode == uj as u32
                    {
                        matrix[(i, j)] -= comp.property.recip();
                        matrix[(j, i)] -= comp.property.recip();
                    }
                }
            }
        }
        matrix
    }

    fn compute_bc_matrix(&self) -> (DMatrix<f64>, DMatrix<f64>) {
        let cells: Vec<&Component> = self.components.iter().filter(|x| x.type_ == 2).collect();
        let mut bmatrix: DMatrix<f64> = DMatrix::zeros(self.ungrounded.len(), cells.len());
        for (i, cell) in cells.iter().enumerate() {
            for (j, node) in self.ungrounded.iter().enumerate() {
                if cell.anode == *node as u32 {
                    bmatrix[(j, i)] = 1.0;
                } else if cell.cathode == *node as u32 {
                    bmatrix[(j, i)] = -1.0;
                }
            }
        }

        let cmatrix = bmatrix.transpose();
        (bmatrix, cmatrix)
    }
}

// =============================
// TESTING BEGINS HERE
// =============================

#[cfg(test)]
mod tests {
    use super::*;
    use ffi::*;
    use nalgebra::DMatrix;

    /// Two isolated subgraphs: nodes {0,1} and nodes {2,3}.
    /// Each subgraph has one resistor (type 1) and one voltage source (type 2).
    fn make_disconnected_snapshot() -> Snapshot {
        let components = vec![
            Component {
                type_: 1,
                voltage: 0.0,
                current: 0.0,
                property: 100.0,
                anode: 1,
                cathode: 0,
            },
            Component {
                type_: 2,
                voltage: 0.0,
                current: 0.0,
                property: 0.0,
                anode: 1,
                cathode: 0,
            },
            Component {
                type_: 1,
                voltage: 0.0,
                current: 0.0,
                property: 100.0,
                anode: 2,
                cathode: 3,
            },
            Component {
                type_: 2,
                voltage: 0.0,
                current: 0.0,
                property: 0.0,
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

    /// Minimal single-loop circuit: one resistor and one voltage source
    /// between nodes 0 and 1.
    fn make_basic_snapshot() -> Snapshot {
        let components = vec![
            Component {
                type_: 1,
                voltage: 0.0,
                current: 0.0,
                property: 100.0,
                anode: 1,
                cathode: 0,
            },
            Component {
                type_: 2,
                voltage: 0.0,
                current: 0.0,
                property: 0.0,
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

    // ── Stage 1: snapshot → components / node_count ───────────────────────

    #[test]
    fn snapshot_converts_components() {
        let circuit = CircuitData::from_snapshot(make_basic_snapshot());
        assert_eq!(circuit.components.len(), 2);
        assert_eq!(circuit.components[0].type_, 1);
        assert_eq!(circuit.components[0].property, 100.0);
        assert_eq!(circuit.components[1].type_, 2);
    }

    #[test]
    fn snapshot_converts_node_count() {
        let circuit = CircuitData::from_snapshot(make_basic_snapshot());
        assert_eq!(circuit.node_count, 2);
    }

    // ── Stage 2: ground_nodes → ungrounded ───────────────────────────────

    /// The first node visited in each subgraph is grounded (index 0 and 2),
    /// so the ungrounded list should be [1, 3].
    #[test]
    fn grounding_produces_correct_ungrounded_nodes() {
        let circuit = CircuitData::from_snapshot(make_disconnected_snapshot());
        assert_eq!(circuit.ungrounded, vec![1, 3]);
    }

    /// A single connected graph should ground exactly one node.
    #[test]
    fn grounding_grounds_one_node_per_subgraph() {
        let circuit = CircuitData::from_snapshot(make_basic_snapshot());
        assert_eq!(circuit.ungrounded.len(), 1);
        // node 0 is grounded, so only node 1 survives
        assert_eq!(circuit.ungrounded, vec![1]);
    }

    // ── Stage 3: populate_a_matrix → a_matrix ────────────────────────────
    //
    // A is laid out as:
    //   [ G  B ]
    //   [ C  D ]
    //
    // With 2 ungrounded nodes and 2 voltage sources the full matrix is 4×4.
    // We extract quadrants by slicing rather than comparing the whole matrix,
    // so a bug in one quadrant doesn't obscure which part failed.

    fn make_a_matrix() -> DMatrix<f64> {
        CircuitData::from_snapshot(make_disconnected_snapshot()).a_matrix
    }

    /// G is n×n (n = ungrounded node count). Each diagonal entry is the sum
    /// of conductances at that node; off-diagonals are negated conductances
    /// between pairs.  With R=100Ω the conductance is 0.01 S.
    #[test]
    fn a_matrix_g_quadrant_is_correct() {
        let a = make_a_matrix();
        let g = a.view((0, 0), (2, 2));
        let mut expected = DMatrix::zeros(2, 2);
        expected[(0, 0)] = 0.01;
        expected[(1, 1)] = 0.01;
        assert_eq!(g, expected.view((0, 0), (2, 2)));
    }

    /// B is n×m (m = voltage source count). Entry (j, i) = +1 when the
    /// source's anode is at ungrounded node j; −1 when it's the cathode.
    #[test]
    fn a_matrix_b_quadrant_is_correct() {
        let a = make_a_matrix();
        let b = a.view((0, 2), (2, 2));
        let mut expected = DMatrix::zeros(2, 2);
        expected[(0, 0)] = 1.0;
        expected[(1, 1)] = -1.0;
        assert_eq!(b, expected.view((0, 0), (2, 2)));
    }

    /// C is the transpose of B.
    #[test]
    fn a_matrix_c_quadrant_is_correct() {
        let a = make_a_matrix();
        let c = a.view((2, 0), (2, 2));
        let mut expected = DMatrix::zeros(2, 2);
        expected[(0, 0)] = 1.0;
        expected[(1, 1)] = -1.0;
        assert_eq!(c, expected.view((0, 0), (2, 2)));
    }

    /// D is all zeros (no coupling between voltage sources).
    #[test]
    fn a_matrix_d_quadrant_is_zeros() {
        let a = make_a_matrix();
        let d = a.view((2, 2), (2, 2));
        assert_eq!(d, DMatrix::<f64>::zeros(2, 2).view((0, 0), (2, 2)));
    }

    /// Overall matrix dimensions: (n + m) × (n + m).
    #[test]
    fn a_matrix_has_correct_dimensions() {
        let a = make_a_matrix();
        assert_eq!(a.nrows(), 4);
        assert_eq!(a.ncols(), 4);
    }

    /// Simple voltage divider: 9V source, two 100ohm resistors in series.
    /// Nodes: 0 (ground), 1 (midpoint), 2 (top)
    /// Expected: V(1) = 4.5V, V(2) = 9.0V
    fn make_voltage_divider_snapshot() -> Snapshot {
        let components = vec![
            Component {
                type_: 2,
                voltage: 0.0,
                current: 0.0,
                property: 9.0,
                anode: 2,
                cathode: 0,
            },
            Component {
                type_: 1,
                voltage: 0.0,
                current: 0.0,
                property: 100.0,
                anode: 2,
                cathode: 1,
            },
            Component {
                type_: 1,
                voltage: 0.0,
                current: 0.0,
                property: 100.0,
                anode: 1,
                cathode: 0,
            },
        ];
        let boxed = components.into_boxed_slice();
        Snapshot {
            component_count: boxed.len() as u32,
            node_count: 3,
            components: Box::into_raw(boxed) as *mut Component,
        }
    }

    /// Single loop: 5V source, one 100Ω resistor.
    /// Nodes: 0 (ground), 1 (top)
    /// Expected: V(1) = 5.0V, branch current = 0.05A
    fn make_single_loop_snapshot() -> Snapshot {
        let components = vec![
            Component {
                type_: 2,
                voltage: 0.0,
                current: 0.0,
                property: 5.0,
                anode: 1,
                cathode: 0,
            },
            Component {
                type_: 1,
                voltage: 0.0,
                current: 0.0,
                property: 100.0,
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

    /// Current source: 1A source pushing current from node 0 to node 1,
    /// with a 10 ohm resistor across the nodes.
    /// Nodes: 0 (ground), 1 (top)
    /// Expected: V(1) = 10.0V  (V = I * R)
    fn make_current_source_snapshot() -> Snapshot {
        let components = vec![
            Component {
                type_: 3,
                voltage: 0.0,
                current: 0.0,
                property: 1.0,
                anode: 1,
                cathode: 0,
            },
            Component {
                type_: 1,
                voltage: 0.0,
                current: 0.0,
                property: 10.0,
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
    fn single_loop_voltage_is_correct() {
        let sol = Solution::from_snapshot(make_single_loop_snapshot());
        unsafe {
            let voltages = std::slice::from_raw_parts(sol.voltages, sol.solution_count as usize);
            assert_eq!(sol.solution_count, 1);
            assert!((voltages[0] - 5.0).abs() < 1e-9);
        }
    }

    #[test]
    fn voltage_divider_midpoint_is_correct() {
        let sol = Solution::from_snapshot(make_voltage_divider_snapshot());
        unsafe {
            let voltages = std::slice::from_raw_parts(sol.voltages, sol.solution_count as usize);
            assert_eq!(sol.solution_count, 2);
            // Node ordering follows ungrounded vec — nodes 1 and 2
            assert!((voltages[0] - 4.5).abs() < 1e-9);
            assert!((voltages[1] - 9.0).abs() < 1e-9);
        }
    }

    #[test]
    fn current_source_voltage_is_correct() {
        let sol = Solution::from_snapshot(make_current_source_snapshot());
        unsafe {
            let voltages = std::slice::from_raw_parts(sol.voltages, sol.solution_count as usize);
            assert_eq!(sol.solution_count, 1);
            assert!((voltages[0] + 10.0).abs() < 1e-9);
        }
    }
}
