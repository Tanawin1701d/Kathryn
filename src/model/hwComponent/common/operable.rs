



#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum CLOCK_MODE {
    CM_CLK_UNUSED,
    CM_CLK_USED,
}

pub trait Operable {
    /// Check if the component is currently enabled
    fn is_enabled(&self) -> bool;

    /// Enable the component
    fn enable(&mut self);

    /// Disable the component
    fn disable(&mut self);

    /// Reset the component to its initial state
    fn reset(&mut self);

    /// Check if the component is in a valid operational state
    fn is_operational(&self) -> bool;
}
