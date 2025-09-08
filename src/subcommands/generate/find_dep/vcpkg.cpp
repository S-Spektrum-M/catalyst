#include "catalyst/log-utils/log.hpp"
#include "catalyst/subcommands/generate.hpp"
#include <expected>
#include <format>

namespace catalyst::generate {
std::expected<find_res, std::string> find_vcpkg(const YAML::Node &dep) {
    auto triplet = dep["triplet"].as<std::string>();

    const char *vcpkg_root_env = std::getenv("VCPKG_ROOT");
    if (vcpkg_root_env == nullptr) {
        return std::unexpected(
            std::format("VCPKG_ROOT is not set, cannot resolve vcpkg dependency '{}'.", dep["name"].as<std::string>()));
    }

    std::string dep_name = dep["name"].as<std::string>();
    catalyst::logger.log(LogLevel::INFO, "Resolving vcpkg dependency: {}", dep_name);

    // Construct the path to the library directory within the specific package folder
    // $VCPKG_ROOT/packages/<package>_<triplet>/lib
    namespace fs = std::filesystem;

    fs::path vcpkg_root(vcpkg_root_env);
    fs::path package_dir_name = std::format("{}_{}", dep_name, triplet);
    fs::path lib_path = vcpkg_root / "packages" / package_dir_name / "lib";

    std::string library_path, libs;

    if (!fs::exists(lib_path) || !fs::is_directory(lib_path)) {
        catalyst::logger.log(LogLevel::WARN, "Could not find library directory for vcpkg package '{}' at: {}", dep_name,
                             lib_path.string());
        libs += std::format(" -l{}", dep_name);
    }

    library_path += std::format(" -L{}", lib_path.string());
    catalyst::logger.log(LogLevel::INFO, "Adding library path: {}", lib_path.string());

// Define the library file extensions based on the operating system.
#if defined(_WIN32)
    const std::vector<std::string> extensions = {".lib"};
#elif defined(__APPLE__)
    const std::vector<std::string> extensions = {".a", ".dylib"};
#else // Linux and other Unix-like systems
    const std::vector<std::string> extensions = {".a", ".so"};
#endif

    // Iterate through the directory and find matching library files.
    for (const auto &entry : fs::directory_iterator(lib_path)) {
        if (entry.is_regular_file()) {
            const fs::path &file_path = entry.path();
            std::string file_ext = file_path.extension().string();

            // Check if the file has one of the target extensions
            for (const auto &expected_ext : extensions) {
                if (file_ext == expected_ext) {
                    // Convert file path to a linker flag (e.g., "libfmt.a" -> "-lfmt")
                    std::string stem = file_path.stem().string();
                    if (stem.rfind("lib", 0) == 0) { // Check if it starts with "lib"
                        stem = stem.substr(3);
                    }
                    libs += std::format(" -l{}", stem);
                    catalyst::logger.log(LogLevel::INFO, "Found and added library: {}", stem);
                    break; // Found a matching extension, move to the next file
                }
            }
        }
    }

    return find_res{
        .lib_path = library_path,
        .inc_path = "", // already set in write_variables
        .libs = libs
    };
}
} // namespace catalyst::generate
