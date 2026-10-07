#include <tora/compat/lp_file.hpp>
#include <filesystem>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <sstream>
#include <stdexcept>

namespace {
using namespace tora;
void require(bool b,const std::string& s){if(!b)throw std::runtime_error(s);}
std::string bytes(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);require(f.good(),"Read fixture "+p.string());return {std::istreambuf_iterator<char>(f),{}};}
struct Input {std::string title,sense;std::size_t n=0,m=0;std::vector<std::vector<std::string>> grid;};
std::map<std::string,Input> models(const std::filesystem::path& path) {
    std::ifstream f(path);f.imbue(std::locale::classic());require(f.good(),"Original model grammar");
    std::map<std::string,Input> result;std::string tag;
    while(f>>tag){
        if(tag.front()=='#'){std::getline(f,tag);continue;}
        require(tag=="case","Case tag");std::string id;Input x;
        f>>std::quoted(id)>>std::quoted(x.title)>>x.sense>>x.n>>x.m;
        x.grid.assign(x.m+5,std::vector<std::string>(x.n+2));
        for(std::size_t j=0;j<x.n;++j){
            f>>tag;require(tag=="variable","Variable tag");f>>std::quoted(x.grid[0][j])>>x.grid[x.m+2][j]>>x.grid[x.m+3][j]>>x.grid[x.m+4][j];
            if(x.grid[x.m+3][j]=="inf")x.grid[x.m+3][j]="infinity";
        }
        f>>tag;require(tag=="objective","Objective tag");for(std::size_t j=0;j<x.n;++j)f>>x.grid[1][j];
        for(std::size_t i=0;i<x.m;++i){f>>tag;require(tag=="constraint","Constraint tag");f>>tag>>x.grid[i+2][x.n+1];x.grid[i+2][x.n]="        "+tag;for(std::size_t j=0;j<x.n;++j)f>>x.grid[i+2][j];}
        f>>tag;require(tag=="end" && !f.fail(),"Complete model grammar");result.emplace(id,std::move(x));
    }
    return result;
}
std::string expectedBytes(const Input& x,int n,int d,int notation){
    // Independent specification assembly; hashes are from actual TORA saves.
    const auto record=[](const std::string& value){std::string text="\"";for(const char c:value){if(c=='"')text+='"';text+=c;}return text+"\"\r\n";};
    std::string out="3000\r\n"+std::to_string(n)+"\r\n"+std::to_string(d)+"\r\n"+std::to_string(notation)+"\r\n"+record(x.title)+std::to_string(x.n)+"\r\n"+std::to_string(x.m)+"\r\n"+record(x.sense=="max"?"Maximize":"Minimize");
    for(const auto& row:x.grid)for(const auto& value:row)out+=record(value);
    return out;
}
double number(const std::string& s){if(s=="infinity")return std::numeric_limits<double>::infinity();std::istringstream f(s);f.imbue(std::locale::classic());double v=0;f>>v;require(!f.fail(),"Fixture numeric token");return v;}
void compare(const compat::LpDocument& doc,const Input& x){
    const auto& p=doc.problem;require(p.title==x.title && p.variables.size()==x.n && p.constraints.size()==x.m,"Title/count fields");
    require(p.sense==(x.sense=="max"?lp::ObjectiveSense::Maximize:lp::ObjectiveSense::Minimize),"Direction field");
    for(std::size_t j=0;j<x.n;++j){const auto& v=p.variables[j];require(v.name==x.grid[0][j] && p.objective[j]==number(x.grid[1][j]) && v.lowerBound==number(x.grid[x.m+2][j]) && v.upperBound==number(x.grid[x.m+3][j]) && v.unrestricted==(x.grid[x.m+4][j]=="y"),"Variable/coefficient/bound/sign fields");}
    for(std::size_t i=0;i<x.m;++i){const auto& row=p.constraints[i];const auto& rel=x.grid[i+2][x.n];const auto expected=rel=="        <="?lp::Relation::LessEqual:(rel=="        >="?lp::Relation::GreaterEqual:lp::Relation::Equal);require(row.relation==expected && row.rhs==number(x.grid[i+2][x.n+1]),"Relation/RHS fields");for(std::size_t j=0;j<x.n;++j)require(row.coefficients[j]==number(x.grid[i+2][j]),"Ordered matrix cells");}
}
} // namespace
int main(int argc,char** argv){try{
    require(argc==4 || argc==5,"Model grammar, metadata, output dir, optional private reference root");
    auto inputs=models(argv[1]);
    for(const auto& id:{"new-decimal","new-scientific"}){
        Input x;x.title=id;x.sense="max";x.n=2;x.m=1;x.grid.assign(6,std::vector<std::string>(4));
        const std::string zero=std::string(id)=="new-scientific"?"0.00000e+00":"0.00";
        for(std::size_t j=0;j<2;++j){x.grid[1][j]=zero;x.grid[2][j]=zero;x.grid[3][j]=zero;x.grid[4][j]="infinity";x.grid[5][j]="n";}x.grid[2][2]="        <=";x.grid[2][3]=zero;inputs.emplace(id,std::move(x));
    }
    Input text;text.title="F4, \"quoted\" cafe";text.sense="max";text.n=2;text.m=3;text.grid.assign(8,std::vector<std::string>(4));
    for(std::size_t j=0;j<2;++j){text.grid[1][j]="0.00";text.grid[5][j]="0.00";text.grid[6][j]="infinity";text.grid[7][j]="n";}
    for(std::size_t i=2;i<5;++i){text.grid[i][0]="0.00";text.grid[i][1]="0.00";text.grid[i][2]="        <=";text.grid[i][3]="0.00";}
    inputs.emplace("quoted-title-recovery",text);
    auto comma=text;comma.title="F4, comma";inputs.emplace("comma-title",comma);
    auto quoted=text;quoted.title="F4 \"quoted\"";inputs.emplace("quoted-title-only",quoted);
    auto unicode=text;unicode.title="F4 cafe \xE9";inputs.emplace("legacy-text",unicode);
    auto precision=inputs.at("probe-1var-1con");precision.title="Clone precision bridge";precision.grid[1][0]="1.2345678901234567";inputs.emplace("clone-precision",precision);
    auto scientific=inputs.at("new-scientific");scientific.title="Clone fresh scientific";inputs.emplace("clone-scientific",scientific);
    std::ifstream metadata(argv[2]);require(metadata.good(),"File hash metadata");std::string id;std::size_t count=0;const std::filesystem::path output=argv[3];std::filesystem::create_directories(output);
    while(metadata>>id){if(id.front()=='#'){std::getline(metadata,id);continue;}
        int n=0,d=0,mode=0;std::size_t length=0;std::string hash,source;metadata>>n>>d>>mode>>length>>hash>>source;require(!metadata.fail(),"Metadata row");
        const auto& input=inputs.at(id);const auto expected=expectedBytes(input,n,d,mode);require(expected.size()==length,id+": oracle byte count");
        auto document=compat::ToraLpCodec::decode(expected);compare(document,input);
        require(!document.dirty() && document.sourcePath().empty() && !document.readOnly(),"Decoded document baseline");
        require(document.inputFormat.n==n && document.inputFormat.d==d && document.inputFormat.notation==(mode==1?core::Notation::Decimal:core::Notation::Scientific),"Input-format fields");
        require(compat::ToraLpCodec::encode(document)==expected,id+": unchanged byte round trip");
        if(argc==5){const auto actual=bytes(std::filesystem::path(argv[4])/source);require(actual==expected,id+": independent transcription must equal actual TORA bytes");require(compat::ToraLpCodec::encode(compat::ToraLpCodec::decode(actual))==actual,id+": actual TORA file round trip");}
        compat::LpFileStore::saveAs(document,output/(id+".txt"));require(!document.dirty(),"Save checkpoint");require(bytes(output/(id+".txt"))==expected,"Binary filesystem framing");
        if(id=="new-decimal" || id=="new-scientific"){auto fresh=compat::ToraLpCodec::create(lp::LpProblem::blank(2,1),document.inputFormat);fresh.problem.title=id;require(compat::ToraLpCodec::encode(fresh)==expected,"Observed fresh-model default lexemes");}
        const double precise=1.2345678901234567;document.problem.objective[0]=document.problem.objective[0]==precise?std::nextafter(precise,2.0):precise;require(document.dirty(),"Model edit marks document dirty");const auto edited=compat::ToraLpCodec::decode(compat::ToraLpCodec::encode(document));require(edited.problem.objective[0]==document.problem.objective[0],"Edited values retain binary64 precision");
        ++count;
    }
    require(count==27,"All 27 actual saved-file expectations required");
    std::cout<<count<<" TORA-saved file expectations: ordered fields, settings, byte round trips and changed-value precision passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
