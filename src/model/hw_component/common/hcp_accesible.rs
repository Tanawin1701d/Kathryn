use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;

pub trait HcpAccessible: HcpReadable + HcpAssignable {

    fn get_bit_width(&self) -> usize;
}