/// @file include/globals.h
/// @brief This file is going to be starting off point for variables that should exist for the lifecycle of the pgram

#include <fstream>
#include <yaml-cpp/yaml.h>

/// This is the log file that we write to. It's important that we have this be append,
/// so that we can keep records over the lifetime of the project.
/// For the specifics on the logfile read DOCS/logfile.md
static std::ofstream logfile{".catalyst.log", std::ios::app};
