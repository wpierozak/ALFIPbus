#pragma once
#include "SwtLink.h"
#include "CruCommandBuffer.h"

class CruCommandExecutor
{
public:
    static void execute(CruCommandBuffer& cmdBuffer, fit_swt::SwtFifo& swtFifo, std::string& response, uint32_t initialEmulatedFifoSize = 0);
private:
    static void exectureRead(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo& swtFifo, uint32_t& emulatedFifoSize, std::string& response);
    static void exectureReadCnt(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo& swtFifo, uint32_t& emulatedFifoSize, std::string& response);
    static void executerWrite(CruCommandSequnce::Command& cmd, fit_swt::SwtFifo& swtFifo, uint32_t& emulatedFifoSize, std::string& response);
    static void executeWait(CruCommandSequnce::Command& cmdB, fit_swt::SwtFifo& swtFifo, uint32_t& emulatedFifoSize, std::string& response);
};