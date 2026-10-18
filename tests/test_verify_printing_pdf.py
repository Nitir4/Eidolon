"""Horizontal table bands must preserve labelled cells, not just their words."""
import json
from pathlib import Path
import unittest

from verify_printing_pdf import normalized, trace_content


PREFIX="SIMPLEX TABLEAUS -- (Starting All-Slack Method) Title: Band regression Iteration 1 "
SINGLE=(PREFIX+"Basic x1 x2 Solution z (max) -3.00 -2.00 0.00 "
        "sx3 1.00 1.00 4.00 Lower Bound 0.00 0.00 "
        "Upper Bound infinity infinity Unrestr'd (y/n)? n n")
BANDED=(PREFIX+"Basic x1 x2 z (max) -3.00 -2.00 sx3 1.00 1.00 "
        "Lower Bound 0.00 0.00 Upper Bound infinity infinity Unrestr'd (y/n)? n n "
        "Basic Solution z (max) 0.00 sx3 4.00")


class TraceBands(unittest.TestCase):
    def test_same_cells_in_two_bands(self):
        self.assertEqual(trace_content(SINGLE),trace_content(BANDED))

    def test_missing_solution_column_is_rejected(self):
        missing=BANDED[:BANDED.index(" Basic Solution")]
        self.assertNotEqual(trace_content(SINGLE),trace_content(missing))

    def test_wrong_rhs_or_column_assignment_is_rejected(self):
        self.assertNotEqual(trace_content(SINGLE),trace_content(BANDED.replace("sx3 4.00","sx3 40.00")))
        self.assertNotEqual(trace_content(SINGLE),trace_content(BANDED.replace("-3.00 -2.00","-2.00 -3.00")))

    def test_missing_row_or_variable_bound_is_rejected(self):
        self.assertNotEqual(trace_content(SINGLE),trace_content(BANDED.replace(" sx3 4.00","")))
        self.assertNotEqual(trace_content(SINGLE),trace_content(BANDED.replace("Lower Bound 0.00 0.00","Lower Bound 0.00")))
        self.assertNotEqual(trace_content(SINGLE),trace_content(BANDED.replace(
            "Lower Bound 0.00 0.00 Upper Bound infinity infinity",
            "Upper Bound infinity infinity Lower Bound 0.00 0.00")))

    def test_duplicate_band_or_truncated_row_is_rejected(self):
        for invalid in (BANDED+" Basic Solution z (max) 0.00 sx3 4.00",
                        BANDED.replace("z (max) -3.00 -2.00","z (max) -3.00")):
            with self.subTest(invalid=invalid),self.assertRaises(AssertionError):
                trace_content(invalid)

    def test_iteration_and_phase_identity_are_preserved(self):
        for changed in (BANDED.replace("Iteration 1","Iteration 2"),
                        BANDED.replace("Iteration 1","Phase 1 (Iter 1")):
            self.assertNotEqual(trace_content(SINGLE),trace_content(changed))
        self.assertNotEqual(trace_content(SINGLE),trace_content(BANDED+" Iteration 1 "+BANDED[len(PREFIX):]))

    def test_all_existing_trace_fixtures_parse(self):
        records=json.loads(Path(__file__).with_name("fixtures").joinpath("printing.json").read_text())
        for record in records:
            if record["method"] in ("final","graphical"):continue
            text=normalized(" ".join(page["text"] for page in record["pages"][1:]))
            with self.subTest(case=record["case"]):
                prefix,iterations=trace_content(text)
                self.assertTrue(prefix.startswith("SIMPLEX TABLEAUS"))
                self.assertTrue(iterations)


if __name__=="__main__":unittest.main()
