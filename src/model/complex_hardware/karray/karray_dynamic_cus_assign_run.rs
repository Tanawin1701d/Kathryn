use crate::model::complex_hardware::karray::karray_dynamic_cus_assign::WriteDim;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

// ===== The cus_dynamic_assign driver =========================================
//
// The write analogue of `karray_dynamic_reduce_get_run.rs`. Where reduce FOLDS many elements
// into one winner, this FANS OUT over the spread dimensions and lets the user's
// callback decide each element's write-enable. There is no mux tree: each element
// is an independent guarded write, so the driver is a plain recursive fan-out.
//
// Everything that touches the arena or the user's callback goes through `WriteEnv`,
// so the algorithm stays PyO3-free and holds NO arena borrow. That is what makes the
// re-entrancy safe: `enable` invokes the user's Python callback (which re-enters the
// arena to build its write-enable expression), so the connector's `WriteEnv` impl
// takes a scoped arena borrow inside each op and calls `enable` with no borrow held.

/// The operations `write_run` performs, factored out so the algorithm stays
/// PyO3-free. The connector implements this over the Python arena: `commit` /
/// `finalize` each take a SCOPED borrow, while `enable` invokes the user's Python
/// callback with NO borrow held — so the callback may re-enter the arena to build
/// its write-enable.
pub trait WriteEnv {
    type Err;

    /// Ask the user's callback for element `coord`'s 1-bit write-enable.
    fn callback_user_enable(&mut self, coord: &[usize]) -> Result<HcpIdent, Self::Err>;

    /// Emit the guarded writes for element `coord` under write-enable `we`.
    fn gen_user_asm_meta(&mut self, coord: &[usize], we: HcpIdent) -> Result<(), Self::Err>;

    /// Join every gathered write into one basic node and attach it to the scope.
    fn gen_and_attach_asm_node(&mut self) -> Result<(), Self::Err>;
}

/// Drive the cus_dynamic_assign fan-out. `dim_sels` marks pin/spread per dimension;
/// a `Pin` fixes its index and descends, a `Spread` iterates its extent. At each
/// fully-pinned coordinate the user's callback supplies the write-enable, then the
/// element is committed. One joined node is attached at the end.
pub fn write_run<E: WriteEnv>(env: &mut E, dim_sels: &[WriteDim], shape: &[usize]) -> Result<(), E::Err> {
    let mut coord = Vec::with_capacity(dim_sels.len());
    write_dim(env, dim_sels, shape, 0, &mut coord)?;
    env.gen_and_attach_asm_node()
}

// Resolve dimension `dim_idx`: a `Pin` fixes its index and recurses; a `Spread`
// fans out over its extent. At the leaf, ask for the write-enable then commit.
fn write_dim<E: WriteEnv>(
    env     : &mut E,
    dim_sels: &[WriteDim],
    shape   : &[usize],
    dim_idx : usize,
    coord   : &mut Vec<usize>,
) -> Result<(), E::Err> {
    if dim_idx == dim_sels.len() {
        let we = env.callback_user_enable(coord)?;
        return env.gen_user_asm_meta(coord, we);
    }
    match dim_sels[dim_idx] {
        WriteDim::Pin(i) => {
            coord.push(i);
            write_dim(env, dim_sels, shape, dim_idx + 1, coord)?;
            coord.pop();
        }
        WriteDim::Spread => {
            for i in 0..shape[dim_idx] {
                coord.push(i);
                write_dim(env, dim_sels, shape, dim_idx + 1, coord)?;
                coord.pop();
            }
        }
    }
    Ok(())
}
