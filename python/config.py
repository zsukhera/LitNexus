from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent

RAW_DATA = ROOT_DIR / "data" / "raw"
CLEAN_DATA = ROOT_DIR / "data" / "clean"

INPUT_HTML = RAW_DATA / "pg768-images.html"
OUTPUT_TEXT = CLEAN_DATA / "pg768_clean.txt"