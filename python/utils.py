from pathlib import Path


def ensure_directory(path):
    """
    Create directory if it doesn't exist.
    """
    Path(path).mkdir(parents=True, exist_ok=True)


def save_text(text, filename):
    """
    Save text using UTF-8 encoding.
    """
    filename.parent.mkdir(parents=True, exist_ok=True)

    with open(filename, "w", encoding="utf-8") as f:
        f.write(text)

    print(f"\nSaved cleaned text to:\n{filename}")