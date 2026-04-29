use crate::snapshot::CircuitData;

pub fn build_system(data: &CircuitData) -> (Vec<Vec<f64>>, Vec<f64>) {
    let n = data.node_count;

    let mut g = vec![vec![0.0; n]; n];
    let mut b = vec![0.0; n];

    for c in &data.components {
        let i = c.anode as usize;
        let j = c.cathode as usize;

        match c.type_ {
            // Resistor
            1 => {
                let gval = 1.0 / c.property;

                g[i][i] += gval;
                g[j][j] += gval;
                g[i][j] -= gval;
                g[j][i] -= gval;
            }

            // Voltage source (simplified placeholder)
            0 => {
                b[i] += c.voltage;
                b[j] -= c.voltage;
            }

            _ => {}
        }
    }

    (g, b)
}