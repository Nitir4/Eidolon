#include <tora/compat/transportation_document.hpp>
#include <tora/transportation/final_summary.hpp>
#include <cassert>
#include <stdexcept>
using namespace tora;
int main() {
    const core::NumberFormat scientific{core::Notation::Scientific,2,3};
    auto doc=compat::TransportationDocument::blank(2,3,"Format draft",scientific);
    assert(doc.problem().format.n==2 && doc.problem().format.d==3);
    assert(doc.cell(1,1)=="0.00e+000");
    doc.setCell(1,1,"1.23456");assert(doc.cell(1,1)=="1.23e+000");
    doc.setCell(1,4,"1.75");assert(doc.cell(1,4)=="2");
    const auto before=doc.encode();assert(before.find("4000\r\n2\r\n3\r\n2\r\n")==0);
    doc.insert(compat::TransportationDocument::Axis::Destination,1);
    assert(doc.cell(1,2)=="0.00e+000");
    doc.erase(compat::TransportationDocument::Axis::Destination,1);assert(doc.encode()==before);
    auto reopened=compat::TransportationDocument::decode(before);assert(reopened.encode()==before);
    assert(reopened.problem().format.notation==core::Notation::Scientific);
    auto decimal=compat::TransportationDocument::blank(2,2,"Decimal",{core::Notation::Decimal,6,3});
    decimal.setCell(1,1,"1.23456");assert(decimal.cell(1,1)=="1.235");
    const auto stable=decimal.encode();
    try{decimal.setCell(1,1,"not numeric");assert(false);}catch(const std::invalid_argument&){}
    assert(decimal.encode()==stable);
    auto invalid=stable;invalid.replace(6,1,"0");
    try{(void)compat::TransportationDocument::decode(invalid);assert(false);}catch(const std::invalid_argument&){}
}
