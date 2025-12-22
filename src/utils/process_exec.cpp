#include "catalyst/process_exec.h"

#include <expected>
#include <future>
#include <string>

/// async wrapper around std::system()
/// TODO: migrate to safe exec functions like in reprco
namespace catalyst {
std::expected<std::future<int>, std::string> process_exec(std::string command) {
    return std::async(std::launch::async, [command] -> int { return std::system(command.c_str()); });
}
} // namespace catalyst
