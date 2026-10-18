"""Extract UTF-8 text from every PDF page without native command-line tools."""
import sys


def main():
    if len(sys.argv) != 2:
        print('Usage: extract_pdf_text.py PDF', file=sys.stderr)
        return 2
    try:
        from pypdf import PdfReader
        text = '\n'.join(page.extract_text() or '' for page in PdfReader(sys.argv[1]).pages)
        sys.stdout.buffer.write(text.encode('utf-8'))
    except Exception as error:
        print(f'PDF text extraction failed for {sys.argv[1]}: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
