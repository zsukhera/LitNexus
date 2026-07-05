from collections import Counter
import spacy

# Load spaCy model
nlp = spacy.load("en_core_web_sm")

# Read the novel
with open("../data/clean/pg768_clean.txt", encoding="utf8") as f:
    text = f.read()

# Process the text
doc = nlp(text)

counter = Counter()

# Ignore these if they appear by themselves
BAD_NAMES = {
    "Miss",
    "Mr",
    "Mrs",
    "Master",
    "Sir",
    "Lady",
}

# Common false positives for Wuthering Heights
BLACKLIST = {
    "Wuthering Heights",
    "Hush",
    "Nay",
    "Sabbath",
    "Green",
    "ech",
    "mim",
    "Gimmerton Kirk",
    "kirkyard",
}

# Merge different names referring to the same character
ALIASES = {
    "Catherine Earnshaw": "Catherine",
    "Catherine Linton": "Catherine",
    "Miss Catherine": "Catherine",
    "Cathy": "Catherine",
    "Miss Cathy": "Catherine",

    "Edgar Linton": "Edgar",

    "Ellen Dean": "Ellen",
    "Nelly Dean": "Ellen",

    "Isabella Linton": "Isabella",

    "Hareton Earnshaw": "Hareton",
    "Little Hareton": "Hareton",

    "Master Heathcliff": "Heathcliff",

    "Linton Heathcliff": "Linton",
}

for ent in doc.ents:
    if ent.label_ != "PERSON":
        continue

    name = ent.text.strip()

    # Remove possessives
    if name.endswith("'s"):
        name = name[:-2]
    if name.endswith("’s"):
        name = name[:-2]

    # Remove extra whitespace
    name = " ".join(name.split())

    # Skip titles
    if name in BAD_NAMES:
        continue

    # Skip obvious false positives
    if name in BLACKLIST:
        continue

    # Merge aliases
    name = ALIASES.get(name, name)

    counter[name] += 1

# Save results
with open("characters.txt", "w", encoding="utf8") as f:
    for name, count in counter.most_common():
        if count >= 2:
            f.write(f"{name}\t{count}\n")

print(f"Found {len(counter)} unique character names.")
print("Results saved to characters.txt")