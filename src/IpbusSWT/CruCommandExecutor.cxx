#include "IpbusSWT/CruCommandExecutor.h"

void CruCommandExecutor::execute(CruCommandBuffer& cmdBuffer, fit_swt::SwtFifo swtFifo, std::string& response)
{
    for(uint32_t idx = 0; idx < cmdBuffer.size; idx++){
        switch(cmdBuffer[idx].type)
        {
            case CruCommandSequnce::Command::Type::Read:
                exectureRead(cmdBuffer[idx], swtFifo, response);
                break;
            case CruCommandSequnce::Command::Type::ReadCnt:
                exectureReadCnt(cmdBuffer[idx], swtFifo, response);
                break;
            case CruCommandSequnce::Command::Type::Write:
                executerWrite(cmdBuffer[idx], swtFifo, response);
                break;
            case CruCommandSequnce::Command::Type::Wait:
                executeWait(cmdBuffer[idx], swtFifo, response);
                break;
            default:
                break;
        }
    }
    cmdBuffer.reset();
}

void CruCommandExecutor::exectureRead(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo swtFifo, std::string& response)
{
    if(swtFifo.empty()){
        throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadStr) + ": there is now data in SWT FIFO to read!");
    }
    
    response.resize(response.size() + swtFifo.size() * (fit_swt::Swt::SwtStrLen + 1));
    while(swtFifo.empty() == false){
        fit_swt::writeSwtFrameToStrBuffer(swtFifo.pop(), response);
        response.append("\n");
    }
}

void CruCommandExecutor::exectureReadCnt(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo swtFifo, std::string& response)
{
    if(swtFifo.empty()){
         throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadStr) + ": there is now data in SWT FIFO to read!");
    }

    response.resize(response.size() + swtFifo.size() * (fit_swt::Swt::SwtStrLen + 1));
    for(uint32_t cnt = 0; cnt < cmd.data.wordsToRead; cnt++){
        fit_swt::writeSwtFrameToStrBuffer(swtFifo.pop(), response);
        response.append("\n");
    }

    if (cmd.data.wordsToRead != swtFifo.size()) {
        throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadCntStr) + ": mismatch between expected words to read and words in SWT FIFO!");
    }
}

void CruCommandExecutor::executerWrite(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo swtFifo, std::string& response)
{
    response.append("0\n");
}

void CruCommandExecutor::executeWait(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo swtFifo, std::string& response)
{
    response.append(std::to_string(cmd.data.waitTime) + "\n");
}