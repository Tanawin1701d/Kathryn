// Thin native entry point. All modules live in the library crate (`lib.rs`);
// the Python extension is built from that same library via maturin.

fn main() {
    println!("Hello, world!");
}
