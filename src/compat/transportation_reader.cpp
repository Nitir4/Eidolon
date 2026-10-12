#include <tora/compat/transportation_reader.hpp>
#include <charconv>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace tora::compat {
namespace {
std::size_t size(const std::string& value) {
    std::size_t result=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
    if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size() || !result || result>100)
        throw std::invalid_argument("Invalid Transportation problem size (native limit: 100).");
    return result;
}
std::string unquote(const std::string& value) {
    if(value.size()<2 || value.front()!='"' || value.back()!='"')
        throw std::invalid_argument("Expected a quoted Transportation text record.");
    std::string text;
    for(std::size_t i=1;i+1<value.size();++i) {
        if(value[i]=='"') {
            if(i+2>=value.size() || value[i+1]!='"')throw std::invalid_argument("Invalid embedded quote.");
            ++i;
        }
        text+=value[i];
    }
    for (unsigned char c:text) if (c>127)
        throw std::invalid_argument("This Transportation preview supports the captured ASCII text profile; legacy code pages remain unverified.");
    return text;
}
double number(const std::string& text) {
    std::string_view value=text;
    while(!value.empty() && (value.front()==' ' || value.front()=='\t'))value.remove_prefix(1);
    while(!value.empty() && (value.back()==' ' || value.back()=='\t'))value.remove_suffix(1);
    if(!value.empty() && value.front()=='+')value.remove_prefix(1);
    double result=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
    if(value.empty() || parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size() || !std::isfinite(result))
        throw std::invalid_argument("Expected a finite Transportation numeric cell.");
    return result;
}
}
transportation::Problem readTransportation(std::string_view bytes) {
    if(bytes.size()>4*1024*1024)throw std::invalid_argument("Transportation file exceeds the native 4 MiB limit.");
    std::vector<std::string> records;
    while(!bytes.empty()) {
        const auto end=bytes.find("\r\n");
        if(end==std::string_view::npos)throw std::invalid_argument("Transportation files require CRLF after each record.");
        const auto line=bytes.substr(0,end);
        if(line.find_first_of("\r\n")!=std::string_view::npos || line.find('\0')!=std::string_view::npos)
            throw std::invalid_argument("Invalid Transportation record framing.");
        records.emplace_back(line);bytes.remove_prefix(end+2);
    }
    if(records.size()<7 || records[0]!="4000")throw std::invalid_argument("Select a TORA Transportation file (4000).");
    if(records[1]!="5" || records[2]!="2" || records[3]!="1")
        throw std::invalid_argument("This Transportation preview opens the captured default format N=5, D=2, decimal notation.");
    const auto m=size(records[5]),n=size(records[6]);
    if(records.size()!=7+(m+2)*(n+2))throw std::invalid_argument("Transportation file has an incorrect grid record count.");
    auto p=transportation::Problem::blank(m,n);p.title=unquote(records[4]);
    const auto cell=[&](std::size_t row,std::size_t col){return unquote(records.at(7+row*(n+2)+col));};
    for(std::size_t j=0;j<n;++j) {
        p.destinationNames[j]=cell(0,j+1);p.demand[j]=number(cell(m+1,j+1));
    }
    for(std::size_t i=0;i<m;++i) {
        p.sourceNames[i]=cell(i+1,0);p.supply[i]=number(cell(i+1,n+1));
        for(std::size_t j=0;j<n;++j)p.costs[i][j]=number(cell(i+1,j+1));
    }
    transportation::validate(p);return p;
}
}
