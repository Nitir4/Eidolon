#include <tora/compat/lp_file.hpp>
#include <charconv>
#include <cmath>
#include <limits>
#include <utility>

namespace tora::compat {
namespace {
bool same(double a,double b) { return a == b && (a != 0 || std::signbit(a) == std::signbit(b)); }
bool same(const core::NumberFormat& a,const core::NumberFormat& b) {
    return a.notation == b.notation && a.n == b.n && a.d == b.d;
}
bool same(const lp::LpProblem& a,const lp::LpProblem& b) {
    if (a.title != b.title || a.sense != b.sense || a.variables.size() != b.variables.size() ||
        a.objective.size() != b.objective.size() || a.constraints.size() != b.constraints.size()) return false;
    for (std::size_t j=0;j<a.objective.size();++j) if (!same(a.objective[j],b.objective[j])) return false;
    for (std::size_t j=0;j<a.variables.size();++j) {
        const auto& x=a.variables[j];const auto& y=b.variables[j];
        if (x.name != y.name || !same(x.lowerBound,y.lowerBound) || !same(x.upperBound,y.upperBound) || x.unrestricted != y.unrestricted) return false;
    }
    for (std::size_t i=0;i<a.constraints.size();++i) {
        const auto& x=a.constraints[i];const auto& y=b.constraints[i];
        if (x.relation != y.relation || !same(x.rhs,y.rhs) || x.coefficients.size() != y.coefficients.size()) return false;
        for (std::size_t j=0;j<x.coefficients.size();++j) if (!same(x.coefficients[j],y.coefficients[j])) return false;
    }
    return true;
}
void formatCheck(const core::NumberFormat& format) {
    try { (void)core::NumberFormatter(format); }
    catch (const std::invalid_argument&) { throw FileError(FileErrorCode::UnsupportedFormat,"Unsupported input number format"); }
}
void modelCheck(const lp::LpProblem& problem) {
    for (const auto& issue:lp::LpValidator::validate(problem).issues) {
        // TORA can save reversed bounds; input validity is checked separately.
        if (issue.code != lp::IssueCode::InconsistentBounds)
            throw FileError(FileErrorCode::Model,issue.message);
    }
}
std::size_t integer(const std::string& s,std::size_t record,std::size_t limit) {
    std::size_t value=0;
    const auto parsed=std::from_chars(s.data(),s.data()+s.size(),value);
    if (s.empty() || parsed.ec != std::errc{} || parsed.ptr != s.data()+s.size() || value > limit)
        throw FileError(FileErrorCode::Header,"Invalid integer setting or problem size",record);
    return value;
}
std::string unquote(const std::string& s,std::size_t record,QuotePolicy policy) {
    if (s.size()<2 || s.front()!='"' || s.back()!='"')
        throw FileError(FileErrorCode::QuotedField,"Expected a quoted text record",record);
    std::string value;
    for (std::size_t i=1;i+1<s.size();++i) {
        if (s[i]=='"') {
            if (i+2>=s.size() || s[i+1]!='"') throw FileError(FileErrorCode::QuotedField,"Invalid embedded quote",record);
            if (policy==QuotePolicy::Reject) throw FileError(FileErrorCode::QuotedField,"Embedded quotes require explicit legacy-file recovery",record);
            ++i;
        }
        value+=s[i];
    }
    return value;
}
std::string quote(const std::string& s,QuotePolicy policy) {
    if (s.find_first_of("\r\n") != std::string::npos || s.find('\0') != std::string::npos)
        throw FileError(FileErrorCode::QuotedField,"Text fields cannot contain record separators or NUL");
    std::string value="\"";
    for (const char ch:s) {
        if (ch=='"') {
            if (policy==QuotePolicy::Reject) throw FileError(FileErrorCode::QuotedField,"Embedded quotes are not supported by the legacy-compatible policy");
            value+='"';
        }
        value+=ch;
    }
    return value+'"';
}
std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front()==' ' || s.front()=='\t')) s.remove_prefix(1);
    while (!s.empty() && (s.back()==' ' || s.back()=='\t')) s.remove_suffix(1);
    return s;
}
double number(const std::string& s,std::size_t record,bool upper=false) {
    auto token=trim(s);
    if (upper && token=="infinity") return std::numeric_limits<double>::infinity();
    if (!token.empty() && token.front()=='+') token.remove_prefix(1);
    double value=0;
    const auto parsed=std::from_chars(token.data(),token.data()+token.size(),value,std::chars_format::general);
    if (token.empty() || parsed.ec != std::errc{} || parsed.ptr != token.data()+token.size() || !std::isfinite(value))
        throw FileError(FileErrorCode::Number,"Expected a finite numeric cell",record);
    return value;
}
std::string spelling(double value,const core::NumberFormat& format,bool bound=false) {
    if (bound && std::isinf(value) && value>0) return "infinity";
    if (value==0) {
        if (std::signbit(value)) return "-0";
        return core::NumberFormatter(format).format(0);
    }
    char buffer[128];
    const auto written=std::to_chars(buffer,buffer+sizeof(buffer),value,std::chars_format::general);
    if (written.ec != std::errc{}) throw FileError(FileErrorCode::Number,"Cannot serialize numeric value");
    return {buffer,written.ptr};
}
} // namespace

