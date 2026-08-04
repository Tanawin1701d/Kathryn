import sys, re
fn = sys.argv[1]
spec = {}
for line in sys.stdin:
    s=line.strip()
    if not s or s.startswith('#'): continue
    ln, tag = s.split('=',1)
    spec[int(ln)] = tag.strip()
TRIP  = re.compile(r'(?<!/)///(?!/)')
UPTAG = re.compile(r'(?<!/)///(?!/)\s*([A-Z][A-Z0-9_+]*)')   # old uppercase tag
with open(fn) as f:
    lines = f.read().split('\n')
# 1) globally strip OLD uppercase ///tags, but NEVER ///DC
for i,l in enumerate(lines):
    m = UPTAG.search(l)
    if m and m.group(1) != 'DC':
        lines[i] = l[:m.start()].rstrip()
# 2) apply new tags per spec (skip ///DC lines entirely)
n=0
for ln, tag in spec.items():
    i=ln-1
    if i<0 or i>=len(lines): print(f"  WARN line {ln} oob"); continue
    m = TRIP.search(lines[i])
    if m and re.match(r'///\s*DC\b', lines[i][m.start():]):
        print(f"  SKIP DC line {ln}"); continue
    lines[i] = lines[i].rstrip() + f'   ///{tag}'
    n+=1
with open(fn,'w') as f:
    f.write('\n'.join(lines))
print(f"{fn}: stripped-old-tags + applied {n}")
