import re
from pathlib import Path

INPUT_FILE = "../data/clean/pg768_clean.txt"
OUTPUT_DIR = Path("../data/paragraphs")

OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

with open(INPUT_FILE, encoding="utf8") as f:
    text = f.read()

# Paragraphs are separated by one or more blank lines
paragraphs = [
    p.strip()
    for p in re.split(r"\n\s*\n+", text)
    if p.strip()
]

for i, paragraph in enumerate(paragraphs, start=1):
    filename = OUTPUT_DIR / f"paragraph_{i:05d}.txt"

    with open(filename, "w", encoding="utf8") as f:
        f.write(paragraph)

print(f"Saved {len(paragraphs)} paragraphs to {OUTPUT_DIR}")