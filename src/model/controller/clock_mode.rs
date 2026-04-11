#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ClockMode {
    PosEdge,
    NegEdge,
    ClkFree,
    ClkUnused,
    Amt,
}
