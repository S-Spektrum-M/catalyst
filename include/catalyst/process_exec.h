#pragma once
#include <expected>
#include <future>

namespace catalyst {
std::expected<std::future<int>, std::string> process_exec(std::string command);
}
