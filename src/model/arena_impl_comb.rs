// Combinational combinator builders — a select (`gen_mux`), a rotation
// (`gen_rotate_left`), an OR reduce (`gen_any_of`) and a population count /
// adder tree (`gen_sum_cnt`). None adds a node type: each is a small assembly
// of the wire / expression / flow-block primitives this arena already has,
// kept HERE so every frontend (the Python DSL, a future one) builds the same
// hardware from the same rules instead of re-implementing them per language.
//
// Decisions (moved verbatim from the former Python-only implementation):
// - `gen_mux` is a WIRE plus `zif`/`zelse`, not a mask expression. Kathryn has
//   no ternary LogicOp, and `ExtendBit` fills with 1'b0 (zero-extension, never
//   replication) so a bit-replicated mask cannot be built. The zif chain is
//   the tested priority-mux path and emits a plain `if/else`. COST: it
//   declares hardware, so an open flow scope is required.
// - `gen_rotate_left` is shifts and an OR, not slice-and-concatenate: an
//   expression follows its LEFT operand's width, so `x << k` and
//   `x >> (w-k)` are both x-wide and the bits shifted off one end are exactly
//   the bits the other end supplies.
// - `gen_any_of` / `gen_sum_cnt` build BALANCED trees (log2(n) depth, not n);
//   the odd element of a level rides to the next level unchanged. `gen_any_of`
//   over nothing is FALSE (a defined value with a defined width); a sum over
//   nothing has neither and errors.
// - `gen_sum_cnt`'s default width is exactly wide enough for the largest sum
//   the inputs can make, so it can never overflow.
//
// Frontend split: int-literal operands never reach here — a connector wraps
// them into width-matched `val`s first (the Python one via `make_const_val`,
// BigInt and all). This file only ever sees `HcpIdent` + `Slice`.

use crate::model::hw_component::arena_impl_hwc::AsmResize;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

impl ModelArena {

    // ---- shared helpers -----------------------------------------------------

    // The slice a combinator actually reads: the given one when it is a real
    // range, else the signal's full width.
    fn resolve_read_slice(&self, sig_i: HcpIdent, slice: Option<Slice>) -> Slice {
        match slice {
            Some(s) if s.check_valid_slice() => s,
            _                                => self.get_hw_slice(&sig_i),
        }
    }

    // A flow-block condition must be exactly 1 bit (slice, or whole variable).
    // Same rule (and messages) as the Python-side `check_cond_slice_match`, but
    // owned by the core so every frontend gets the guard.
    fn check_comb_cond(&self, cond_i: HcpIdent, cond_slice: Option<Slice>) -> Result<(), String> {
        let var_width  = self.get_hw_bit_sz(&cond_i);
        let cond_width = match cond_slice {
            Some(s) if s.check_valid_slice() => {
                if s.stop > var_width {
                    return Err(format!(
                        "cond slice {s:?} does not fit cond variable of width {var_width}"));
                }
                s.get_size()
            }
            _ => var_width,
        };
        if cond_width != 1 {
            return Err(format!(
                "cond must be 1-bit, got {cond_width} (slice the cond variable down to one bit)"));
        }
        Ok(())
    }

    // ---- select -------------------------------------------------------------

    // `cond ? true : false` into a fresh combinational wire. Declares hardware,
    // so an open flow scope is required. `width` defaults to the true arm's
    // read width. Returns the wire plus the two arms' resize reports so a
    // frontend can surface implicit width changes.
    pub fn gen_mux(
        &mut self,
        name        : &str,
        cond_i      : HcpIdent,
        cond_slice  : Option<Slice>,
        true_i      : HcpIdent,
        true_slice  : Option<Slice>,
        false_i     : HcpIdent,
        false_slice : Option<Slice>,
        width       : Option<i32>,
    ) -> Result<(HcpIdent, [AsmResize; 2]), String> {
        self.check_comb_cond(cond_i, cond_slice)?;
        let true_slice  = self.resolve_read_slice(true_i,  true_slice);
        let false_slice = self.resolve_read_slice(false_i, false_slice);
        let width       = width.unwrap_or_else(|| true_slice.get_size());
        if width < 1 {
            return Err(format!("mux needs a width >= 1, got {width}"));
        }

        // Declared BEFORE the branches open, so the wire belongs to the
        // enclosing scope and only its two assignments live inside the arms.
        let out_i = self.make_wire(true, name, width);
        let full  = Slice::new(0, width);

        let zif_i = self.make_flow_block_zif(&format!("{name}_zif"), cond_i, cond_slice);
        self.initialize_flow_block(zif_i);
        let true_resize = self.gen_basic_assign(out_i, true_i, Some(full), true_slice);
        self.finalize_flow_block(zif_i, false);
        self.check_flow_block_prefinalize(zif_i)?;

        let zelse_i = self.make_flow_block_zelse(&format!("{name}_zelse"));
        self.initialize_flow_block(zelse_i);
        let false_resize = self.gen_basic_assign(out_i, false_i, Some(full), false_slice);
        self.finalize_flow_block(zelse_i, false);
        self.check_flow_block_prefinalize(zelse_i)?;

        Ok((out_i, [true_resize, false_resize]))
    }

    // ---- rotation -----------------------------------------------------------

