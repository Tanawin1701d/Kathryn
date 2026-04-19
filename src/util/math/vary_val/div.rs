use std::ops::Div;
use super::VaryVal;

/// Returns (quotient, remainder) via binary long division.
pub fn divmod(dividend: &VaryVal, divisor: &VaryVal) -> (VaryVal, VaryVal) {
    dividend.assert_same_width(divisor);
    assert!(!divisor.is_zero(), "VaryVal: division by zero");

    let bw = dividend.bit_width();
    let mut quotient  = VaryVal::new(bw);
    let mut remainder = VaryVal::new(bw);

    for i in (0..bw).rev() {
        // remainder = (remainder << 1) | dividend.bit(i)
        shl1_in_place(&mut remainder);
        remainder.set_bit(0, dividend.get_bit(i));

        if remainder.partial_cmp(divisor) != Some(std::cmp::Ordering::Less) {
            remainder = &remainder - divisor;
            quotient.set_bit(i, true);
        }
    }
    (quotient, remainder)
}

fn shl1_in_place(v: &mut VaryVal) {
    let mut carry = 0u64;
    for limb in v.limbs.iter_mut() {
        let next_carry = *limb >> 63;
        *limb = (*limb << 1) | carry;
        carry = next_carry;
    }
    v.apply_mask();
}

impl Div for VaryVal {
    type Output = VaryVal;
    fn div(self, rhs: VaryVal) -> VaryVal { divmod(&self, &rhs).0 }
}

impl Div<&VaryVal> for VaryVal {
    type Output = VaryVal;
    fn div(self, rhs: &VaryVal) -> VaryVal { divmod(&self, rhs).0 }
}

impl Div<VaryVal> for &VaryVal {
    type Output = VaryVal;
    fn div(self, rhs: VaryVal) -> VaryVal { divmod(self, &rhs).0 }
}

impl Div<&VaryVal> for &VaryVal {
    type Output = VaryVal;
    fn div(self, rhs: &VaryVal) -> VaryVal { divmod(self, rhs).0 }
}
