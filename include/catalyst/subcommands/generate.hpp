#pragma once
#include <CLI/App.hpp>
#include <expected>
#include <ostream>
#include <stdexcept>
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
    virtual std::expected<void, std::string> add_rule(std::string_view name,
                                                      std::string_view command,
                                                      std::string_view description,
                                                      std::string_view depfile = "",
                                                      std::string_view deps = "") = 0;
    virtual std::expected<void, std::string> add_build(const std::vector<target_t> &outputs,
                                                       std::string_view rule,
                                                       const std::vector<target_t> &inputs,
                                                       const std::vector<target_t> &implicit_deps = {}
                                                       // e.g., headers for validation
                                                       ) = 0;

    virtual void add_comment(std::string_view comment) = 0;
    virtual void add_default(std::string_view target) = 0;
};

enum class TargetType {
    Ninja,
    Make,
    VisualStudio,
    XCode,
};

template <TargetType T> class DerivedWriter : public BaseWriter {
private:
    static consteval bool is_implemented(TargetType __t) {
        return __t == TargetType::Ninja;
    }

#if __cplusplus >= 202602L
    static consteval std::string warning_msg(TargetType __t) {
        switch (__t) {
            case TargetType::Ninja:
                return "Unimplemented specialization for DerivedWriter<Ninja>. "
                       "Add explicit template specialization.";
            case TargetType::Make:
                return "Unimplemented specialization for DerivedWriter<Make>. "
                       "Add explicit template specialization.";
            case TargetType::VisualStudio:
                return "Unimplemented specialization for DerivedWriter<VisualStudio>. "
                       "Add explicit template specialization.";
            case TargetType::XCode:
                return "Unimplemented specialization for DerivedWriter<XCode>. "
                       "Add explicit template specialization.";
        }
        return "Unknown TargetType";
    }
    static_assert(is_implemented(T), warning_msg(T));
#else
    static_assert(is_implemented(T),
                  "Unimplemented specialization for DerivedWriter. Add wxplicit template specialization.");
#endif

public:
    explicit DerivedWriter(std::ostream &stream) : BaseWriter(stream) {
    }

    ~DerivedWriter() override = default;

    // Delete copy/move to prevent slicing
    DerivedWriter(const DerivedWriter &) = delete;
    DerivedWriter &operator=(const DerivedWriter &) = delete;
    DerivedWriter(DerivedWriter &&) = delete;
    DerivedWriter &operator=(DerivedWriter &&) = delete;

    std::expected<void, std::string> add_variable([[maybe_unused]] std::string_view name,
                                                  [[maybe_unused]] std::string_view value) override {
        throw std::logic_error("Unimplemented base template method");
    }

    std::expected<void, std::string> add_rule([[maybe_unused]] std::string_view name,
                                              [[maybe_unused]] std::string_view command,
                                              [[maybe_unused]] std::string_view description,
                                              [[maybe_unused]] std::string_view depfile = "",
                                              [[maybe_unused]] std::string_view deps = "") override {
        throw std::logic_error("Unimplemented base template method");
    }

    std::expected<void, std::string>
    add_build([[maybe_unused]] const std::vector<target_t> &outputs,
              [[maybe_unused]] std::string_view rule,
              [[maybe_unused]] const std::vector<target_t> &inputs,
              [[maybe_unused]] const std::vector<target_t> &implicit_deps = {}) override {
        throw std::logic_error("Unimplemented base template method");
    }

    void add_comment([[maybe_unused]] std::string_view comment) override {
        throw std::logic_error("Unimplemented base template method");
    }

    void add_default([[maybe_unused]] std::string_view target) override {
        throw std::logic_error("Unimplemented base template method");
    }
};

class NinjaWriter : public BaseWriter {
public:
    explicit NinjaWriter(std::ostream &stream);
    std::expected<void, std::string> add_variable(std::string_view name, std::string_view value) override;

    std::expected<void, std::string> add_rule(std::string_view name,
                                              std::string_view command,
                                              std::string_view description,
                                              std::string_view depfile = "",
                                              std::string_view deps = "") override;

    std::expected<void, std::string> add_build(const std::vector<target_t> &outputs,
                                               std::string_view rule,
                                               const std::vector<target_t> &inputs,
                                               const std::vector<target_t> &implicit_deps = {}) override;

    void add_comment(std::string_view comment) override;
    void add_default(std::string_view target) override;
};

} // namespace BuildWriters
} // namespace catalyst::generate
