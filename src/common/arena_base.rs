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
}

pub struct ArenaNode<T> {
    value      : T,
    generation : u32,

}

impl<T> ArenaNode<T> {
    pub fn new(value: T) -> Self {
        Self { value, generation: 0 }
    }

    pub fn get    (&self)     -> &T     { &self.value     }
    pub fn get_mut(&mut self) -> &mut T { &mut self.value }
    pub fn generation(&self)  -> u32    { self.generation }

    pub fn replace(&mut self, new_value: T) -> T {
        self.generation += 1;
        std::mem::replace(&mut self.value, new_value)
    }

    pub fn free(&mut self) { self.generation += 1; }
}