    // `sig` rotated left by `amount` (an elaboration-time constant, taken mod
    // `width`), as a pure expression. `width` defaults to the signal's read
    // width and may only rotate WITHIN it. `Ok(None)` = a full turn — the
    // identity; the caller keeps using the input signal unchanged.
    pub fn gen_rotate_left(
        &mut self,
        name      : &str,
        sig_i     : HcpIdent,
        sig_slice : Option<Slice>,
        amount    : i64,
        width     : Option<i32>,
    ) -> Result<Option<HcpIdent>, String> {
        let sig_slice = self.resolve_read_slice(sig_i, sig_slice);
        let actual    = sig_slice.get_size();
        let width     = match width {
            None                 => actual,
            Some(w) if w < 1     => return Err(format!("rotate_left needs a width >= 1, got {w}")),
            Some(w) if w > actual => return Err(format!(
                "rotate_left width {w} exceeds the signal's {actual} bits — \
                 a rotate is only a rotate over bits that are there")),
            Some(w)              => w,
        };

        let amount = amount.rem_euclid(width as i64) as i32;
        if amount == 0 {
            return Ok(None);
        }

        // Both halves come out `width` wide (an expression follows its LEFT
        // operand), so what falls off the top of one is exactly what the other
        // puts back at bit 0. The shift-amount constants are sized to the
        // signal's FULL width, matching how an int operand is wrapped elsewhere.
        let const_w   = self.get_hw_bit_sz(&sig_i);
        let shl_amt_i = self.make_val(true, &format!("{name}_c"), const_w, amount as u64);
        let shr_amt_i = self.make_val(true, &format!("{name}_c"), const_w, (width - amount) as u64);
        let shl_i     = self.make_expression(true, &format!("{name}_shl"), LogicOp::BitwiseShl, sig_i, shl_amt_i, Some(sig_slice), None);
        let shr_i     = self.make_expression(true, &format!("{name}_shr"), LogicOp::BitwiseShr, sig_i, shr_amt_i, Some(sig_slice), None);
        Ok(Some(self.make_expression(true, name, LogicOp::BitwiseOr, shl_i, shr_i, None, None)))
    }

    // ---- reduction ----------------------------------------------------------

    // True when any of `terms` is — a balanced `||` tree over 1-bit signals.
    // Pure expression. No terms yields a constant-0 val (an empty disjunction
    // has a defined value and width); ONE term yields `None` — the identity,
    // the caller keeps using that term unchanged (its slice view intact).
    pub fn gen_any_of(
        &mut self,
        name  : &str,
        terms : Vec<(HcpIdent, Slice)>,
    ) -> Option<HcpIdent> {
        if terms.is_empty() {
            return Some(self.make_val(true, name, 1, 0));
        }
        if terms.len() == 1 {
            return None;
        }
        let level = terms.into_iter().map(|(term_i, s)| (term_i, Some(s))).collect();
        Some(self.fold_balanced(name, level, LogicOp::LogicalOr))
    }

    // ---- population count / adder tree --------------------------------------

    // How many of `bits` are set — a balanced adder tree. Pure expression.
    // Every term is first zero-extended to the result width, so no
    // intermediate add truncates; the default `width` cannot overflow.
    pub fn gen_sum_cnt(
        &mut self,
        name  : &str,
        bits  : Vec<(HcpIdent, Slice)>,
        width : Option<i32>,
    ) -> Result<HcpIdent, String> {
        if bits.is_empty() {
            return Err("sum_cnt of no signals has no width — pass at least one".to_string());
        }
        // Widest a sum of these can get: len(bits) copies of the largest input.
        let max_in = bits.iter().map(|(_, s)| s.get_size()).max().unwrap().max(1);
        let out_w  = width.unwrap_or_else(|| natural_sum_width(bits.len(), max_in));

        let level = bits.into_iter()
            .map(|(bit_i, s)| {
                let ext_i = self.make_expression_constant(
                    true, &format!("{name}_ext"), LogicOp::ExtendBit, bit_i, out_w, Some(s));
                (ext_i, None)
            })
            .collect();
        Ok(self.fold_balanced(name, level, LogicOp::ArithPlus))
    }

    // ---- balanced tree fold -------------------------------------------------

    // Pair up a level, combine each pair with `op`, repeat: log2(n) depth, not
    // n. The odd element of a level rides to the next level unchanged (slice
    // view intact — combined nodes read full width, hence `None`).
    fn fold_balanced(
        &mut self,
        name  : &str,
        level : Vec<(HcpIdent, Option<Slice>)>,
        op    : LogicOp,
    ) -> HcpIdent {
        let mut level = level;
        while level.len() > 1 {
            let mut paired = Vec::with_capacity(level.len() / 2 + 1);
            for pair in level.chunks(2) {
                match *pair {
                    [(a_i, a_slice), (b_i, b_slice)] => {
                        let comb_i = self.make_expression(
                            true, &format!("{name}_t"), op, a_i, b_i, a_slice, b_slice);
                        paired.push((comb_i, None));
                    }
                    [last] => paired.push(last),   // odd one out rides to the next level
                    _      => unreachable!(),
                }
            }
            level = paired;
        }
        level[0].0
    }
}

// Exact bit length of `n * (2^m - 1)` — the largest sum n inputs of m bits can
// make — without bigint. Direct u128 arithmetic while it fits; above that the
// analytic form: bitlen(n<<m - n) is L+m when (n - 2^(L-1))*2^m >= n (with
// L = bitlen(n)), else L+m-1 — and at m > 100 the product dwarfs any usize n,
// so the test collapses to "n is not a power of two".
fn natural_sum_width(n: usize, m: i32) -> i32 {
    let n = n as u128;
    let m = m as u32;
    if m <= 100 {
        let v = n * ((1u128 << m) - 1);
        (128 - v.leading_zeros()) as i32
    } else {
        let bitlen_n = 128 - n.leading_zeros();
        let is_carry = n > (1u128 << (bitlen_n - 1));   // n not a power of two
        (if is_carry { bitlen_n + m } else { bitlen_n + m - 1 }) as i32
    }
}
