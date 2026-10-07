#include <tora/compat/lp_file.hpp>
#include <cassert>
#include <filesystem>
#include <iostream>

using namespace tora::compat;
int main(int argc,char** argv) {
    assert(argc==2);
    auto document=ToraLpCodec::create(tora::lp::LpProblem::blank(2,1));
    auto cells=ToraLpCodec::gridText(document);
    assert(cells.size()==24 && cells[0].empty() && cells[4]=="0.00");
    cells[0]="decision";cells[4]="1.2300";
    ToraLpCodec::setGridText(document,cells);
    assert(document.problem.variables[0].name=="decision");
    assert(document.problem.objective[0]==1.23);
    assert(ToraLpCodec::gridText(document)[4]=="1.2300");
    const auto bytes=ToraLpCodec::encode(document);
    assert(bytes.find("\"1.2300\"\r\n")!=std::string::npos);
    auto invalid=cells;invalid[4]="NaN";
    try {ToraLpCodec::setGridText(document,invalid);assert(false);}
    catch (const FileError& e) {assert(e.code()==FileErrorCode::Number);}
    assert(ToraLpCodec::encode(document)==bytes);
    invalid.pop_back();
    try {ToraLpCodec::setGridText(document,invalid);assert(false);}
    catch (const FileError& e) {assert(e.code()==FileErrorCode::RecordCount);}
    assert(ToraLpCodec::encode(document)==bytes);
    const auto path=std::filesystem::path(argv[1])/"lexical-grid.txt";
    std::filesystem::create_directories(path.parent_path());
    LpFileStore::saveAs(document,path);
    auto copy=LpFileStore::open(path,true);assert(!copy.dirty());
    cells[4]="2.50";ToraLpCodec::setGridText(copy,cells);
    assert(copy.sourcePath()==path && copy.readOnly() && copy.dirty());
    assert(copy.problem.objective[0]==2.5 && ToraLpCodec::gridText(copy)[4]=="2.50");
    assert(LpFileStore::open(path).problem.objective[0]==1.23);
    std::cout<<"Lexical grid edits preserve complete records, source/read-only checkpoints and failure rollback\n";
}
