#include <tora/compat/transportation_document.hpp>
#include <functional>
#include <iostream>
#include <stdexcept>

using tora::compat::TransportationDocument;
static void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
static void rejects(const std::function<void()>& action) {
    try { action(); } catch (const std::exception&) { return; }
    throw std::runtime_error("Invalid document edit accepted");
}
int main() {
    try {
        // Independently authored records exercise lexical and reserved-cell
        // preservation, separately from numeric solver expectations.
        const std::string bytes = "4000\r\n5\r\n2\r\n1\r\n\"A, title\"\r\n1\r\n1\r\n"
            "\"S/D Name\"\r\n\"Depot\"\r\n\"reserved\"\r\n"
            "\"Factory\"\r\n\" +2.5000 \"\r\n\"4e0\"\r\n"
            "\"\"\r\n\"4\"\r\n\"tail\"\r\n";
        auto doc = TransportationDocument::decode(bytes);
        check(doc.encode() == bytes, "Unchanged file lost exact records");
        auto padded = bytes; padded.replace(padded.find("\r\n1\r\n1\r\n\"S/D Name"), 8, "\r\n01\r\n01\r\n");
        check(TransportationDocument::decode(padded).encode() == padded, "Dimension lexemes were not retained");
        check(doc.problem().costs[0][0] == 2.5, "Numeric parsing failed");
        doc.setCell(1, 1, "1.234567");
        check(doc.cell(1, 1) == "1.23", "Edited costs must use observed D=2");
        check(doc.problem().costs[0][0] == 1.23, "Rounded model not synchronized");
        check(doc.cell(1, 2) == "4e0" && doc.cell(0, 2) == "reserved" && doc.cell(2, 2) == "tail",
              "Editing changed untouched/reserved records");
        const auto before = doc.encode();
        for (const auto& value : {"nan", "inf", "1junk", "1e309", "1\n2"})
            rejects([&] { doc.setCell(1, 1, value); });
        rejects([&] { doc.setCell(1, 2, "-1"); });
        rejects([&] { doc.setCell(0, 0, "changed header"); });
        rejects([&] { doc.setCell(99, 99, "1"); });
        rejects([&] { doc.setTitle("bad\r\ntitle"); });
        rejects([&] { doc.setTitle(std::string("bad\0title", 9)); });
        rejects([&] { doc.setTitle("caf\xc3\xa9"); });
        check(doc.encode() == before, "Failed edit mutated the document");
        doc.setCell(1, 1, "-2.5");
        doc.setCell(0, 1, "D, \"one\"");
        doc.setTitle("A \"quoted\" title");
        auto reopened = TransportationDocument::decode(doc.encode());
        check(reopened.problem().title == doc.problem().title && reopened.cell(0, 1) == "D, \"one\"",
              "Doubled quotes failed native recovery round trip");
        auto blank = TransportationDocument::blank(2, 3, "New problem");
        check(blank.cell(1, 1) == "0.00" && blank.cell(1, 4).empty() && blank.cell(3, 1).empty(),
              "New grid defaults disagree with observed input");
        check(TransportationDocument::decode(blank.encode()).encode() == blank.encode(), "Blank-file round trip failed");
        blank.setCell(1, 4, "20");
        check(blank.cell(1, 4) == "20", "Integer quantities must not acquire decimal zeros");
        blank.setCell(2, 4, "30.125");
        check(blank.cell(2, 4) == "30", "Quantity commitment must round to whole numbers");
        blank.setCell(2, 4, "21.5");
        check(blank.cell(2, 4) == "22", "Quantity half tie must round away from zero");
        check(blank.problem().supply[0] == 20 && blank.problem().demand[0] == 0, "Unbalanced draft rejected");
        auto model = tora::transportation::Problem::blank(1, 1);
        model.costs = {{2.5}}; model.supply = {4}; model.demand = {4};
        const auto created = TransportationDocument::create(model);
        check(created.cell(1, 1) == "2.5" && created.cell(1, 2) == "4", "Model serialization lost round-trip precision");
        rejects([] { TransportationDocument::blank(0, 2); });
        rejects([] { TransportationDocument::blank(101, 2); });
        std::cout << "Transportation document preservation, editing and safety checks passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
