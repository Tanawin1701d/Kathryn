use std::sync::atomic::Ordering;
use crate::util::file::file_writer::FileWriter;
use super::flags::DebugFlag;

// ---- OutputMode ----

pub enum OutputMode {
    Console,
    File,
    Both,
}

// ---- DebugSink — output target; flag state lives in FLAG_TABLE, not here ----

pub struct DebugSink {
    pub(super) mode        : OutputMode,
    pub(super) file_writer : Option<FileWriter>,
}

impl DebugSink {

    // Format and dispatch one debug line to the configured output(s).
    pub fn write_entry(&mut self, flag: DebugFlag, file: &str, line: u32, msg: &str) {
        let entry = format!("[{flag}]  {file}:{line}  |  {msg}\n");
        match self.mode {
            OutputMode::Console => { print!("{}", entry); }
            OutputMode::File    => { if let Some(fw) = &mut self.file_writer {
                                        fw.write(&entry);
                                        }
                                    }
            OutputMode::Both    => {
                print!("{}", entry);
                if let Some(fw) = &mut self.file_writer { fw.write(&entry); }
            }
        }
    }
}

// ---- DebugBuilder — fluent API to configure and install the debug session ----

pub struct DebugBuilder {
    flags     : Vec<DebugFlag>,
    mode      : OutputMode,
    file_path : Option<String>,
}

impl DebugBuilder {

    pub fn new() -> Self {
        Self { flags: Vec::new(), mode: OutputMode::Console, file_path: None }
    }

    // ---- flag selection ----

    /// Enable a single debug category.
    pub fn flag(mut self, f: DebugFlag) -> Self { self.flags.push(f); self }

    /// Enable a slice of debug categories at once.
    pub fn flags(mut self, fs: &[DebugFlag]) -> Self { self.flags.extend_from_slice(fs); self }

    /// Enable every defined DebugFlag.
    pub fn all_flags(mut self) -> Self { self.flags.extend_from_slice(&DebugFlag::ALL); self }

    // ---- output target ----

    /// Route output to stdout (default).
    pub fn to_console(mut self) -> Self { self.mode = OutputMode::Console; self }

    /// Route output to a file; path is a string literal embedded in source.
    pub fn to_file(mut self, path: impl Into<String>) -> Self {
        self.mode      = OutputMode::File;
        self.file_path = Some(path.into());
        self
    }

    /// Route output to both stdout and a file.
    pub fn to_both(mut self, path: impl Into<String>) -> Self {
        self.mode      = OutputMode::Both;
        self.file_path = Some(path.into());
        self
    }

    // ---- build ----

    /// Flip the AtomicBool for each enabled flag and install the output sink.
    /// Panics if called more than once — one debug session per process run.
    pub fn build(self) {
        for f in &self.flags {
            super::FLAG_TABLE[*f as usize].store(true, Ordering::Relaxed);
        }

        let file_writer = match &self.mode {
            OutputMode::File | OutputMode::Both => {
                let path = self.file_path
                    .expect("to_file / to_both requires a file path");
                Some(
                    FileWriter::new(path, 4096)
                        .expect("debug: cannot create log file")
                )
            }
            OutputMode::Console => None,
        };

        let sink = DebugSink { mode: self.mode, file_writer };

        // OnceLock::set returns Err if already set; FileWriter is not Debug so we can't .expect().
        if super::DEBUG_SINK.set(std::sync::Mutex::new(sink)).is_err() {
            panic!("debug::init().build() called twice — one debug session per run");
        }
    }
}
