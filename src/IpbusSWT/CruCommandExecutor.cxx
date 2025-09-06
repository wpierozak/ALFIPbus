#include "IpbusSWT/CruCommandExecutor.h"

void CruCommandExecutor::execute(CruCommandBuffer& cmdBuffer, fit_swt::SwtFifo& swtFifo, std::string& response)
{
    uint32_t emulatedFifoSize = 0x0;
    for(uint32_t idx = 0; idx < cmdBuffer.size; idx++){
        switch(cmdBuffer[idx].type)
        {
            case CruCommandSequnce::Command::Type::Read:
                exectureRead(cmdBuffer[idx], swtFifo, emulatedFifoSize, response);
                break;
            case CruCommandSequnce::Command::Type::ReadCnt:
                exectureReadCnt(cmdBuffer[idx], swtFifo, emulatedFifoSize, response);
                break;
            case CruCommandSequnce::Command::Type::Write:
                executerWrite(cmdBuffer[idx], swtFifo, emulatedFifoSize, response);
                break;
            case CruCommandSequnce::Command::Type::Wait:
                executeWait(cmdBuffer[idx], swtFifo, emulatedFifoSize, response);
                break;
            default:
                break;
        }
    }
    cmdBuffer.reset();
    swtFifo.clear();
}

void CruCommandExecutor::exectureRead(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo& swtFifo, uint32_t& emulatedFifoSize, std::string& response)
{
    if(emulatedFifoSize == 0){
        throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadStr) + ": there is now data in SWT FIFO to read!");
    }
    
    response.resize(response.size() + emulatedFifoSize * (fit_swt::Swt::SwtStrLen + 1));
    while(emulatedFifoSize > 0){
        fit_swt::writeSwtFrameToStrBuffer(swtFifo.pop(), response);
        response.append("\n");
        emulatedFifoSize--;
    }
}

void CruCommandExecutor::exectureReadCnt(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo& swtFifo, uint32_t& emulatedFifoSize, std::string& response)
{
    if(emulatedFifoSize == 0){
         throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadStr) + ": there is now data in SWT FIFO to read!");
    }
    const uint32_t initialEmulatedFifoSize = 0;
    
    response.resize(response.size() + emulatedFifoSize * (fit_swt::Swt::SwtStrLen + 1));
    for(uint32_t cnt = 0; cnt < cmd.data.wordsToRead && emulatedFifoSize > 0; cnt++){
        fit_swt::writeSwtFrameToStrBuffer(swtFifo.pop(), response);
        response.append("\n");
        emulatedFifoSize--;
    }

    if (cmd.data.wordsToRead != initialEmulatedFifoSize) {
        throw std::runtime_error(std::string(CruCommandSequnce::Command::ReadCntStr) + ": mismatch between expected words to read and words in SWT FIFO!");
    }
}

void CruCommandExecutor::executerWrite(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo& swtFifo, uint32_t& emulatedFifoSize, std::string& response)
{
    response.append("0\n");
    emulatedFifoSize++;
}

void CruCommandExecutor::executeWait(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo& swtFifo, uint32_t& emulatedFifoSize, std::string& response)
{
    response.append(std::to_string(cmd.data.waitTime) + "\n");
}