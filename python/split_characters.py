import re
from pathlib import Path

INPUT_FILE = "../data/clean/pg768_clean.txt"
OUTPUT_DIR = Path("../data/chapters")

OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

with open(INPUT_FILE, encoding="utf8") as f:
    text = f.read()

# Matches:
# CHAPTER I
# CHAPTER IV
# CHAPTER 1
# Chapter 15
chapter_pattern = re.compile(
    r"(?im)^(chapter\s+(?:[ivxlcdm]+|\d+).*)$"
)

matches = list(chapter_pattern.finditer(text))

if not matches:
    print("No chapters found.")
    exit()

for i, match in enumerate(matches):
    start = match.start()

    if i + 1 < len(matches):
        end = matches[i + 1].start()
    else:
        end = len(text)

    chapter_text = text[start:end].strip()

    filename = OUTPUT_DIR / f"chapter_{i+1:02d}.txt"

    with open(filename, "w", encoding="utf8") as f:
        f.write(chapter_text)

print(f"Saved {len(matches)} chapters to {OUTPUT_DIR}")