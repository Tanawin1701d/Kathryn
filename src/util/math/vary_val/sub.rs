use std::ops::Sub;
use super::VaryVal;

/// Wrapping subtraction (two's complement within bit_width).
fn sub_inner(mut lhs: VaryVal, rhs: &VaryVal) -> VaryVal {
    lhs.assert_same_width(rhs);
    let mut borrow = false;
    for (a, b) in lhs.limbs.iter_mut().zip(rhs.limbs.iter()) {
        let (diff1, b1) = a.overflowing_sub(*b);
        let (diff2, b2) = diff1.overflowing_sub(borrow as u64);
        *a     = diff2;
        borrow = b1 || b2;
    }
    lhs.apply_mask();
    lhs
}

impl Sub for VaryVal {
    type Output = VaryVal;
    fn sub(self, rhs: VaryVal) -> VaryVal { sub_inner(self, &rhs) }
}

impl Sub<&VaryVal> for VaryVal {
    type Output = VaryVal;
    fn sub(self, rhs: &VaryVal) -> VaryVal { sub_inner(self, rhs) }
}

impl Sub<VaryVal> for &VaryVal {
    type Output = VaryVal;
    fn sub(self, rhs: VaryVal) -> VaryVal { sub_inner(self.clone(), &rhs) }
}

impl Sub<&VaryVal> for &VaryVal {
    type Output = VaryVal;
    fn sub(self, rhs: &VaryVal) -> VaryVal { sub_inner(self.clone(), rhs) }
}
