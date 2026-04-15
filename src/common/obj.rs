use std::rc::Rc;
use std::cell::RefCell;

// SP = Shared Pointer
pub(crate) type SPTR<T> = Rc<RefCell<T>>;