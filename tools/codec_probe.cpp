// Qt-free reference bridge. Generated outputs are private build artifacts.
#include <tora/compat/lp_file.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc,char** argv) {
    try {
        if (argc!=4) throw std::invalid_argument("Use --roundtrip/--edit input output, or --new decimal/scientific output");
        tora::compat::LpDocument document;
        const std::string operation=argv[1];
        if (operation=="--new") {
            tora::core::NumberFormat format;
            if (std::string(argv[2])=="scientific") format.notation=tora::core::Notation::Scientific;
            else if (std::string(argv[2])!="decimal") throw std::invalid_argument("Unknown notation");
            document=tora::compat::ToraLpCodec::create(tora::lp::LpProblem::blank(2,1),format);
            document.problem.title="Clone fresh "+std::string(argv[2]);
        } else {
            document=tora::compat::LpFileStore::open(argv[2]);
            if (operation=="--edit") {
                document.problem.title="Clone precision bridge";
                document.problem.objective.at(0)=1.2345678901234567;
                document.problem.variables.at(0).name="decision";
            } else if (operation!="--roundtrip") throw std::invalid_argument("Unknown operation");
        }
        tora::compat::LpFileStore::saveAs(document,argv[3]);
        std::cout<<"Clone wrote "<<document.problem.variables.size()<<" variables / "<<document.problem.constraints.size()<<" constraints\n";
    } catch (const std::exception& error) {
        std::cerr<<error.what()<<'\n';return 1;
    }
}
