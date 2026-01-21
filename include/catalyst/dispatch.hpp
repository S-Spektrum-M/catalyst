#include "catalyst/GLOBALS.hpp"
#include "catalyst/subcommands/add.hpp"
#include "catalyst/subcommands/build.hpp"
#include "catalyst/subcommands/clean.hpp"
#include "catalyst/subcommands/download.hpp"
#include "catalyst/subcommands/fetch.hpp"
#include "catalyst/subcommands/fmt.hpp"
#include "catalyst/subcommands/generate.hpp"
#include "catalyst/subcommands/init.hpp"
#include "catalyst/subcommands/install.hpp"
#include "catalyst/subcommands/run.hpp"
#include "catalyst/subcommands/test.hpp"
#include "catalyst/subcommands/tidy.hpp"
#include "catalyst/workspace.hpp"

namespace catalyst {
struct CliContext {
    CLI::App app{"Catalyst is a Modern Declarative C++ Build System."};
    std::optional<Workspace> workspace;
    bool show_version{false};
    bool helped{false};

    CLI::App *add_subc{nullptr};
    std::unique_ptr<catalyst::add::parse_t> add_res{nullptr};

    CLI::App *build_subc{nullptr};
    std::unique_ptr<catalyst::build::parse_t> build_res{nullptr};

    CLI::App *clean_subc{nullptr};
    std::unique_ptr<catalyst::clean::parse_t> clean_res{nullptr};

    CLI::App *download_subc{nullptr};
    std::unique_ptr<catalyst::download::parse_t> download_res{nullptr};

    CLI::App *fetch_subc{nullptr};
    std::unique_ptr<catalyst::fetch::parse_t> fetch_res{nullptr};

    CLI::App *fmt_subc{nullptr};
    std::unique_ptr<catalyst::fmt::parse_t> fmt_res{nullptr};

    CLI::App *generate_subc{nullptr};
    std::unique_ptr<catalyst::generate::parse_t> generate_res{nullptr};

    CLI::App *init_subc{nullptr};
    std::unique_ptr<catalyst::init::parse_t> init_res{nullptr};

    CLI::App *install_subc{nullptr};
    std::unique_ptr<catalyst::install::parse_t> install_res{nullptr};

    CLI::App *run_subc{nullptr};
    std::unique_ptr<catalyst::run::parse_t> run_res{nullptr};

    CLI::App *test_subc{nullptr};
    std::unique_ptr<catalyst::test::parse_t> test_res{nullptr};

    CLI::App *tidy_subc{nullptr};
    std::unique_ptr<catalyst::tidy::parse_t> tidy_res{nullptr};

    CLI::App *add_git_subc{nullptr};
    std::unique_ptr<catalyst::add::git::parse_t> add_git_res{nullptr};

    CLI::App *add_system_subc{nullptr};
    std::unique_ptr<catalyst::add::system::parse_t> add_system_res{nullptr};

    CLI::App *add_local_subc{nullptr};
    std::unique_ptr<catalyst::add::local::parse_t> add_local_res{nullptr};

    CLI::App *add_vcpkg_subc{nullptr};
    std::unique_ptr<catalyst::add::vcpkg::parse_t> add_vcpkg_res{nullptr};
};

std::pair<int, bool> parseCli(int argc, char **argv, catalyst::CliContext &ctx);
int dispatch(const catalyst::CliContext &ctx, const std::string &args_str);
} // namespace catalyst
