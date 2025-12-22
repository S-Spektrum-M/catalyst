#pragma once
#include <expected>
#include <future>
#include <vector>

namespace catalyst {
std::expected<std::future<int>, std::string> process_exec(std::vector<std::string> &&args);
} // namespace catalyst
