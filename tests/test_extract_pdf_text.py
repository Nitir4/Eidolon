"""Exercise the cross-platform PDF reader CLI with no native tools on PATH."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from pypdf import PdfWriter
from pypdf.generic import DecodedStreamObject, DictionaryObject, NameObject

READER = Path(__file__).with_name('extract_pdf_text.py')


class PdfTextReader(unittest.TestCase):
    def read(self, path):
        return subprocess.run([sys.executable, str(READER), str(path)],
                              env={**os.environ, 'PATH': ''}, capture_output=True)

    def write_pdf(self, path):
        writer = PdfWriter()
        for content in [b'caf\\351', b'Second page']:
            page = writer.add_blank_page(width=200, height=100)
            font = DictionaryObject({NameObject('/Type'): NameObject('/Font'),
                                     NameObject('/Subtype'): NameObject('/Type1'),
                                     NameObject('/BaseFont'): NameObject('/Helvetica'),
                                     NameObject('/Encoding'): NameObject('/WinAnsiEncoding')})
            page[NameObject('/Resources')] = DictionaryObject({
                NameObject('/Font'): DictionaryObject({NameObject('/F1'): font})})
            stream = DecodedStreamObject()
            stream.set_data(b'BT /F1 12 Tf 10 50 Td (' + content + b') Tj ET')
            page[NameObject('/Contents')] = stream
        with path.open('wb') as handle:
            writer.write(handle)

    def test_all_pages_are_extracted_as_utf8_without_native_tools(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / 'native.pdf'
            self.write_pdf(path)
            result = self.read(path)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.decode('utf-8'), 'café\nSecond page')

    def test_unicode_and_spaces_in_pdf_path(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / 'Native install café.pdf'
            self.write_pdf(path)
            result = self.read(path)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn('café', result.stdout.decode('utf-8'))

    def test_missing_pdf_returns_clear_failure(self):
        with tempfile.TemporaryDirectory() as folder:
            result = self.read(Path(folder) / 'missing.pdf')
            self.assertNotEqual(result.returncode, 0)
            self.assertIn(b'PDF text extraction failed', result.stderr)
            self.assertEqual(result.stdout, b'')

    def test_malformed_pdf_returns_clear_failure(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / 'invalid.pdf'
            path.write_bytes(b'not a PDF')
            result = self.read(path)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn(b'PDF text extraction failed', result.stderr)
            self.assertEqual(result.stdout, b'')


if __name__ == '__main__':
    unittest.main()
