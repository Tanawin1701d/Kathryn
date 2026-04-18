pub struct ArenaNode<T> {
    value      : Option<T>,
    generation : u32,
}

impl<T> ArenaNode<T> {
    pub fn new(value: T) -> Self {
        Self { value: Some(value), generation: 0 }
    }

    pub fn empty() -> Self {
        Self { value: None, generation: 0 }
    }

    pub fn get    (&self)     -> Option<&T>     { self.value.as_ref() }
    pub fn get_mut(&mut self) -> Option<&mut T> { self.value.as_mut() }
    pub fn generation(&self)  -> u32            { self.generation     }
}