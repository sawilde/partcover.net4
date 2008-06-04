#include "StdAfx.h"
#include "interface.h"
#include "MessageCenter.h"
#include "coveragegatherer.h"

CoverageGatherer::CoverageGatherer(void)
{
}

CoverageGatherer::~CoverageGatherer(void)
{
}

void CoverageGatherer::SendResults(MessageCenter& center)
{
    const char* coverageHeader = "<coverage/>";

    Message message;
    message.code = eResult;
    message.value.push_back(eCoverageGathererResult);
    message.value.insert(message.value.end(), coverageHeader, coverageHeader + strlen(coverageHeader) + 1);

    ATLTRACE("CoverageGatherer::SendResults - send eResult");
    center.SendOption(message);
}

bool CoverageGatherer::ReceiveResults(Message& message) {
    if (message.value.size() == 0 || message.value.front() != eCoverageGathererResult)
        return false;
    return true;
}
