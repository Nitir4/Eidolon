#include "equations_document.hpp"
#include <charconv>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
namespace tora::desktop {
namespace {
double number(const std::string& s) {double v=0;auto p=std::from_chars(s.data(),s.data()+s.size(),v);if(s.empty()||p.ec!=std::errc{}||p.ptr!=s.data()+s.size()||!std::isfinite(v))throw std::invalid_argument("Enter a finite augmented number.");return v;}
std::string line(std::istream& in){std::string s;if(!std::getline(in,s))throw std::invalid_argument("Truncated Equations file.");if(!s.empty()&&s.back()=='\r')s.pop_back();return s;}
int integer(std::istream& in){const auto s=line(in);int v=0;auto p=std::from_chars(s.data(),s.data()+s.size(),v);if(p.ec!=std::errc{}||p.ptr!=s.data()+s.size())throw std::invalid_argument("Invalid Equations integer.");return v;}
std::string quoted(std::istream& in){const auto s=line(in);if(s.size()<2||s.front()!='"'||s.back()!='"')throw std::invalid_argument("Invalid quoted Equations field.");std::string v;for(std::size_t i=1;i+1<s.size();++i){if(s[i]=='"'){if(i+2>=s.size()||s[i+1]!='"')throw std::invalid_argument("Invalid embedded Equations quote.");++i;}v+=s[i];}return v;}
std::string quote(std::string_view s){std::string v="\"";for(char c:s){v+=c;if(c=='"')v+=c;}return v+'"';}
void validTitle(const std::string& s){if(s.find_first_of("\r\n\0",0,3)!=std::string::npos)throw std::invalid_argument("Equations title must occupy one line.");}
}
EquationsDocument EquationsDocument::blank(int variables,std::string title,core::NumberFormat format,bool matrixOnly){if(variables<1||variables>100)throw std::invalid_argument("Equations require 1 to 100 variables.");int rows=variables,columns=variables+1;(void)core::NumberFormatter(format);validTitle(title);EquationsDocument d;d.matrixOnly_=matrixOnly;d.augmented_=core::Matrix(rows,columns);d.cells_.assign(static_cast<std::size_t>(rows*columns),"0");d.title_=std::move(title);d.format_=format;return d;}
EquationsDocument EquationsDocument::decode(std::string_view bytes){std::istringstream in{std::string(bytes)};const int marker=integer(in);if(marker!=1000&&marker!=2000)throw std::invalid_argument("Expected a 1000 Matrix or 2000 Equations file.");core::NumberFormat f;f.n=integer(in);f.d=integer(in);int notation=integer(in);if(notation!=1&&notation!=2)throw std::invalid_argument("Invalid Equations notation.");f.notation=notation==1?core::Notation::Decimal:core::Notation::Scientific;auto title=quoted(in);int rows=integer(in);auto d=blank(rows,title,f,marker==1000);int columns=marker==1000?rows:rows+1;for(int r=0;r<rows;++r)for(int c=0;c<columns;++c){auto s=quoted(in);d.augmented_(r,c)=number(s);d.cells_[static_cast<std::size_t>(r*(rows+1)+c)]=std::move(s);}if(in.peek()!=std::char_traits<char>::eof())throw std::invalid_argument("Unexpected Equations fields.");d.original_=std::string(bytes);return d;}
std::string EquationsDocument::encode()const{if(!original_.empty())return original_;std::ostringstream out;out<<(matrixOnly_?1000:2000)<<"\r\n"<<format_.n<<"\r\n"<<format_.d<<"\r\n"<<(format_.notation==core::Notation::Decimal?1:2)<<"\r\n"<<quote(title_)<<"\r\n"<<augmented_.rows()<<"\r\n";for(std::size_t r=0;r<augmented_.rows();++r)for(std::size_t c=0;c<(matrixOnly_?augmented_.rows():augmented_.columns());++c)out<<quote(cell(static_cast<int>(r),static_cast<int>(c)))<<"\r\n";return out.str();}
const std::string& EquationsDocument::cell(int r,int c)const{(void)augmented_(r,c);return cells_.at(static_cast<std::size_t>(r)*augmented_.columns()+static_cast<std::size_t>(c));}
void EquationsDocument::setCell(int r,int c,std::string s){if(matrixOnly_&&c==static_cast<int>(augmented_.rows()))throw std::invalid_argument("A matrix-only file has no RHS column.");const double v=number(s);auto& old=augmented_(r,c);if(cell(r,c)==s)return;old=v;cells_[static_cast<std::size_t>(r)*augmented_.columns()+static_cast<std::size_t>(c)]=std::move(s);original_.clear();}
void EquationsDocument::setTitle(std::string s){validTitle(s);if(title_!=s){title_=std::move(s);original_.clear();}}
void EquationsDocument::setFormat(core::NumberFormat f){(void)core::NumberFormatter(f);format_=f;original_.clear();}
core::Matrix EquationsDocument::coefficients() const{const auto n=augmented_.rows();core::Matrix a(n,n);for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<n;++j)a(i,j)=augmented_(i,j);return a;}
std::vector<double> EquationsDocument::rhs() const{std::vector<double>b(augmented_.rows());for(std::size_t i=0;i<b.size();++i)b[i]=augmented_(i,b.size());return b;}
}
