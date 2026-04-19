use std::ops::Mul;
use super::VaryVal;

/// Grade-school O(n²) multiplication, result truncated to bit_width.
fn mul_inner(lhs: &VaryVal, rhs: &VaryVal) -> VaryVal {
    lhs.assert_same_width(rhs);
    let n = lhs.limbs().len();
    let mut result = VaryVal::new(lhs.bit_width());

    for (i, &a) in lhs.limbs().iter().enumerate() {
        let mut carry: u128 = 0;
        for (j, &b) in rhs.limbs().iter().enumerate() {
            let pos = i + j;
            if pos >= n { break; }
            let prod = (a as u128) * (b as u128) + (result.limbs[pos] as u128) + carry;
            result.limbs[pos] = prod as u64;
            carry = prod >> 64;
        }
    }
    result.apply_mask();
    result
}

impl Mul for VaryVal {
    type Output = VaryVal;
    fn mul(self, rhs: VaryVal) -> VaryVal { mul_inner(&self, &rhs) }
}

impl Mul<&VaryVal> for VaryVal {
    type Output = VaryVal;
    fn mul(self, rhs: &VaryVal) -> VaryVal { mul_inner(&self, rhs) }
}

impl Mul<VaryVal> for &VaryVal {
    type Output = VaryVal;
    fn mul(self, rhs: VaryVal) -> VaryVal { mul_inner(self, &rhs) }
}

impl Mul<&VaryVal> for &VaryVal {
    type Output = VaryVal;
    fn mul(self, rhs: &VaryVal) -> VaryVal { mul_inner(self, rhs) }
}
