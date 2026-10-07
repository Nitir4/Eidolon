#include <tora/compat/lp_file.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
using namespace tora;
void require(bool b,const std::string& s){if(!b)throw std::runtime_error(s);}
std::vector<std::string> records(const std::string& s){std::vector<std::string> r;std::size_t at=0;for(auto end=s.find("\r\n");end!=std::string::npos;end=s.find("\r\n",at)){r.push_back(s.substr(at,end-at));at=end+2;}return r;}
std::string join(const std::vector<std::string>& r){std::string s;for(const auto& line:r)s+=line+"\r\n";return s;}
void rejected(const std::string& s,compat::FileErrorCode code,std::optional<std::size_t> record={}){
    try{(void)compat::ToraLpCodec::decode(s,compat::QuotePolicy::Reject);}catch(const compat::FileError& e){require(e.code()==code,"Wrong codec diagnostic: expected "+std::to_string(static_cast<int>(code))+", got "+std::to_string(static_cast<int>(e.code()))+": "+e.what());if(record)require(e.record()==record,"Wrong one-based record location");return;}throw std::runtime_error("Malformed file accepted");
}
}
int main(){try{
    auto doc=compat::ToraLpCodec::create(lp::LpProblem::blank(2,3));const auto valid=compat::ToraLpCodec::encode(doc);auto r=records(valid);
    auto changed=r;changed[0]="9999";
    try{(void)compat::ToraLpCodec::decode(join(changed));throw std::runtime_error("Wrong marker accepted");}catch(const compat::FileError& e){require(e.code()==compat::FileErrorCode::Marker && e.origin()==lp::DiagnosticOrigin::ObservedTora && e.title()=="Open FileSave As Warning" && std::string(e.what())=="Target file does not match format of elected TORA module. Retry?","Exact captured module warning");}
    for(const auto& bad:{"0","101","-1","1.5","bogus","184467440737095516160"}){changed=r;changed[5]=bad;rejected(join(changed),compat::FileErrorCode::Header,6);}
    changed=r;changed[6]="0";rejected(join(changed),compat::FileErrorCode::Header,7);
    changed=r;changed[3]="3";rejected(join(changed),compat::FileErrorCode::UnsupportedFormat,4);
    changed=r;changed[1]="0";rejected(join(changed),compat::FileErrorCode::UnsupportedFormat);
    changed=r;changed[4]="unquoted";rejected(join(changed),compat::FileErrorCode::QuotedField,5);
    changed=r;changed[4]="\"F4, \"\"quoted\"\" cafe\"";
    rejected(join(changed),compat::FileErrorCode::QuotedField,5);
    auto quoted=compat::ToraLpCodec::decode(join(changed));
    require(quoted.problem.title=="F4, \"quoted\" cafe" && compat::ToraLpCodec::encode(quoted)==join(changed),"Decode the doubled quotes observed in TORA's saved record");
    auto fromModel=compat::ToraLpCodec::create(quoted.problem,quoted.inputFormat,compat::QuotePolicy::Recover);
    require(records(compat::ToraLpCodec::encode(fromModel))[4]==changed[4],"Writer follows observed doubled-quote storage");
    changed=r;changed[4]="\"dangling\"quote\"";rejected(join(changed),compat::FileErrorCode::QuotedField,5);
    changed=r;changed[7]="\"Maximum\"";rejected(join(changed),compat::FileErrorCode::Header,8);
    changed=r;changed.resize(15);rejected(join(changed),compat::FileErrorCode::RecordCount);
    rejected(valid.substr(0,valid.size()-2),compat::FileErrorCode::Framing);
    auto lf=valid;for(auto pos=lf.find('\r');pos!=std::string::npos;pos=lf.find('\r'))lf.erase(pos,1);rejected(lf,compat::FileErrorCode::Framing);
    for(const auto& bad:{"bogus","NaN","inf","1e999","1e-9999","3junk",""}){changed=r;changed[12]='"'+std::string(bad)+'"';rejected(join(changed),compat::FileErrorCode::Number,13);}
    changed=r;changed[18]="\"?\"";rejected(join(changed),compat::FileErrorCode::Relation,19);
    changed=r;changed[36]="\"maybe\"";rejected(join(changed),compat::FileErrorCode::Flag,37);
    rejected(std::string(compat::ToraLpCodec::MaxFileBytes+1,'x'),compat::FileErrorCode::TooLarge);
    changed=r;changed[10]="\"reserved\"";changed.push_back("\"extra\"");
    auto preserved=compat::ToraLpCodec::decode(join(changed));require(compat::ToraLpCodec::encode(preserved)==join(changed),"Reserved cells and extra records retained");
    preserved.problem.objective[0]=2.5;const auto edited=records(compat::ToraLpCodec::encode(preserved));require(edited[10]==changed[10] && edited.back()==changed.back(),"Editing a known field keeps unrelated reserved/trailing records");
    auto signedZero=r;signedZero[12]="\"-0.000\"";auto zero=compat::ToraLpCodec::decode(join(signedZero));require(std::signbit(zero.problem.objective[0]) && compat::ToraLpCodec::encode(zero)==join(signedZero),"Signed zero spelling survives");zero.problem.objective[0]=0;require(!std::signbit(compat::ToraLpCodec::decode(compat::ToraLpCodec::encode(zero)).problem.objective[0]),"Changing signed zero regenerates cell");
    doc.problem.variables[0].lowerBound=3;doc.problem.variables[0].upperBound=1;const auto invalidBounds=compat::ToraLpCodec::decode(compat::ToraLpCodec::encode(doc));require(!lp::LpValidator::validate(invalidBounds.problem).valid(),"Saved input may need model validation; codec does not solve/repair reversed bounds");
    doc.problem.objective[0]=std::numeric_limits<double>::infinity();bool stopped=false;try{(void)compat::ToraLpCodec::encode(doc);}catch(const compat::FileError& e){stopped=e.code()==compat::FileErrorCode::Model;}require(stopped,"Reject nonfinite matrix writes");
    std::cout<<"Observed marker warning, bounded strict parsing, nonfinite/shape safety and lexical/reserved preservation passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