FileError::FileError(FileErrorCode code,std::string message,std::optional<std::size_t> record,
                     lp::DiagnosticOrigin origin,std::string title)
    : std::runtime_error(std::move(message)),code_(code),record_(record),origin_(origin),title_(std::move(title)) {}

bool LpDocument::dirty() const {
    return !savedProblem_ || !same(problem,*savedProblem_) || !same(inputFormat,savedFormat_);
}

LpDocument ToraLpCodec::create(lp::LpProblem problem,core::NumberFormat format,QuotePolicy quotes) {
    modelCheck(problem);formatCheck(format);
    LpDocument document;document.problem=std::move(problem);document.inputFormat=format;document.quotePolicy_=quotes;
    return document;
}

LpDocument ToraLpCodec::decode(std::string_view bytes,QuotePolicy quotes) {
    if (bytes.size()>MaxFileBytes) throw FileError(FileErrorCode::TooLarge,"LP file exceeds the supported byte limit");
    std::vector<std::string> records;
    std::size_t at=0;
    while (at<bytes.size()) {
        const auto end=bytes.find("\r\n",at);
        if (end==std::string_view::npos) throw FileError(FileErrorCode::Framing,"LP files require CRLF after every record",records.size()+1);
        const auto line=bytes.substr(at,end-at);
        if (line.find_first_of("\r\n") != std::string_view::npos || line.find('\0') != std::string_view::npos)
            throw FileError(FileErrorCode::Framing,"Invalid record separator or NUL",records.size()+1);
        records.emplace_back(line);at=end+2;
    }
    if (records.size()<8) throw FileError(FileErrorCode::RecordCount,"LP header is truncated");
    if (records[0]!="3000")
        throw FileError(FileErrorCode::Marker,"Target file does not match format of elected TORA module. Retry?",1,
                        lp::DiagnosticOrigin::ObservedTora,"Open FileSave As Warning");
    core::NumberFormat format;
    format.n=static_cast<int>(integer(records[1],2,10));format.d=static_cast<int>(integer(records[2],3,10));
    const auto mode=integer(records[3],4,std::numeric_limits<std::size_t>::max());
    if (mode<1 || mode>2) throw FileError(FileErrorCode::UnsupportedFormat,"Unsupported notation mode",4);
    format.notation=mode==1?core::Notation::Decimal:core::Notation::Scientific;
    formatCheck(format);
    const auto n=integer(records[5],6,lp::LpValidator::StudentSizeLimit);
    const auto m=integer(records[6],7,lp::LpValidator::StudentSizeLimit);
    if (n==0 || m==0) throw FileError(FileErrorCode::Header,"Invalid problem size",n==0?6:7);
    const auto required=8+(m+5)*(n+2);
    if (records.size()<required) throw FileError(FileErrorCode::RecordCount,"LP grid is truncated",records.size()+1);
    auto model=lp::LpProblem::blank(n,m);model.title=unquote(records[4],5,quotes);
    const auto sense=unquote(records[7],8,quotes);
    if (sense!="Maximize" && sense!="Minimize") throw FileError(FileErrorCode::Header,"Invalid objective direction",8);
    model.sense=sense=="Maximize"?lp::ObjectiveSense::Maximize:lp::ObjectiveSense::Minimize;
    std::vector<std::string> cells;
    for (std::size_t k=8;k<required;++k) cells.push_back(unquote(records[k],k+1,quotes));
    const auto index=[n](std::size_t row,std::size_t col){return row*(n+2)+col;};
    const auto cell=[&](std::size_t row,std::size_t col)->const std::string&{return cells[index(row,col)];};
    const auto numeric=[&](std::size_t row,std::size_t col,bool upper=false){return number(cell(row,col),9+index(row,col),upper);};
    for (std::size_t j=0;j<n;++j) {
        auto& variable=model.variables[j];variable.name=cell(0,j);model.objective[j]=numeric(1,j);
        variable.lowerBound=numeric(m+2,j);variable.upperBound=numeric(m+3,j,true);
        const auto& flag=cell(m+4,j);
        if (flag!="n" && flag!="y") throw FileError(FileErrorCode::Flag,"Unrestricted flag must be n or y",9+index(m+4,j));
        variable.unrestricted=flag=="y";
    }
    for (std::size_t i=0;i<m;++i) {
        auto& row=model.constraints[i];for (std::size_t j=0;j<n;++j) row.coefficients[j]=numeric(i+2,j);
        const auto rel=trim(cell(i+2,n));
        if (rel!="<=" && rel!=">=" && rel!="=") throw FileError(FileErrorCode::Relation,"Constraint relation must be <=, >= or =",9+index(i+2,n));
        row.relation=rel=="<="?lp::Relation::LessEqual:(rel==">="?lp::Relation::GreaterEqual:lp::Relation::Equal);
        row.rhs=numeric(i+2,n+1);
    }
    modelCheck(model);
    LpDocument document;document.problem=model;document.inputFormat=format;document.quotePolicy_=quotes;
    document.originalProblem_=model;document.savedProblem_=std::move(model);
    document.originalFormat_=format;document.savedFormat_=format;document.originalRecords_=std::move(records);
    return document;
}

