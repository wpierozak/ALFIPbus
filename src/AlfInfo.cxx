#include "AlfInfo.h"
#include <fstream>
#include <boost/log/trivial.hpp>

void AlfInfo::rpcHandler()
{
    std::ifstream file(MetadataFile.data());
    if(file.is_open() == false){
        BOOST_LOG_TRIVIAL(error) << "Metadata file is missing! Expected: " << MetadataFile.data();
    }
    std::string response;
    std::string content(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
    setData(content.c_str());
}


bool AlfInfo::isMetadataFileAvailable()
{
    std::ifstream file(MetadataFile.data());
    if(file.is_open() == false){
        BOOST_LOG_TRIVIAL(error) << "Metadata file is missing! Expected: " << MetadataFile.data();
        return false;
    }
    return true;
}