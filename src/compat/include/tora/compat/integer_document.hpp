#pragma once
#include "lp_file.hpp"
namespace tora::compat {
// Observed5000 original LP grid followed by an Integer(y/n)? row.
class IntegerDocument {
public:
    LpDocument lp;
    std::vector<bool> integral;
    static IntegerDocument decode(std::string_view bytes);
    static IntegerDocument create(LpDocument lp,std::vector<bool> integral);
    std::string encode()const;
private:
    std::vector<std::string> flagRecords_;
    std::vector<bool> originalFlags_;
};
}
