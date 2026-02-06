import os
import re
from collections import defaultdict

# Change this to your target directory
TARGET_DIR = "./core"

# Regex: ///CTRL <GROUP>
CTRL_PATTERN = re.compile(r"///\s*CTRL\s+(\w+)")

total = 0
def scan_folder(folder):
    global total
    counts = defaultdict(int)
    for root, _, files in os.walk(folder):
        for fname in files:
            path = os.path.join(root, fname)

            # Skip binary-like files
            try:
                with open(path, "r", encoding="utf-8", errors="ignore") as f:
                    for line in f:
                        match = CTRL_PATTERN.search(line)
                        if match:
                            group = match.group(1)
                            counts[group] += 1
                            total += 1
            except Exception as e:
                print(f"Skip {path}: {e}")

    return counts

if __name__ == "__main__":
    counts = scan_folder(TARGET_DIR)

    print("=== CTRL TAG COUNTS ===")
    for group, count in sorted(counts.items()):
        print(f"{group:10s}, {count}")
    print(f"Total: {total}")
