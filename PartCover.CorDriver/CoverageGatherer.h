#pragma once

class MessageCenter;

class CoverageGatherer : public IResultContainer
{
public:
    CoverageGatherer(void);
    ~CoverageGatherer(void);


    void SendResults(MessageCenter&);
    bool ReceiveResults(Message& message);
};
