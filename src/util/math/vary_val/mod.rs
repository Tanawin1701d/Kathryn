mod add;
mod sub;
mod mul;
mod div;

/// Arbitrary-width integer value stored as little-endian u64 limbs.
/// limbs[0] holds bits 0..63, limbs[1] holds bits 64..127, etc.
pub struct VaryVal {
    limbs    : Vec<u64>,
    bit_width: usize,
}

impl VaryVal {

    pub fn new(bit_width: usize) -> Self {
        let words = (bit_width + 63) / 64;
        Self { limbs: vec![0u64; words], bit_width }
    }

    pub fn from_u64(val: u64, bit_width: usize) -> Self {
        let mut v = Self::new(bit_width);
        if !v.limbs.is_empty() {
            v.limbs[0] = val;
        }
        v.apply_mask();
        v
    }

    pub fn bit_width(&self) -> usize { self.bit_width }
    pub fn limbs    (&self) -> &[u64] { &self.limbs }

    pub fn is_zero(&self) -> bool {
        self.limbs.iter().all(|&l| l == 0)
    }

    pub fn get_bit(&self, pos: usize) -> bool {
        let (limb_idx, bit_idx) = (pos / 64, pos % 64);
        self.limbs.get(limb_idx).map_or(false, |l| (l >> bit_idx) & 1 == 1)
    }

    pub fn set_bit(&mut self, pos: usize, val: bool) {
        let (limb_idx, bit_idx) = (pos / 64, pos % 64);
        if let Some(limb) = self.limbs.get_mut(limb_idx) {
            if val { *limb |=  (1u64 << bit_idx); }
            else   { *limb &= !(1u64 << bit_idx); }
        }
    }

    // Clamp the top limb to exactly bit_width bits.
    pub(super) fn apply_mask(&mut self) {
        let remainder = self.bit_width % 64;
        if remainder != 0 {
            if let Some(last) = self.limbs.last_mut() {
                *last &= (1u64 << remainder) - 1;
            }
        }
    }

    pub(super) fn assert_same_width(&self, rhs: &VaryVal) {
        assert_eq!(self.bit_width, rhs.bit_width,
            "VaryVal bit_width mismatch: {} vs {}", self.bit_width, rhs.bit_width);
    }
}

impl Clone for VaryVal {
    fn clone(&self) -> Self {
        Self { limbs: self.limbs.clone(), bit_width: self.bit_width }
    }
}

impl PartialEq for VaryVal {
    fn eq(&self, other: &Self) -> bool {
        self.bit_width == other.bit_width && self.limbs == other.limbs
    }
}

impl PartialOrd for VaryVal {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        self.assert_same_width(other);
        for (a, b) in self.limbs.iter().zip(other.limbs.iter()).rev() {
            match a.cmp(b) {
                std::cmp::Ordering::Equal => continue,
                ord => return Some(ord),
            }
        }
        Some(std::cmp::Ordering::Equal)
    }
}
