#pragma once
#include <filesystem>
namespace catalyst {
class DirectoryChangeGuard {
public:
    explicit DirectoryChangeGuard(const std::filesystem::path &path_to_change_to)
        : original_path(std::filesystem::current_path()) {
        std::filesystem::current_path(path_to_change_to);
    }
    ~DirectoryChangeGuard() noexcept {
        std::filesystem::current_path(original_path);
    }

    DirectoryChangeGuard(const DirectoryChangeGuard &) =
        delete; ///< RAII guard must have unique ownership of directory state"
    DirectoryChangeGuard &
    operator=(const DirectoryChangeGuard &) = delete; ///< RAII guard must have unique ownership of directory state"
    DirectoryChangeGuard(DirectoryChangeGuard &&) =
        delete; ///< moved-from object would restore to invalid path on destruction
    DirectoryChangeGuard &
    operator=(DirectoryChangeGuard &&) = delete; ///< moved-from object would restore to invalid path on destruction

private:
    std::filesystem::path original_path;
};
} // namespace catalyst
