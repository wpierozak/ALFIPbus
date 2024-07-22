#pragma once

#include <string>
#include <vector>
#include <stdexcept>

namespace utils
{
void throwRuntime(std::string mess, std::string file, int line);
std::vector<std::string> splitString(const std::string& text, std::string by);
} // namespace utils

#define THROW_RUNTIME(mess) utils::throwRuntime(mess, __FILE__, __LINE__)