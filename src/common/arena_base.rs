use crate::model::common::identifier::Identifiable;

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub struct ArenaHandle {
    index      : usize,
    generation : u32,
}

impl Default for ArenaHandle {
    fn default() -> Self {
        Self { index: 0, generation: u32::MAX }
    }
}

pub struct ArenaGroup<T : Identifiable> {
    nodes      : Vec<ArenaNode<T>>,
    free_slots : Vec<usize>,
}

impl<T: Identifiable> ArenaGroup<T> {
    pub fn new() -> Self {
        Self { nodes: Vec::new(), free_slots: Vec::new() }
    }

    pub fn insert(&mut self, value: T) -> ArenaHandle {
        let index = if let Some(i) = self.free_slots.pop() {
            self.nodes[i].replace(value);
            i
        } else {
            let i = self.nodes.len();
            self.nodes.push(ArenaNode::new(value));
            i
        };
        let h = ArenaHandle { index, generation: self.nodes[index].generation() };
        self.nodes[index].get_mut().set_arena_handle(h);
        h
    }

    pub fn is_valid(&self, handle: ArenaHandle) -> bool {
        self.nodes
            .get(handle.index)
            .map_or(false, |n| n.generation() == handle.generation)
    }

    pub fn get(&self, handle: ArenaHandle) -> &T {
        assert!(handle.index < self.nodes.len(), "ArenaHandle index out of range");
        let node = &self.nodes[handle.index];
        assert_eq!(node.generation(), handle.generation, "ArenaHandle generation mismatch");
        node.get()
    }

    pub fn get_mut(&mut self, handle: ArenaHandle) -> &mut T {
        assert!(handle.index < self.nodes.len(), "ArenaHandle index out of range");
        let node = &mut self.nodes[handle.index];
        assert_eq!(node.generation(), handle.generation, "ArenaHandle generation mismatch");
        node.get_mut()
    }

    pub fn len(&self) -> usize   { self.nodes.len() - self.free_slots.len() }
    pub fn is_empty(&self) -> bool { self.len() == 0 }

    pub fn get_last(&self) -> Option<&T> {
        if self.nodes.is_empty() {
            return None;
        }
        let index = self.nodes.len() - 1;
        Some(self.nodes[index].get())
    }

    pub fn get_last_mut(&mut self) -> Option<&mut T> {
        if self.nodes.is_empty() {
            return None;
        }
        let index = self.nodes.len() - 1;
        Some(self.nodes[index].get_mut())
    }

    pub fn free(&mut self, handle: ArenaHandle) {
        self.nodes[handle.index].free();
        self.free_slots.push(handle.index);
    }

    pub fn take(&mut self, handle: ArenaHandle) -> T where T: Default {
        assert!(handle.index < self.nodes.len(), "ArenaHandle index out of range");
        let node = &mut self.nodes[handle.index];
        assert_eq!(node.generation(), handle.generation, "ArenaHandle generation mismatch");
        node.temp_take()
    }

    pub fn replace_back(&mut self, handle: ArenaHandle, value: T) {
        assert!(handle.index < self.nodes.len(), "ArenaHandle index out of range");
        let node = &mut self.nodes[handle.index];
        assert_eq!(node.generation(), handle.generation, "ArenaHandle generation mismatch");
        node.temp_replace_back(value);
    }
}

pub struct ArenaNode<T> {
    value         : T,
    generation    : u32,
    is_temp_taken : bool,
}

impl<T> ArenaNode<T> {
    pub fn new(value: T) -> Self {
        Self { value, generation: 0, is_temp_taken: false }
    }

    pub fn get(&self) -> &T {
        debug_assert!(!self.is_temp_taken, "ArenaNode::get called while value is temp-taken");
        &self.value
    }

    pub fn get_mut(&mut self) -> &mut T {
        debug_assert!(!self.is_temp_taken, "ArenaNode::get_mut called while value is temp-taken");
        &mut self.value
    }

    pub fn generation(&self) -> u32 { self.generation }

    pub fn replace(&mut self, new_value: T) -> T {
        debug_assert!(!self.is_temp_taken, "ArenaNode::replace called while value is temp-taken");
        self.generation += 1;
        std::mem::replace(&mut self.value, new_value)
    }

    pub fn free(&mut self) {
        debug_assert!(!self.is_temp_taken, "ArenaNode::free called while value is temp-taken");
        self.generation += 1;
    }

    pub fn temp_take(&mut self) -> T where T: Default {
        debug_assert!(!self.is_temp_taken, "ArenaNode::temp_take called on already taken slot");
        self.is_temp_taken = true;
        std::mem::take(&mut self.value)
    }

    pub fn temp_replace_back(&mut self, value: T) {
        debug_assert!(self.is_temp_taken, "ArenaNode::temp_replace_back called on a slot that was not taken");
        self.is_temp_taken = false;
        self.value = value;
    }
}

