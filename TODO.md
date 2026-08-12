## TODO

### features
- karray dynamic indexing

### to test
- test zif multi chain
- test memblk (basic coverage now in tc28_mem_blk)

### done
- hierarchy module implementation — routing double-take + expression-operand
  remap fixed; covered by tc37_hier_basic / tc38_hier_deep_sibling

### known issues (found while building cupg, 2026-07)
- **mem_ele drops slice views on its index signal** (keeps only the ident):
  `mem_ele(blk, addr[7, 2], ...)` indexes with the full `addr`. Workaround:
  materialize the slice into a wire first. Fix: honor the index slice.
- **explicit `.default()` / plain bare assigns override zero-cycle-gated
  (zif/zstate) assigns** on the same wire regardless of declaration order when
  used OUTSIDE a seq (UpdatePool priority). Intended? If not, priority of
  wire defaults should sit below z-gated user assigns in bare context too.
- **Karray dynamic-index auto-names compound per mux level**
  ("…_DMUXIDX_<id>" repeated), which forced MAX_NAME_LEN 128 → 256 for a
  32-entry regfile. Fix: non-compounding names, then shrink MAX_NAME_LEN back.
- **bare clocked assigns (outside any flow block) panic** at emission
  ("PosEdge with no clk source") — workaround is a constant-true zif wrapper;
  a default clock source would be friendlier.
