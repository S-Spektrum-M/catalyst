#include "catalyst/subcommands/generate.hpp"

#include <print>

namespace catalyst::generate::BuildWriters {

BaseWriter::BaseWriter(std::ostream &stream) : stream(stream) {
}
} // namespace catalyst::generate::BuildWriters
