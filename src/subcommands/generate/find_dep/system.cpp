#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include <expected>
#include <string>

namespace catalyst::generate {
std::expected<find_res, std::string> find_system(const YAML::Node &dep) {
    std::string dep_name = dep["name"].as<std::string>();
    catalyst::logger.log(LogLevel::INFO, "Resolving system dependency: {}", dep_name);
    bool used_explicit_paths = false;

    std::string linkage = "shared";
    if (dep["linkage"] && dep["linkage"].IsScalar()) {
        linkage = dep["linkage"].as<std::string>();
    }

    std::string inc_path;
    if (dep["include"]) {
        inc_path += std::format(" -I{}", dep["include"].as<std::string>());
        used_explicit_paths = true;
    } else {
#if defined(_WIN32)
#elif defined(__APPLE__)
        inc_path = "-I/usr/local/include";
#else
        inc_path = "-I/usr/include";
#endif
    }

    std::string lib_path;
    if (dep["lib"]) {
        lib_path += std::format(" -L{}", dep["lib"].as<std::string>());
        used_explicit_paths = true;
    } else {
#if defined(_WIN32)
#elif defined(__APPLE__)
        lib_path = "-L/usr/local/lib";
#else
        lib_path = "-L/usr/lib";
#endif
    }

    std::string libs;
    if (used_explicit_paths && (linkage == "static" || linkage == "shared")) {
        libs += std::format(" -l{}", dep_name);
    }
    return find_res{.lib_path = lib_path, .inc_path = inc_path, .libs = libs};
}
} // namespace catalyst::generate
