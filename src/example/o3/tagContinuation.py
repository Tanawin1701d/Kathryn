#!/usr/bin/env python3
"""
tagContinuation.py DIR [DIR ...]

Convention: count EVERY physical code line. A multi-line statement's continuation
lines each get the SAME category as the statement (the nearest ///<CAT> <GROUP>
tag above them). Only these stay untagged: blank lines, comments, preprocessor,
`namespace`/`using`, structural keywords (begin/end/endmodule/endcase/...), and
bare closer lines (only brackets/braces/semicolons/commas). Lines already tagged
(///CAT or ///DC) are left as-is. Append-only (never alters code).
"""
import re, os, sys, glob

_CATS = ('CTRL_HWD', 'CTRL_CL', 'CTRL_DT', 'CTRL_HC',
         'DATA_HWD', 'DATA_CL', 'DATA_DT', 'DATA_HC', 'MD', 'PARAM', 'HLH')
_ALT = '|'.join(sorted(_CATS, key=len, reverse=True))   # longest-first
CAT = re.compile(rf'(?<!/)///(?!/)\s*((?:{_ALT})(?:\+(?:{_ALT}))*)\s+(\w+)')
DC  = re.compile(r'(?<!/)///(?!/)\s*DC\b')
BARE = re.compile(r'^[\s)}\];,({]*;?\s*$')                       # only brackets/punct
KW   = re.compile(r'^\s*(begin|end|endmodule|endcase|endgenerate|endfunction|endtask)\b')
CMT  = re.compile(r'^\s*(//|/\*|\*)')
PRE  = re.compile(r'^\s*[#`]')
NS   = re.compile(r'^\s*(namespace|using)\b')
ASRT = re.compile(r'^\s*assert\s*\(')                            # host assert -> excluded
BELSE = re.compile(r'^\s*(\}|end)?\s*(else|zelse)\s*(\{|begin)?\s*$')  # bare else-junction

def process(path):
    with open(path) as f:
        lines = f.read().split('\n')
    cur = None; inblk = False; changed = 0
    for i, l in enumerate(lines):
        code = l.split('///')[0]           # code portion, before any /// comment
        if inblk:
            if '*/' in l: inblk = False
            continue
        if '/*' in code and '*/' not in code:
            inblk = True; continue
        m = CAT.search(l)
        if m:                              # already category-tagged -> statement head
            cur = (m.group(1), m.group(2)); continue
        if DC.search(l):                   # excluded line -> leave, don't inherit
            continue
        s = code.strip()
        if (s == '' or CMT.match(code) or PRE.match(code) or NS.match(code)
                or KW.match(code) or BARE.match(code) or ASRT.match(code)
                or BELSE.match(code)):
            continue
        if cur:                            # real-code continuation line -> inherit
            lines[i] = l.rstrip() + f' ///{cur[0]} {cur[1]}'
            changed += 1
    if changed:
        with open(path, 'w') as f:
            f.write('\n'.join(lines))
    return changed

def main():
    exts = ('*.h', '*.cpp', '*.v', '*.vh')
    total = 0
    for d in sys.argv[1:]:
        for ext in exts:
            for path in glob.glob(os.path.join(d, '**', ext), recursive=True):
                if 'obj_dir' in path or os.path.basename(path) in ('btb.h', 'gshare.h'):
                    continue
                c = process(path)
                if c:
                    print(f'  {os.path.relpath(path)}: +{c}')
                    total += c
    print(f'TOTAL continuation lines tagged: {total}')

if __name__ == '__main__':
    main()
