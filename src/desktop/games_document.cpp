#include "games_document.hpp"
#include <charconv>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
namespace tora::desktop {
namespace {
double number(std::string_view s) {
 while(!s.empty()&&(s.front()==' '||s.front()=='\t'))s.remove_prefix(1);
 while(!s.empty()&&(s.back()==' '||s.back()=='\t'))s.remove_suffix(1);
 if(!s.empty()&&s.front()=='+')s.remove_prefix(1);
 double v=0;auto p=std::from_chars(s.data(),s.data()+s.size(),v);
 if(s.empty()||p.ec!=std::errc{}||p.ptr!=s.data()+s.size()||!std::isfinite(v))throw std::invalid_argument("Enter a finite payoff number.");
 return v;
}
std::string line(std::istream& in){std::string s;if(!std::getline(in,s))throw std::invalid_argument("Truncated Game file.");if(!s.empty()&&s.back()=='\r')s.pop_back();return s;}
int integer(std::istream& in){const auto s=line(in);int v=0;auto p=std::from_chars(s.data(),s.data()+s.size(),v);if(p.ec!=std::errc{}||p.ptr!=s.data()+s.size())throw std::invalid_argument("Invalid Game integer.");return v;}
std::string quoted(std::istream& in){const auto s=line(in);if(s.size()<2||s.front()!='"'||s.back()!='"')throw std::invalid_argument("Invalid quoted Game field.");std::string v;for(std::size_t i=1;i+1<s.size();++i){if(s[i]=='"'){if(i+2>=s.size()||s[i+1]!='"')throw std::invalid_argument("Invalid embedded Game quote.");++i;}v+=s[i];}return v;}
std::string quote(std::string_view s){std::string v="\"";for(char c:s){v+=c;if(c=='"')v+=c;}return v+'"';}
void validTitle(const std::string& s){if(s.find_first_of("\r\n\0",0,3)!=std::string::npos)throw std::invalid_argument("Game title must occupy one line.");}
}
GamesDocument GamesDocument::blank(int rows,int columns,std::string title,core::NumberFormat format){if(rows<1||rows>100||columns<1||columns>100)throw std::invalid_argument("Games require 1 to 100 strategies per player.");(void)core::NumberFormatter(format);validTitle(title);GamesDocument d;d.payoff_=core::Matrix(rows,columns);d.cells_.assign(static_cast<std::size_t>(rows*columns),"0");d.title_=std::move(title);d.format_=format;return d;}
GamesDocument GamesDocument::decode(std::string_view bytes){std::istringstream in{std::string(bytes)};if(integer(in)!=9000)throw std::invalid_argument("Expected a 9000 Game file.");core::NumberFormat f;f.n=integer(in);f.d=integer(in);int notation=integer(in);if(notation!=1&&notation!=2)throw std::invalid_argument("Invalid Game notation.");f.notation=notation==1?core::Notation::Decimal:core::Notation::Scientific;auto title=quoted(in);int rows=integer(in);int columns=integer(in);auto d=blank(rows,columns,title,f);for(int r=0;r<rows;++r)for(int c=0;c<columns;++c){auto s=quoted(in);d.payoff_(r,c)=number(s);d.cells_[static_cast<std::size_t>(r*columns+c)]=std::move(s);}if(in.peek()!=std::char_traits<char>::eof())throw std::invalid_argument("Unexpected Game fields.");d.original_=std::string(bytes);return d;}
std::string GamesDocument::encode()const{if(!original_.empty())return original_;std::ostringstream out;out<<"9000\r\n"<<format_.n<<"\r\n"<<format_.d<<"\r\n"<<(format_.notation==core::Notation::Decimal?1:2)<<"\r\n"<<quote(title_)<<"\r\n"<<payoff_.rows()<<"\r\n"<<payoff_.columns()<<"\r\n";for(const auto& s:cells_)out<<quote(s)<<"\r\n";return out.str();}
const std::string& GamesDocument::cell(int r,int c)const{(void)payoff_(r,c);return cells_.at(static_cast<std::size_t>(r)*payoff_.columns()+static_cast<std::size_t>(c));}
void GamesDocument::setCell(int r,int c,std::string s){const double v=number(s);auto& old=payoff_(r,c);if(cell(r,c)==s)return;old=v;cells_[static_cast<std::size_t>(r)*payoff_.columns()+static_cast<std::size_t>(c)]=std::move(s);original_.clear();}
void GamesDocument::setTitle(std::string s){validTitle(s);if(title_!=s){title_=std::move(s);original_.clear();}}
void GamesDocument::setFormat(core::NumberFormat f){(void)core::NumberFormatter(f);format_=f;original_.clear();}
std::vector<std::string> GamesDocument::line(bool row,int at)const{std::vector<std::string> values;const int count=static_cast<int>(row?payoff_.columns():payoff_.rows());for(int i=0;i<count;++i)values.push_back(cell(row?at:i,row?i:at));return values;}
void GamesDocument::paste(bool row,int at,const std::vector<std::string>& values){if(values.size()!=(row?payoff_.columns():payoff_.rows()))throw std::invalid_argument("Copy a matching strategy line first.");auto next=*this;for(std::size_t i=0;i<values.size();++i)next.setCell(row?at:static_cast<int>(i),row?static_cast<int>(i):at,values[i]);*this=std::move(next);}
void GamesDocument::insert(bool row,int at){const int oldRows=static_cast<int>(payoff_.rows()),oldCols=static_cast<int>(payoff_.columns()),count=row?oldRows:oldCols;if(at<0||at>count)throw std::invalid_argument("Invalid strategy position.");auto next=blank(oldRows+(row?1:0),oldCols+(row?0:1),title_,format_);for(int r=0;r<oldRows;++r)for(int c=0;c<oldCols;++c)next.setCell(r+(row&&r>=at?1:0),c+(!row&&c>=at?1:0),cell(r,c));*this=std::move(next);}
void GamesDocument::erase(bool row,int at){const int oldRows=static_cast<int>(payoff_.rows()),oldCols=static_cast<int>(payoff_.columns()),count=row?oldRows:oldCols;if(at<0||at>=count||count<=1)throw std::invalid_argument("Keep at least one strategy per player.");auto next=blank(oldRows-(row?1:0),oldCols-(row?0:1),title_,format_);for(int r=0;r<oldRows;++r)for(int c=0;c<oldCols;++c){if((row&&r==at)||(!row&&c==at))continue;next.setCell(r-(row&&r>at?1:0),c-(!row&&c>at?1:0),cell(r,c));}*this=std::move(next);}

}
