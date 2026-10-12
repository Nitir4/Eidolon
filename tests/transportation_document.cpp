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
static void structuralEdits() {
    using Axis = TransportationDocument::Axis;
    // A deliberately asymmetric matrix exposes row/column swaps. Fractional
    // imported quantities and padded dimension records must survive copying.
    const std::string bytes = "4000\r\n5\r\n2\r\n1\r\n\"Structural input\"\r\n02\r\n03\r\n"
        "\"S/D Name\"\r\n\"D-A\"\r\n\"D-B\"\r\n\"D-C\"\r\n\"top reserve\"\r\n"
        "\"Factory A\"\r\n\" +2.5000 \"\r\n\"3e0\"\r\n\"-4.75\"\r\n\"4.125\"\r\n"
        "\"Factory B\"\r\n\"5\"\r\n\"6.000\"\r\n\"7e0\"\r\n\"8.875\"\r\n"
        "\"left reserve\"\r\n\"1.25\"\r\n\"2e0\"\r\n\"9.75\"\r\n\"tail reserve\"\r\n";
    const std::vector<std::vector<std::string>> sources = {
        {"Factory A", " +2.5000 ", "3e0", "-4.75", "4.125"},
        {"Factory B", "5", "6.000", "7e0", "8.875"}};
    const std::vector<std::vector<std::string>> destinations = {
        {"D-A", " +2.5000 ", "5", "1.25"},
        {"D-B", "3e0", "6.000", "2e0"},
        {"D-C", "-4.75", "7e0", "9.75"}};
    const auto roundTrip = [](const TransportationDocument& doc) {
        check(TransportationDocument::decode(doc.encode()).encode() == doc.encode(),
              "Structural operation produced an inconsistent encoded document");
    };
    auto original = TransportationDocument::decode(bytes);
    for (std::size_t i = 0; i < sources.size(); ++i)
        check(original.line(Axis::Source, i) == sources[i], "Source extraction mapped the wrong cells");
    for (std::size_t i = 0; i < destinations.size(); ++i)
        check(original.line(Axis::Destination, i) == destinations[i], "Destination extraction mapped the wrong cells");
    check(original.encode() == bytes, "Copying a line changed the document");

    for (std::size_t position = 0; position <= sources.size(); ++position) {
        auto doc = original;
        doc.insert(Axis::Source, position);
        check(doc.problem().costs.size() == 3, "Source insertion did not update model dimensions");
        check(doc.line(Axis::Source, position) == std::vector<std::string>({"", "0.00", "0.00", "0.00", ""}),
              "Inserted source defaults differ from observed EditGrid behavior");
        for (std::size_t old = 0; old < sources.size(); ++old)
            check(doc.line(Axis::Source, old + (old >= position ? 1 : 0)) == sources[old],
                  "Source insertion displaced or normalized an existing record");
        check(doc.cell(0, 4) == "top reserve" && doc.cell(4, 0) == "left reserve" &&
                  doc.cell(4, 1) == "1.25" && doc.cell(4, 3) == "9.75" && doc.cell(4, 4) == "tail reserve",
              "Source insertion lost the reserved edges or demand row");
        roundTrip(doc);
        doc.erase(Axis::Source, position);
        // Structural edits may rewrite dimension spelling; all cell records
        // still have to match the original independently authored matrix.
        for (std::size_t old = 0; old < sources.size(); ++old)
            check(doc.line(Axis::Source, old) == sources[old], "Source insert/erase failed to restore cells");
        roundTrip(doc);
    }
    for (std::size_t position = 0; position <= destinations.size(); ++position) {
        auto doc = original;
        doc.insert(Axis::Destination, position);
        check(doc.problem().costs[0].size() == 4, "Destination insertion did not update model dimensions");
        check(doc.line(Axis::Destination, position) == std::vector<std::string>({"", "0.00", "0.00", ""}),
              "Inserted destination defaults differ from observed EditGrid behavior");
        for (std::size_t old = 0; old < destinations.size(); ++old)
            check(doc.line(Axis::Destination, old + (old >= position ? 1 : 0)) == destinations[old],
                  "Destination insertion displaced or normalized an existing record");
        check(doc.cell(0, 5) == "top reserve" && doc.cell(1, 5) == "4.125" &&
                  doc.cell(2, 5) == "8.875" && doc.cell(3, 0) == "left reserve" && doc.cell(3, 5) == "tail reserve",
              "Destination insertion lost the reserved edges or supply column");
        roundTrip(doc);
        doc.erase(Axis::Destination, position);
        for (std::size_t old = 0; old < destinations.size(); ++old)
            check(doc.line(Axis::Destination, old) == destinations[old], "Destination insert/erase failed to restore cells");
        roundTrip(doc);
    }
    for (std::size_t position = 0; position < sources.size(); ++position) {
        auto doc = original;
        doc.erase(Axis::Source, position);
        check(doc.line(Axis::Source, 0) == sources[1 - position], "Source erase retained the wrong line");
        check(doc.cell(2, 1) == "1.25" && doc.cell(2, 4) == "tail reserve", "Source erase damaged bottom edge");
        roundTrip(doc);
    }
    for (std::size_t position = 0; position < destinations.size(); ++position) {
        auto doc = original;
        doc.erase(Axis::Destination, position);
        for (std::size_t old = 0; old < destinations.size(); ++old) {
            if (old == position) continue;
            check(doc.line(Axis::Destination, old - (old > position ? 1 : 0)) == destinations[old],
                  "Destination erase shifted cells incorrectly");
        }
        check(doc.cell(1, 3) == "4.125" && doc.cell(3, 3) == "tail reserve", "Destination erase damaged right edge");
        roundTrip(doc);
    }
    auto sourcePaste = original;
    sourcePaste.paste(Axis::Source, 1, original.line(Axis::Source, 0));
    check(sourcePaste.line(Axis::Source, 1) == sources[0] && sourcePaste.line(Axis::Source, 0) == sources[0],
          "Source paste failed to retain copied alias or lexical quantity/costs");
    check(sourcePaste.problem().supply[1] == 4.125 && sourcePaste.problem().costs[1][0] == 2.5,
          "Source paste did not update its numeric model");
    check(sourcePaste.encode().find("\r\n02\r\n03\r\n") != std::string::npos,
          "Paste unnecessarily rewrote padded dimension records");
    roundTrip(sourcePaste);
    auto destinationPaste = original;
    destinationPaste.paste(Axis::Destination, 2, original.line(Axis::Destination, 0));
    check(destinationPaste.line(Axis::Destination, 2) == destinations[0], "Destination paste lost lexical cells");
    check(destinationPaste.problem().demand[2] == 1.25 && destinationPaste.problem().costs[1][2] == 5,
          "Destination paste did not update its numeric model");
    check(destinationPaste.cell(1, 4) == "4.125" && destinationPaste.cell(0, 4) == "top reserve" &&
              destinationPaste.cell(3, 4) == "tail reserve", "Paste changed unrelated reserved/quantity records");
    roundTrip(destinationPaste);

    auto invalid = original;
    const auto before = invalid.encode();
    const auto unchangedRejects = [&](const std::function<void()>& action) {
        rejects(action);
        check(invalid.encode() == before, "Rejected structural operation partially changed records");
    };
    unchangedRejects([&] { invalid.insert(Axis::Source, 3); });
    unchangedRejects([&] { invalid.insert(Axis::Destination, 4); });
    unchangedRejects([&] { invalid.erase(Axis::Source, 2); });
    unchangedRejects([&] { invalid.erase(Axis::Destination, 3); });
    unchangedRejects([&] { invalid.line(Axis::Source, 2); });
    unchangedRejects([&] { invalid.line(Axis::Destination, 3); });
    unchangedRejects([&] { invalid.paste(Axis::Source, 2, sources[0]); });
    unchangedRejects([&] { invalid.paste(Axis::Destination, 3, destinations[0]); });
    unchangedRejects([&] { invalid.paste(Axis::Source, 0, {"short"}); });
    unchangedRejects([&] { invalid.paste(Axis::Destination, 0, {"too", "many", "1", "2", "3"}); });
    for (const auto& value : {"nan", "inf", "1junk", "1e309"}) {
        auto line = sources[0]; line[3] = value;
        unchangedRejects([&] { invalid.paste(Axis::Source, 0, line); });
    }
    auto badQuantity = destinations[0]; badQuantity.back() = "-1";
    unchangedRejects([&] { invalid.paste(Axis::Destination, 0, badQuantity); });
    auto badSupply = sources[0]; badSupply.back() = "-1";
    unchangedRejects([&] { invalid.paste(Axis::Source, 0, badSupply); });
    auto badDestinationCost = destinations[0]; badDestinationCost[2] = "nan";
    unchangedRejects([&] { invalid.paste(Axis::Destination, 0, badDestinationCost); });
    auto badAlias = sources[0]; badAlias.front() = "bad\nname";
    unchangedRejects([&] { invalid.paste(Axis::Source, 0, badAlias); });
    auto minimum = TransportationDocument::blank(1, 1);
    const auto minimumBefore = minimum.encode();
    rejects([&] { minimum.erase(Axis::Source, 0); });
    rejects([&] { minimum.erase(Axis::Destination, 0); });
    check(minimum.encode() == minimumBefore, "Minimum dimension rejection mutated document");
    auto maximum = TransportationDocument::blank(100, 100);
    const auto maximumBefore = maximum.encode();
    rejects([&] { maximum.insert(Axis::Source, 0); });
    rejects([&] { maximum.insert(Axis::Destination, 100); });
    check(maximum.encode() == maximumBefore, "Maximum dimension rejection mutated document");
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
        structuralEdits();
        std::cout << "Transportation document preservation, editing and safety checks passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
