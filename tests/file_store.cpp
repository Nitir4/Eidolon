#include <tora/compat/lp_file.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
using namespace tora;
void require(bool b,const std::string& s){if(!b)throw std::runtime_error(s);}
std::string bytes(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);require(f.good(),"Read saved bytes");return {std::istreambuf_iterator<char>(f),{}};}
template<class F>void error(F action,compat::FileErrorCode code){try{action();}catch(const compat::FileError& e){require(e.code()==code,"Wrong file error");return;}throw std::runtime_error("File operation should fail");}
}
int main(int argc,char** argv){try{
    require(argc==2,"Pass isolated test directory");const auto dir=std::filesystem::path(argv[1]);std::filesystem::create_directories(dir);
    auto doc=compat::ToraLpCodec::create(lp::LpProblem::blank(1,1));doc.problem.title="File store probe";require(doc.dirty(),"Unsaved document dirty");
    error([&]{compat::LpFileStore::save(doc);},compat::FileErrorCode::MissingFilename);
    error([&]{(void)compat::LpFileStore::open(dir/"missing.txt");},compat::FileErrorCode::Io);
#ifdef __cpp_char8_t
    const auto path=dir/std::filesystem::path(u8"caf\u00e9 without suffix");
#else
    const auto path=dir/std::filesystem::u8path(u8"caf\u00e9 without suffix");
#endif
    compat::LpFileStore::saveAs(doc,path);
    require(doc.sourcePath()==path && !doc.dirty() && !doc.readOnly() && std::filesystem::is_regular_file(path),"Exact filename, Unicode OS path and save checkpoint");
    const auto before=bytes(path);doc.problem.objective[0]=1.2345678901234567;require(doc.dirty(),"Edit dirty");compat::LpFileStore::save(doc);
    auto reopened=compat::LpFileStore::open(path);require(!reopened.dirty() && reopened.problem.objective[0]==doc.problem.objective[0] && bytes(path)!=before,"Save replaces existing file without rounding");
    auto readOnly=compat::LpFileStore::open(path,true);readOnly.problem.title="Unsaved change";const auto original=bytes(path);
    error([&]{compat::LpFileStore::save(readOnly);},compat::FileErrorCode::ReadOnly);require(bytes(path)==original && readOnly.dirty(),"Readonly denial preserves bytes and dirty state");
    const auto copy=dir/"readonly-copy.txt";compat::LpFileStore::saveAs(readOnly,copy);
    require(bytes(path)==original && !readOnly.dirty() && !readOnly.readOnly() && readOnly.sourcePath()==copy && compat::LpFileStore::open(copy).problem.title=="Unsaved change","Readonly Save As creates an editable copy without changing its source");
    auto failed=doc;failed.problem.title="Failure must not checkpoint";const auto target=dir/"existing-directory";std::filesystem::create_directories(target);
    error([&]{compat::LpFileStore::saveAs(failed,target);},compat::FileErrorCode::Io);
    require(failed.sourcePath()==path && failed.dirty() && bytes(path)==original && std::filesystem::is_directory(target),"Failed replacement leaves old document/file intact");
    for(const auto& entry:std::filesystem::directory_iterator(dir))require(entry.path().filename().string().find(".tora-tmp-")==std::string::npos,"Failed save cleans its temporary file");
    doc.problem.objective.clear();error([&]{compat::LpFileStore::save(doc);},compat::FileErrorCode::Model);require(bytes(path)==original,"Invalid serialization never truncates destination");
    std::cout<<"Binary open/save, exact filenames, readonly denial, precision and atomic rollback passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
