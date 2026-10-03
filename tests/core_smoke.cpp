#include <tora/build_info.hpp>

#include <iostream>

static_assert(__cplusplus >= 201703L, "C++17 or newer is required");

int main()
{
    if (tora::project_name.empty() || tora::version.empty()) {
        std::cerr << "Missing configured project metadata\n";
        return 1;
    }
    std::cout << tora::project_name << ' ' << tora::version << '\n';
    return 0;
}
