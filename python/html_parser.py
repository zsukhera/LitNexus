from bs4 import BeautifulSoup
from config import INPUT_HTML, OUTPUT_TEXT
from utils import save_text


REMOVE_TAGS = [
    "script",
    "style",
    "svg",
    "img",
    "figure",
    "iframe",
    "noscript",
    "footer",
    "header",
]


def load_html(filepath):
    with open(filepath, "r", encoding="utf-8") as f:
        return f.read()


def remove_unwanted_tags(soup):
    for tag in REMOVE_TAGS:
        for item in soup.find_all(tag):
            item.decompose()


def extract_clean_text(soup):
    lines = []

    body = soup.find("body")

    if body is None:
        body = soup

    for element in body.descendants:

        if element.name == "h1":
            title = element.get_text(" ", strip=True)
            lines.append("\n" + "=" * 70)
            lines.append(title.upper())
            lines.append("=" * 70 + "\n")

        elif element.name == "h2":
            chapter = element.get_text(" ", strip=True)
            lines.append("\n")
            lines.append("-" * 60)
            lines.append(chapter.upper())
            lines.append("-" * 60)
            lines.append("")

        elif element.name == "p":
            text = element.get_text(" ", strip=True)

            if text:
                lines.append(text)
                lines.append("")

    return "\n".join(lines)


def main():

    print("Reading HTML...")

    html = load_html(INPUT_HTML)

    print("Parsing HTML...")

    soup = BeautifulSoup(html, "lxml")

    print("Removing unwanted tags...")

    remove_unwanted_tags(soup)

    print("Extracting clean text...")

    cleaned = extract_clean_text(soup)

    print("Saving...")

    save_text(cleaned, OUTPUT_TEXT)

    print("Done.")


if __name__ == "__main__":
    main()