import sys, os, importlib
sys.path.insert(0, '/media/tanawin/tanawin1701e/project2/Kathryn/src/example/o3')
import countMeasure as cm; importlib.reload(cm)
from collections import defaultdict
for f in sys.argv[1:]:
    per=defaultdict(float); unk=defaultdict(int)
    for line in open(f):
        if cm.DC_PATTERN.search(line): continue
        ms=cm.CAT_PATTERN.findall(line)
        if not ms:
            if '///' in line:
                for t in cm.ANY_TAG.findall(line):
                    if t!='DC' and not cm.VALID_TAG.match(t): unk[t]+=1
            continue
        for expr,grp in ms:
            cats=[c for c in expr.split('+') if c]; s=1/len(cats)
            for c in cats: per[c]+=s
    name=os.path.basename(f)
    flag='  <-- STRAY '+str(dict(unk)) if unk else ''
    print(f"{name:16s} tot={cm.fmt(sum(per.values())):6s} " + " ".join(f"{c}:{cm.fmt(per[c])}" for c in cm.CATEGORIES if per.get(c)) + flag)
