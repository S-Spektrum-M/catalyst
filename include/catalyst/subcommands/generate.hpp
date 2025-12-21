#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace catalyst::generate {
struct parse_t {
    std::vector<std::string> profiles;
    std::vector<std::string> enabled_features;
};

struct find_res {
    std::string lib_path;
    std::string inc_path;
    std::string libs;
};

std::expected<YAML::Node, std::string> profile_composition(const std::vector<std::string> &profiles);
std::pair<CLI::App *, std::unique_ptr<parse_t>> parse(CLI::App &app);
std::expected<void, std::string> action(const parse_t &);
void resolve_vcpkg_dependency(const YAML::Node &dep,
                              const std::string &triplet,
                              std::string &ldflags,
                              std::string &ldlibs);
std::expected<void, std::string> resolve_local_dependency(
    const YAML::Node &dep, std::string &cxxflags, std::string &ccflags, std::string &ldflags, std::string &ldlibs);
void resolve_pkg_config_dependency(
    const YAML::Node &dep, std::string &cxxflags, std::string &ccflags, std::string &ldflags, std::string &ldlibs);
void resolve_system_dependency(
    const YAML::Node &dep, std::string &cxxflags, std::string &ccflags, std::string &ldflags, std::string &ldlibs);
std::expected<std::string, std::string> lib_path(const YAML::Node &profile);
std::expected<find_res, std::string> find_dep(const std::string &build_dir, const YAML::Node &dep);
std::expected<find_res, std::string> find_local(const YAML::Node &dep);
std::expected<find_res, std::string> find_system(const YAML::Node &dep);
std::expected<find_res, std::string> find_vcpkg(const YAML::Node &dep);
std::expected<find_res, std::string> find_git(const std::string &build_dir, const YAML::Node &dep);

std::expected<std::unordered_set<std::filesystem::path>, std::string>
build_source_set(std::vector<std::string> source_dirs, const std::vector<std::string> &profiles);

namespace BuildWriters {
class BaseWriter {
protected:
    std::ostream &stream;
    explicit BaseWriter(std::ostream &stream);

public:
    using rule_t = std::string;
    using target_t = std::string;

    virtual ~BaseWriter() = default;

    virtual std::expected<void, std::string> add_variable(std::string_view name, std::string_view value) = 0;
    virtual std::expected<void, std::string>
    add_rule(std::string_view name, std::string_view command, std::string_view description, std::string_view depfile = "",
             std::string_view deps = "") = 0;
    virtual std::expected<void, std::string> add_build(const std::vector<target_t> &outputs,
                                                       std::string_view rule,
                                                       const std::vector<target_t> &inputs,
                                                       const std::vector<target_t> &implicit_deps = {}
                                                       // e.g., headers for validation
                                                       ) = 0;

    virtual void add_comment(std::string_view comment) = 0;
};

class NinjaWriter : public BaseWriter {
    std::string escape(std::string_view str);

public:
    explicit NinjaWriter(std::ostream &stream);
    std::expected<void, std::string> add_variable(std::string_view name, std::string_view value) override;

    std::expected<void, std::string>
    add_rule(std::string_view name, std::string_view command, std::string_view description, std::string_view depfile = "",
             std::string_view deps = "") override;

    std::expected<void, std::string> add_build(const std::vector<target_t> &outputs,
                                               std::string_view rule,
                                               const std::vector<target_t> &inputs,
                                               const std::vector<target_t> &implicit_deps = {}) override;

    void add_comment(std::string_view comment) override;
};

} // namespace BuildWriters
} // namespace catalyst::generate
