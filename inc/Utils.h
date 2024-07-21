#pragma once

#include<string>
#include<vector>
#include<stdexcept>

namespace Utils
{
    void throw_runtime(std::string mess, std::string file, int line);
    std::vector<std::string> splitString(const std::string& text, std::string by);
}

#define THROW_RUNTIME(mess) Utils::throw_runtime(mess, __FILE__, __LINE__)