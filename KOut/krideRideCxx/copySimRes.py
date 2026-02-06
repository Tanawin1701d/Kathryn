import shutil
from pathlib import Path


def transfer():
    SRC = Path(".")
    DST = Path("/media/tanawin/tanawin1701e/project2/KathrynDoc/O3Anal/result")

    for file in SRC.rglob("*result"):
        if file.is_file():
            target = DST / file.relative_to(SRC)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(file, target)

if __name__ == "__main__":
    transfer()