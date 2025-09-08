#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace catalyst::build {
void build_compdb() {
    std::cout << std::setw(4) << json::meta() << std::endl;
}
}; // namespace catalyst::build
