#ifndef SWT_UTILS_H
#define SWT_UTILS_H

#include <string>
#include <vector>
#include <stdexcept>

namespace fit_swt
{

namespace utils
{
    std::vector<std::string> splitString(const std::string& text, std::string by);
} // namespace utils

}

#endif