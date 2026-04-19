use std::ops::Add;
use super::VaryVal;

fn add_inner(mut lhs: VaryVal, rhs: &VaryVal) -> VaryVal {
    lhs.assert_same_width(rhs);
    let mut carry: u128 = 0;
    for (a, b) in lhs.limbs.iter_mut().zip(rhs.limbs.iter()) {
        let sum = (*a as u128) + (*b as u128) + carry;
        *a   = sum as u64;
        carry = sum >> 64;
    }
    lhs.apply_mask();
    lhs
}

impl Add for VaryVal {
    type Output = VaryVal;
    fn add(self, rhs: VaryVal) -> VaryVal { add_inner(self, &rhs) }
}

impl Add<&VaryVal> for VaryVal {
    type Output = VaryVal;
    fn add(self, rhs: &VaryVal) -> VaryVal { add_inner(self, rhs) }
}

impl Add<VaryVal> for &VaryVal {
    type Output = VaryVal;
    fn add(self, rhs: VaryVal) -> VaryVal { add_inner(self.clone(), &rhs) }
}

impl Add<&VaryVal> for &VaryVal {
    type Output = VaryVal;
    fn add(self, rhs: &VaryVal) -> VaryVal { add_inner(self.clone(), rhs) }
}
