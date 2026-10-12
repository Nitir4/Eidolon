#include <tora/transportation/north_west.hpp>
#include <tora/compat/transportation_reader.hpp>
#include <tora/compat/transportation_document.hpp>
#include <iostream>
#include <locale>
#include <fstream>
#include <iterator>
#include <string>
#include <tuple>
#include <functional>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

static void quote(const std::string& value) {
    std::cout << '"';
    for (char c : value) { if (c=='"' || c=='\\') std::cout << '\\'; std::cout << c; }
    std::cout << '"';
}
static void array(const std::vector<std::string>& values) {
    std::cout << '[';
    for (std::size_t i=0;i<values.size();++i) {if(i)std::cout<<',';quote(values[i]);}
    std::cout << ']';
}
static void matrix(const std::vector<std::vector<std::string>>& values) {
    std::cout << '[';
    for (std::size_t i=0;i<values.size();++i) {if(i)std::cout<<',';array(values[i]);}
    std::cout << ']';
}
int main(int argc,char** argv) {
    try {
        std::cin.imbue(std::locale::classic());
        tora::transportation::Problem problem;
        auto method=tora::transportation::StartMethod::NorthWest;
        std::string filename;
        bool roundtrip=false;
        using Document = tora::compat::TransportationDocument;
        std::vector<std::function<void(Document&)>> edits;
        const auto axis = [](const std::string& name) {
            if (name == "source") return Document::Axis::Source;
            if (name == "destination") return Document::Axis::Destination;
            throw std::invalid_argument("Unknown structural axis");
        };
        std::size_t newSources=0,newDestinations=0;std::string newTitle;
        for(int i=1;i<argc;++i) {
            const std::string arg=argv[i];
            if(arg=="--file" && i+1<argc)filename=argv[++i];
            else if(arg=="--roundtrip")roundtrip=true;
            else if(arg=="--new" && i+3<argc) {
                newSources=static_cast<std::size_t>(std::stoul(argv[++i]));
                newDestinations=static_cast<std::size_t>(std::stoul(argv[++i]));newTitle=argv[++i];
            }
            else if(arg=="--edit" && i+3<argc) {
                const auto row=std::stoi(argv[++i]),column=std::stoi(argv[++i]);
                const std::string value = argv[++i];
                edits.emplace_back([row,column,value](Document& document) {
                    document.setCell(static_cast<std::size_t>(row),static_cast<std::size_t>(column),value);
                });
            }
            else if((arg=="--insert" || arg=="--erase") && i+2<argc) {
                const auto selectedAxis=axis(argv[++i]);
                const auto position=static_cast<std::size_t>(std::stoul(argv[++i]));
                edits.emplace_back([arg,selectedAxis,position](Document& document) {
                    if(arg=="--insert")document.insert(selectedAxis,position);
                    else document.erase(selectedAxis,position);
                });
            }
            else if(arg=="--paste-line" && i+3<argc) {
                const auto selectedAxis=axis(argv[++i]);
                const auto from=static_cast<std::size_t>(std::stoul(argv[++i]));
                const auto to=static_cast<std::size_t>(std::stoul(argv[++i]));
                edits.emplace_back([selectedAxis,from,to](Document& document) {
                    document.paste(selectedAxis,to,document.line(selectedAxis,from));
                });
            }
            else if(arg=="--method" && i+1<argc) {
                const std::string name=argv[++i];
                if(name=="leastcost")method=tora::transportation::StartMethod::LeastCost;
                else if(name=="vogel")method=tora::transportation::StartMethod::Vogel;
                else if(name!="northwest")throw std::invalid_argument("Unknown method");
            } else throw std::invalid_argument("Unknown or incomplete probe option");
        }
#ifdef _WIN32
        if(roundtrip)_setmode(_fileno(stdout),_O_BINARY);
#endif
        const auto applyEdits=[&edits](tora::compat::TransportationDocument& document) {
            for(const auto& edit:edits) edit(document);
        };
        if(newSources) {
            auto document=tora::compat::TransportationDocument::blank(newSources,newDestinations,newTitle);
            applyEdits(document);
            if(roundtrip) {std::cout<<document.encode();return 0;}
            problem=document.problem();
        }
        else if(!filename.empty()) {
            std::ifstream file(filename,std::ios::binary);
            if(!file)throw std::runtime_error("Cannot open probe file");
            const std::string bytes{std::istreambuf_iterator<char>(file),{}};
            auto document=tora::compat::TransportationDocument::decode(bytes);
            applyEdits(document);
            if(roundtrip) {std::cout<<document.encode();return 0;}
            problem=document.problem();
        }else {
        std::size_t m=0,n=0;std::cin>>m>>n;
        problem=tora::transportation::Problem::blank(m,n);
        for (auto& row:problem.costs)for(auto& value:row)std::cin>>value;
        for(auto& value:problem.supply)std::cin>>value;
        for(auto& value:problem.demand)std::cin>>value;
        if(!std::cin)throw std::runtime_error("Invalid probe input");
        if(roundtrip) {std::cout<<tora::compat::TransportationDocument::create(problem).encode();return 0;}
        }
        const auto trace=tora::transportation::solve(problem,method);
        std::cout<<'[';
        for(std::size_t k=0;k<trace.size();++k) {
            if(k)std::cout<<',';
            const auto display=tora::transportation::display(problem,trace[k],k+1);
            std::cout<<"{\"label\":";quote(display.label);
            std::cout<<",\"objective\":";quote(display.objective);
            std::cout<<",\"u\":";array(display.u);std::cout<<",\"v\":";array(display.v);
            std::cout<<",\"costs\":";matrix(display.costs);
            std::cout<<",\"shipments\":";matrix(display.shipments);
            std::cout<<",\"reduced_costs\":";matrix(display.reducedCosts);
            std::cout<<",\"supply\":";array(display.supply);
            std::cout<<",\"demand\":";array(display.demand);std::cout<<'}';
        }
        std::cout<<']';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
