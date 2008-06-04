#pragma once

#include "ILhelpers.h"

class DriverLog;

struct InstrumentedBlock {
    LPDWORD counter;
    DWORD maxCounter;

    DWORD position;
    ULONG length;

    ULONG32 fileId;
    ULONG32 startLine;
    ULONG32 startColumn;
    ULONG32 endLine;
    ULONG32 endColumn;

    InstrumentedBlock() {
        LPDWORD counter = 0;
        DWORD maxCounter = 0;
        DWORD position = 0;
        ULONG length = 0;
        fileId = startLine = startColumn = endLine = endColumn = 0;
    }
};

typedef std::vector<InstrumentedBlock> InstrumentedBlocks;

class InstrumentedILBody
{
    COR_ILMETHOD_DECODER m_decoder;
    bool                 m_parsed;

    ILHelpers::ILopCodes m_originalIlops;
    ILHelpers::ILopCodes m_newIlops;

    ILHelpers::ChangeBlocks m_counterBlocks;

    ILHelpers::ILopCodes m_modifiedBranches;

    void ParseBody(LPCBYTE ilStart, LPCBYTE ilEnd);

    InstrumentedBlocks m_instrumentedBlocks;

    LPBYTE m_newBody;
    ULONG  m_newBodySize;

public:

    InstrumentedILBody(LPCBYTE body, ULONG bodySize);
    ~InstrumentedILBody(void);

    void ConstructNewBody();
    void DumpNewBody();

    void CreateSequenceCountersFromCode();
    void CreateSequenceCounters(ULONG32 pointsCount, ULONG32* points);

    bool IsBodyParsed() const { return m_parsed; }

    LPCBYTE GetInstrumentedBody() const { return m_newBody; }
    ULONG   GetInstrumentedBodySize() const { return m_newBodySize; }

    InstrumentedBlocks& GetInstrumentedBlocks() { return m_instrumentedBlocks; }

private:

    void SetPositionToInstrumentedBlocks();
};
