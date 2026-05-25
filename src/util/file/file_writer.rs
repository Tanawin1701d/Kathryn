use std::fs::File;
use std::io::{self, BufWriter, Write};
use std::path::PathBuf;

// ---- FileWriter — batched, auto-flushing file sink ----
//
// Accumulates output in `pending` (application-level batch); flushes to the
// OS-buffered `inner` when the batch reaches `batch_cap` bytes, on an explicit
// `flush()` call, or when the instance is dropped.
//
// Panic / crash safety: Rust unwinds the stack on panic by default, so
// `Drop::drop` fires and flushes any remaining data automatically.

pub struct FileWriter {
    path      : PathBuf,
    inner     : BufWriter<File>,
    pending   : String,
    batch_cap : usize,
}

impl FileWriter {

    // ---- construction ----

    /// Open (or create + truncate) `path`; auto-flush when pending reaches `batch_cap` bytes.
    pub fn new(path: impl Into<PathBuf>, batch_cap: usize) -> io::Result<Self> {
        let path = path.into();
        let file = File::create(&path)?;
        Ok(Self {
            inner    : BufWriter::new(file),
            pending  : String::with_capacity(batch_cap),
            path,
            batch_cap,
        })
    }

    // ---- write surface ----

    /// Append `s` to the pending batch; auto-flush to the OS buffer when full.
    pub fn write(&mut self, s: &str) {
        self.pending.push_str(s);
        if self.pending.len() >= self.batch_cap {
            let _ = self.flush_batch();
        }
    }

    /// Force-flush: drain pending into BufWriter, then flush BufWriter to the OS.
    pub fn flush(&mut self) -> io::Result<()> { self.flush_batch() }

    // ---- query ----

    pub fn get_path(&self) -> &PathBuf { &self.path }

    // ---- internal ----

    fn flush_batch(&mut self) -> io::Result<()> {
        self.inner.write_all(self.pending.as_bytes())?;
        self.pending.clear();
        self.inner.flush()
    }
}

impl Drop for FileWriter {
    // Best-effort flush on destruction; errors are silently discarded.
    fn drop(&mut self) { let _ = self.flush_batch(); }
}