std::string ToraLpCodec::encode(const LpDocument& document) {
    modelCheck(document.problem);formatCheck(document.inputFormat);
    const auto& p=document.problem;const auto n=p.variables.size(),m=p.constraints.size();
    const auto quoted=[&](const std::string& text){return quote(text,document.quotePolicy_);};
    const auto* old=document.originalProblem_?&*document.originalProblem_:nullptr;
    const bool shape=old && old->variables.size()==n && old->constraints.size()==m;
    std::vector<std::string> records(8+(m+5)*(n+2),"\"\"");
    const auto put=[&](std::size_t k,std::string raw,bool unchanged){records[k]=unchanged && shape?document.originalRecords_.at(k):std::move(raw);};
    records[0]="3000";
    put(1,std::to_string(document.inputFormat.n),old && document.inputFormat.n==document.originalFormat_.n);
    put(2,std::to_string(document.inputFormat.d),old && document.inputFormat.d==document.originalFormat_.d);
    put(3,document.inputFormat.notation==core::Notation::Decimal?"1":"2",old && document.inputFormat.notation==document.originalFormat_.notation);
    put(4,quoted(p.title),old && p.title==old->title);put(5,std::to_string(n),true);put(6,std::to_string(m),true);
    put(7,quoted(p.sense==lp::ObjectiveSense::Maximize?"Maximize":"Minimize"),old && p.sense==old->sense);
    const auto index=[n](std::size_t row,std::size_t col){return 8+row*(n+2)+col;};
    if (shape) {
        // Reserved relation/RHS cells outside constraint rows remain opaque.
        for (const auto row:{std::size_t(0),std::size_t(1),m+2,m+3,m+4})
            for (std::size_t col=n;col<n+2;++col) records[index(row,col)]=document.originalRecords_[index(row,col)];
    } else if (old) {
        const auto oldN=old->variables.size(),oldM=old->constraints.size();
        for (const auto row:{std::size_t(0),std::size_t(1),oldM+2,oldM+3,oldM+4})
            for (std::size_t col=oldN;col<oldN+2;++col)
                if (document.originalRecords_[8+row*(oldN+2)+col]!="\"\"")
                    throw FileError(FileErrorCode::Model,"Cannot resize a document with unknown reserved grid data");
    }
    for (std::size_t j=0;j<n;++j) {
        const auto& v=p.variables[j];const auto* before=shape?&old->variables[j]:nullptr;
        put(index(0,j),quoted(v.name),before && v.name==before->name);
        put(index(1,j),quoted(spelling(p.objective[j],document.inputFormat)),shape && same(p.objective[j],old->objective[j]));
        put(index(m+2,j),quoted(spelling(v.lowerBound,document.inputFormat,true)),before && same(v.lowerBound,before->lowerBound));
        put(index(m+3,j),quoted(spelling(v.upperBound,document.inputFormat,true)),before && same(v.upperBound,before->upperBound));
        put(index(m+4,j),quoted(v.unrestricted?"y":"n"),before && v.unrestricted==before->unrestricted);
    }
    for (std::size_t i=0;i<m;++i) {
        const auto& row=p.constraints[i];const auto* before=shape?&old->constraints[i]:nullptr;
        for (std::size_t j=0;j<n;++j) put(index(i+2,j),quoted(spelling(row.coefficients[j],document.inputFormat)),before && same(row.coefficients[j],before->coefficients[j]));
        put(index(i+2,n),quoted(row.relation==lp::Relation::LessEqual?"        <=":(row.relation==lp::Relation::GreaterEqual?"        >=":"        =")),before && row.relation==before->relation);
        put(index(i+2,n+1),quoted(spelling(row.rhs,document.inputFormat)),before && same(row.rhs,before->rhs));
    }
    if (old) {
        const auto start=8+(old->constraints.size()+5)*(old->variables.size()+2);
        records.insert(records.end(),document.originalRecords_.begin()+static_cast<std::ptrdiff_t>(start),document.originalRecords_.end());
    }
    std::string bytes;for (const auto& record:records) bytes+=record+"\r\n";
    if (bytes.size()>MaxFileBytes) throw FileError(FileErrorCode::TooLarge,"LP file exceeds the supported byte limit");
    return bytes;
}
} // namespace tora::compat
