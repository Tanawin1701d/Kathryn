/// Represents a slice with start (inclusive) and stop (exclusive) indices.
/// Range: [start, stop)
/// Default values {-1, -1} refer to entire section
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Slice {
    /// start index inclusive
    pub start: i32,
    /// stop index exclusive
    pub stop: i32,
}

impl Default for Slice {
    fn default() -> Self {
        Self {
            start: -1,
            stop: -1,
        }
    }
}

impl Slice {
    /// Create a new Slice with specified start and stop indices
    pub fn new(start: i32, stop: i32) -> Self {
        Self { start, stop }
    }

    /// Check if the slice is valid (start >= 0 and start < stop)
    pub fn check_valid_slice(&self) -> bool {
        (self.start >= 0) && (self.start < self.stop)
    }

    /// Get the size of the slice
    pub fn get_size(&self) -> i32 {
        self.stop - self.start
    }

    /// Check if a relative bit position is within the slice range
    pub fn is_bit_in_range_rel(&self, bit: i32) -> bool {
        (self.start + bit) < self.stop
    }

    pub fn is_size_in_range_rel(&self, sz: i32) -> bool {(self.start + sz) <= self.stop}

    /// Check if this slice contains another slice
    pub fn is_contain(&self, rhs: &Slice) -> bool {
        (self.start <= rhs.start) && (self.stop >= rhs.stop)
    }

    /// Check if this slice intersects with another slice
    pub fn is_intersec(&self, rhs: &Slice) -> bool {
        ((self.start >= rhs.start) && (self.start < rhs.stop)) ||
            ((rhs.start >= self.start) && (rhs.start < self.stop))
    }

    /// Get a sub-slice matching the size of another slice
    /// b is used to retrieve size only
    /// v-----0
    /// |---------------- A ----------------|
    /// |--- SIZE_OF(B) ---|
    /// |-------- C -------|  <----- result
    pub fn get_match_size_sub_slice_from_0(&self, b: &Slice) -> Slice {
        assert!(b.check_valid_slice());
        assert!(self.check_valid_slice());
        self.get_sub_slice_with_shrink_msb(&Slice::new(0, b.get_size()))
    }

    /// Get a sub-slice matching the size of another slice
    /// b is used to retrieve size only
    /// v----- start
    /// |---------------- A ----------------|
    /// |--- SIZE_OF(B) ---|
    /// |-------- C -------|  <----- result

    pub fn get_match_size_sub_slice(&self, b: &Slice) -> Slice {
        assert!(self.is_size_in_range_rel(b.get_size()));
        Slice::new(self.start, std::cmp::min(self.stop, self.start + b.get_size()))
    }

    /// Get a sub-slice with MSB shrinking
    /// The start bit must be in range but stop doesn't have to be
    /// b is relative value
    /// |------- A -------|
    ///       |------- B -------|
    ///       |---- C ----|              <----- result

    pub fn get_sub_slice_with_shrink_msb(&self, b_rel: &Slice) -> Slice {
        assert!(self.is_bit_in_range_rel(b_rel.start));
        assert!(b_rel.check_valid_slice());
        Slice::new(
            self.start + b_rel.start,
            std::cmp::min(self.stop, self.start + b_rel.stop),
        )
    }


    /// Get a sub-slice using a relative indexer
    /// indexer is relative value
    pub fn get_sub_slice(&self, indexer: &Slice) -> Slice {
        assert!(indexer.check_valid_slice());
        assert!((self.start + indexer.stop) <= self.stop);
        Slice::new(self.start + indexer.start, self.start + indexer.stop)
    }

    /// Get a sub-slice from a starting index to the end
    pub fn get_sub_slice_from(&self, start_idx: i32) -> Slice {
        assert!(start_idx < self.stop);
        Slice::new(self.start + start_idx, self.stop)
    }
}
