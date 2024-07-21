#include "Utils.h"

namespace Utils
{

void throw_runtime(std::string mess, std::string file, int line)
{
    throw std::runtime_error("\nFILE: " + file + "\nLINE: " + std::to_string(line) + "\nMESSAGE: " + mess);
}

std::vector<std::string> splitString(const std::string& text, std::string by)
{
    std::vector<std::string> result;
    std::string temp = text;
    
    while(temp.size())
    {
        size_t index = temp.find(by);
        if(index != std::string::npos)
        {
            result.push_back(temp.substr(0, index));
            temp = temp.substr(index + by.length());
        }
        else
        {
            result.push_back(temp);
            temp = "";
            break;
        }
    }

    return result;
}
}