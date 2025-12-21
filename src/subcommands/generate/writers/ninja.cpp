#include "catalyst/subcommands/generate.hpp"

namespace catalyst::generate::BuildWriters {
std::string NinjaWriter::escape(std::string_view str) {
    constexpr double ESCAPE_TUNING_FACTOR = 1.25;
    std::string result;
    result.reserve(static_cast<size_t>(str.size() * ESCAPE_TUNING_FACTOR)); // NOTE: allow some extra space for escape
                                                                            // sequences
                                                                            // PERF: subject to tunining
    for (char c : str) {
        switch (c) {
            case '$':
                result.append("$$");
                break;
            case ' ':
                result.append("$ ");
                break;
            case ':':
                result.append("$:");
                break;
            case '\n':
                result.append("$\n");
                break;
            default:
                result.push_back(c);
                break;
        }
    }
    return result;
}

NinjaWriter::NinjaWriter(std::ostream &stream) : BaseWriter(stream) {
}

std::expected<void, std::string> NinjaWriter::add_variable(std::string_view name, std::string_view value) {
    std::println(stream, "{} = {}", name, value);
    return {};
}

std::expected<void, std::string> NinjaWriter::add_rule(std::string_view name,
                                                       std::string_view command,
                                                       std::string_view description,
                                                       std::string_view depfile,
                                                       std::string_view deps) {
    std::println(stream, "rule {}\n  command = {} ", name, command);
    if (!description.empty()) {
        std::println(stream, "  description = {}", description);
    }
    if (!depfile.empty()) {
        std::println(stream, "  depfile = {}", depfile);
    }
    if (!deps.empty()) {
        std::println(stream, "  deps = {}", deps);
    }
    std::println(stream);
    return {};
}

std::expected<void, std::string> NinjaWriter::add_build(const std::vector<target_t> &outputs,
                                                        std::string_view rule,
                                                        const std::vector<target_t> &inputs,
                                                        const std::vector<target_t> &implicit_deps) {
    std::print(stream, "build");
    for (const auto &out : outputs)
        std::print(stream, " {}", escape(out));

    std::print(stream, ": {}", rule);

    for (const auto &in : inputs)
        std::print(stream, " {}", escape(in));

    if (!implicit_deps.empty()) {
        std::print(stream, " |");
        for (const auto &dep : implicit_deps)
            std::print(stream, " {}", escape(dep));
    }

    std::println(stream);
    return {};
}

void NinjaWriter::add_comment(std::string_view comment) {
    std::println(stream, "# {}", comment);
}

void NinjaWriter::add_default(std::string_view target) {
    std::println(stream, "default {}", escape(target));
}
}
