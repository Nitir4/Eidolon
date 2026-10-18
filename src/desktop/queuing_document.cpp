#include "queuing_document.hpp"
#include <charconv>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
namespace tora::desktop {
namespace {
std::string line(std::istream& in){std::string s;if(!std::getline(in,s))throw std::invalid_argument("Truncated Queuing file.");if(!s.empty()&&s.back()=='\r')s.pop_back();return s;}
int integer(const std::string& s){int v=0;auto p=std::from_chars(s.data(),s.data()+s.size(),v);if(p.ec!=std::errc{}||p.ptr!=s.data()+s.size())throw std::invalid_argument("Expected a whole number.");return v;}
std::string quoted(std::istream& in){const auto s=line(in);if(s.size()<2||s.front()!='"'||s.back()!='"')throw std::invalid_argument("Invalid quoted Queuing field.");std::string v;for(std::size_t i=1;i+1<s.size();++i){if(s[i]=='"'){if(i+2>=s.size()||s[i+1]!='"')throw std::invalid_argument("Invalid embedded Queuing quote.");++i;}v+=s[i];}return v;}
std::string quote(std::string_view s){std::string v="\"";for(char c:s){v+=c;if(c=='"')v+=c;}return v+'"';}
double rate(const std::string& s){double v=0;auto p=std::from_chars(s.data(),s.data()+s.size(),v);if(s.empty()||p.ec!=std::errc{}||p.ptr!=s.data()+s.size()||!std::isfinite(v)||v<0)throw std::invalid_argument("Enter a finite nonnegative rate.");return v;}
std::optional<std::size_t> limit(const std::string& s){if(s=="infinity")return {};int n=integer(s);if(n<1||n>100000)throw std::invalid_argument("Limits must be 1..100000 or infinity.");return static_cast<std::size_t>(n);}
void validTitle(const std::string& s){if(s.find_first_of("\r\n\0",0,3)!=std::string::npos)throw std::invalid_argument("Queuing title must occupy one line.");}
}
QueuingDocument QueuingDocument::blank(std::string title,core::NumberFormat f){validTitle(title);(void)core::NumberFormatter(f);QueuingDocument d;d.title_=std::move(title);d.format_=f;d.addScenario();return d;}
QueuingDocument QueuingDocument::decode(std::string_view bytes){std::istringstream in{std::string(bytes)};if(integer(line(in))!=6000)throw std::invalid_argument("Expected a 6000 Queuing file.");core::NumberFormat f;f.n=integer(line(in));f.d=integer(line(in));const int notation=integer(line(in));if(notation!=1&&notation!=2)throw std::invalid_argument("Invalid Queuing notation.");f.notation=notation==1?core::Notation::Decimal:core::Notation::Scientific;auto title=quoted(in);const int count=integer(line(in));if(count<1||count>100)throw std::invalid_argument("Queuing requires 1..100 scenarios.");auto d=blank(title,f);d.rows_.clear();for(int row=0;row<count;++row){d.addScenario();for(int col=0;col<5;++col)d.setCell(row,col,quoted(in));}if(in.peek()!=std::char_traits<char>::eof())throw std::invalid_argument("Unexpected Queuing fields.");d.original_=std::string(bytes);return d;}
std::string QueuingDocument::encode()const{if(!original_.empty())return original_;std::ostringstream out;out<<"6000\r\n"<<format_.n<<"\r\n"<<format_.d<<"\r\n"<<(format_.notation==core::Notation::Decimal?1:2)<<"\r\n"<<quote(title_)<<"\r\n"<<rows_.size()<<"\r\n";for(const auto& row:rows_)for(const auto& value:row)out<<quote(value)<<"\r\n";return out.str();}
const std::string& QueuingDocument::cell(std::size_t row,std::size_t col)const{return rows_.at(row).at(col);}
void QueuingDocument::setCell(std::size_t row,std::size_t col,std::string value){(void)cell(row,col);if(col<2)(void)rate(value);else if(col==2){auto c=integer(value);if(c<1||c>10000)throw std::invalid_argument("Servers must be 1..10000.");}else (void)limit(value);if(rows_[row][col]!=value){rows_[row][col]=std::move(value);original_.clear();}}
void QueuingDocument::setTitle(std::string title){validTitle(title);if(title_!=title){title_=std::move(title);original_.clear();}}
void QueuingDocument::setFormat(core::NumberFormat f){(void)core::NumberFormatter(f);format_=f;original_.clear();}
QueuingDocument::Scenario QueuingDocument::scenario(std::size_t row)const{return rows_.at(row);}
void QueuingDocument::pasteScenario(std::size_t row,const Scenario& values){auto next=*this;for(std::size_t col=0;col<values.size();++col)next.setCell(row,col,values[col]);*this=std::move(next);}
void QueuingDocument::insertScenario(std::size_t position){if(rows_.size()>=100)throw std::invalid_argument("At most 100 scenarios are supported.");if(position>rows_.size())throw std::invalid_argument("Select a valid scenario heading.");rows_.insert(rows_.begin()+static_cast<std::ptrdiff_t>(position),{"0","0","1","infinity","infinity"});original_.clear();}
void QueuingDocument::addScenario(){insertScenario(rows_.size());}
void QueuingDocument::deleteScenario(std::size_t row){if(rows_.size()<=1||row>=rows_.size())throw std::invalid_argument("Keep at least one valid scenario.");rows_.erase(rows_.begin()+static_cast<std::ptrdiff_t>(row));original_.clear();}
queuing::Model QueuingDocument::model(std::size_t row)const{queuing::Model m;m.arrivalRate=rate(cell(row,0));m.serviceRate=rate(cell(row,1));m.servers=static_cast<std::size_t>(integer(cell(row,2)));m.systemLimit=limit(cell(row,3));m.sourceLimit=limit(cell(row,4));if(m.sourceLimit&&(!m.systemLimit||*m.systemLimit>*m.sourceLimit))throw std::invalid_argument("System Limit exceeds Source Limit.");return m;}
}
