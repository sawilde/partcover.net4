#include "StdAfx.h"
#include "corhdr.h"
#include "corhlpr.h"
#include "il_instrumentedbody.h"
#include "logging.h"
#include "helpers.h"

#include "corhlpr.cpp"

#pragma warning(push)
#pragma warning(disable : 4312)
#pragma warning(disable : 4311)

//#define DUMP_ORIGINAL_CODE
//#define DUMP_CONTINUOUS_BLOCKS
//#define DUMP_CHANGED_BLOCK
//#define DUMP_NEW_CODE

//#undef METHOD_INNER
//#define METHOD_INNER eNull

using namespace ILHelpers;

const DWORD MaxCounterValue = 0x1869F;
LPCTSTR SLogPrefix = _T("      ");
LPCTSTR LogPrefix = _T("        ");

InstrumentedILBody::InstrumentedILBody(LPCBYTE body, ULONG bodySize, ILHelpers::Allocator& allocator) 
	: m_decoder((COR_ILMETHOD*) body), m_parsed(false), m_allocator(allocator)
{
    m_newBody = 0;
    m_newBodySize = 0;

    ASSERT_SIZEOF(SBYTE, BYTE);
    ASSERT_SIZEOF(SWORD, WORD);
    ASSERT_SIZEOF(SDWORD, DWORD);

    DriverLog& log = DriverLog::get();

#ifdef DUMP_ORIGINAL_CODE
    log.WriteLine(_T("%scode: %s, header %d bytes, code length %d bytes, stack %d, %d eh-sections, additional sections - %s"), 
        SLogPrefix,
        m_decoder.IsFat() ? _T("FAT") : _T("TINY"), 
        m_decoder.Size * 4, 
        m_decoder.CodeSize, 
        m_decoder.GetMaxStack(),
        m_decoder.EHCount(),
        m_decoder.Sect == 0 ? _T("no") : _T("yes"));
#endif

    ParseBody(m_decoder.Code, m_decoder.Code + m_decoder.CodeSize);
}

InstrumentedILBody::~InstrumentedILBody(void) {
    delete[] m_newBody;
}

void InstrumentedILBody::ParseBody(LPCBYTE ilStart, LPCBYTE ilEnd) {
    m_parsed = ILHelpers::DisasmCode(ilStart, ilEnd, &m_originalIlops);
#ifdef DUMP_ORIGINAL_CODE
    DumpCode(DriverLog::get(), m_originalIlops, LogPrefix);
#endif
}

struct InstrumentedCodeInserter {
    ChangeBlocks& changes;
    InstrumentedBlocks& counters;
	ILHelpers::Allocator& m_allocator;

    InstrumentedCodeInserter(ChangeBlocks& _changes, InstrumentedBlocks& _counters, ILHelpers::Allocator& allocator) 
		: changes(_changes), counters(_counters), m_allocator(allocator) {}

    void operator() (const ContinuousBlock& block) {
        LOGINFO2(DUMP_INSTRUMENTATION, "%screate instrumented block at %8X", SLogPrefix, block.byteOffset);
        CreateBlockFromPoint(block.byteOffset);
    }

    void operator() (const ULONG32& point) {
        LOGINFO2(DUMP_INSTRUMENTATION, "%screate instrumented block at %8X", SLogPrefix, point);
        CreateBlockFromPoint(point);
    }

    void CreateBlockFromPoint(ULONG32 point) {
        ChangeBlock change;
        change.original     = point;
        change.originalSize = 0;
        ILopCodes& modified = change.modifiedCode;

        LOGINFO1(DUMP_INSTRUMENTATION, "%sallocate counter", LogPrefix);
        InstrumentedBlock iBlock;
		iBlock.counter = m_allocator.NewDword();
        iBlock.maxCounter = 999999;
        iBlock.position = change.original;

        ASSERT_SIZEOF(LPDWORD, DWORD);

        ILop ldc_i4_counter = ILHelpers::FindILOpByCode(CEE_LDC_I4);
        ldc_i4_counter.inlineParameter.opDword = (DWORD)iBlock.counter;
        ILop ldc_i4_maxCounter = ldc_i4_counter;
        ldc_i4_maxCounter.inlineParameter.opDword = iBlock.maxCounter;
        ILop branch = ILHelpers::FindILOpByCode(CEE_BEQ_S);
        branch.SetBranchOffset(10);

        LOGINFO1(DUMP_INSTRUMENTATION, "%sallocate counter code", LogPrefix);

        modified.push_back( ldc_i4_counter );
        modified.push_back( ILHelpers::FindILOpByCode(CEE_LDIND_I4) );
        modified.push_back( ldc_i4_maxCounter );
        modified.push_back( branch );

        modified.push_back( ldc_i4_counter );
        modified.push_back( ILHelpers::FindILOpByCode(CEE_DUP) );
        modified.push_back( ILHelpers::FindILOpByCode(CEE_LDIND_I4) );
        modified.push_back( ILHelpers::FindILOpByCode(CEE_LDC_I4_1) );
        modified.push_back( ILHelpers::FindILOpByCode(CEE_ADD) );
        modified.push_back( ILHelpers::FindILOpByCode(CEE_STIND_I4) );

        LOGINFO1(DUMP_INSTRUMENTATION, "%sstore change", LogPrefix);
        changes.push_back( change );
        LOGINFO1(DUMP_INSTRUMENTATION, "%sstore counter", LogPrefix);
        counters.push_back( iBlock );
        LOGINFO1(DUMP_INSTRUMENTATION, "%smove to next", LogPrefix);
    }
};

void InstrumentedILBody::CreateSequenceCountersFromCode() {
    LOGINFO1(DUMP_INSTRUMENTATION, "%sget continuous blocks", SLogPrefix);
    ContinuousBlocks blocks = GetContinuousBlocks(m_originalIlops);

#ifdef DUMP_CONTINUOUS_BLOCKS
    DumpContinuousBlocks(DriverLog::get(), blocks, LogPrefix);
#endif 

    //make insert for modified
    std::for_each(blocks.begin(), blocks.end(), 
		InstrumentedCodeInserter(m_counterBlocks, m_instrumentedBlocks, m_allocator));

    SetPositionToInstrumentedBlocks();
}

void InstrumentedILBody::CreateSequenceCounters(ULONG32 pointsCount, ULONG32* points)
{
    //make insert for modified
    std::for_each(points, points + pointsCount, 
		InstrumentedCodeInserter(m_counterBlocks, m_instrumentedBlocks, m_allocator));

    SetPositionToInstrumentedBlocks();
}

void InstrumentedILBody::SetPositionToInstrumentedBlocks() {
    if (m_instrumentedBlocks.size() > 0) {
        LOGINFO1(DUMP_INSTRUMENTATION, "%sset position to instrumented blocks", SLogPrefix);
        m_instrumentedBlocks.back().length = m_decoder.CodeSize - m_instrumentedBlocks.back().position;
        InstrumentedBlocks::iterator ins_it = m_instrumentedBlocks.begin();
        while(ins_it != m_instrumentedBlocks.end()) {
            InstrumentedBlocks::iterator block_it = ins_it++;
            if (ins_it == m_instrumentedBlocks.end())
                break;
            block_it->length = ins_it->position - block_it->position;
        }
    }
}

void InstrumentedILBody::DumpNewBody() {
    if (!m_newBody || !DriverLog::get().CanWrite(DUMP_INSTRUMENTATION))
        return;

    DriverLog& log = DriverLog::get();

    COR_ILMETHOD_DECODER decoder((COR_ILMETHOD*)m_newBody);
    log.WriteLine(_T("%snew code: %s, header %d bytes, code length %d bytes, stack %d, %d eh-sections, additional sections - %s"), 
        SLogPrefix,
        decoder.IsFat() ? _T("FAT") : _T("TINY"), 
        decoder.Size * 4, 
        decoder.CodeSize, 
        decoder.GetMaxStack(),
        decoder.EHCount(),
        decoder.Sect == 0 ? _T("no") : _T("yes"));

    ILopCodes newBody;
    ILHelpers::DisasmCode(decoder.Code, decoder.Code + decoder.CodeSize, &newBody);
    DumpCode(log, newBody, LogPrefix);
}

void InstrumentedILBody::ConstructNewBody() {
    if (m_newBody)
        return;

    DriverLog& log = DriverLog::get();
    //build code with fixies
    LOGINFO1(DUMP_INSTRUMENTATION, "%sbuild new code", SLogPrefix);
    BuildCodeWithChanges(&m_newIlops, m_originalIlops, m_counterBlocks, &m_modifiedBranches);

    LOGINFO1(DUMP_INSTRUMENTATION, "%scalc new stack size", SLogPrefix);
    ULONG stackSize = m_decoder.GetMaxStack() + 4;

    LOGINFO1(DUMP_INSTRUMENTATION, "%scalc new code size", SLogPrefix);
    ULONG codeSize  = ILHelpers::GetCodeSize(m_newIlops);

    COR_ILMETHOD_FAT method;
    memset(&method, 0, sizeof COR_ILMETHOD_FAT);
    memcpy(&method, &m_decoder, sizeof COR_ILMETHOD_FAT);
    method.CodeSize = codeSize;
    method.MaxStack = stackSize;

    LOGINFO1(DUMP_INSTRUMENTATION, "%scalc new header size", SLogPrefix);
    unsigned headerSize = COR_ILMETHOD::Size(&method, !!(m_decoder.Flags & CorILMethod_MoreSects));

    unsigned sectSize = 0;
    LOGINFO1(DUMP_INSTRUMENTATION, "%scalc new eh-sections size", SLogPrefix);
    const COR_ILMETHOD_SECT* section = m_decoder.EH;
    while(section != 0) {
        unsigned sectionSize = ILHelpers::EmitEHSection(0, section, m_counterBlocks, m_modifiedBranches);
        LOGINFO2(DUMP_INSTRUMENTATION, "%ssection (%d bytes)", LogPrefix, sectionSize);
        sectSize += sectionSize;
        section = section->Next();
    }

    LOGINFO1(DUMP_INSTRUMENTATION, "%scalc new sections size", SLogPrefix);
    section = m_decoder.Sect;
    while(section != 0) {
        unsigned sectionSize = ILHelpers::EmitSection(0, section);
        LOGINFO2(DUMP_INSTRUMENTATION, "%ssection (%d bytes)", LogPrefix, sectionSize);
        sectSize += sectionSize;
        section = section->Next();
    }

    m_newBodySize = headerSize;
    m_newBodySize += (codeSize + 3) & ~3;
    m_newBodySize += sectSize;

    LOGINFO4(DUMP_INSTRUMENTATION, "%sconstruct new body. %d body length, %d code length, %d stack", SLogPrefix, m_newBodySize, method.CodeSize, method.MaxStack);
    m_newBody = new BYTE[m_newBodySize];
    memset(m_newBody, 0, m_newBodySize);

    LPBYTE buffer = m_newBody;

    // write all data
    LOGINFO2(DUMP_INSTRUMENTATION, "%semit new header (sizeof %d)", SLogPrefix, headerSize);
    COR_ILMETHOD::Emit(headerSize, &method, !!(m_decoder.Flags & CorILMethod_MoreSects), buffer);
    buffer += headerSize;

    LOGINFO2(DUMP_INSTRUMENTATION, "%semit new code (sizeof %d)", SLogPrefix, codeSize);
    ILHelpers::EmitCode(buffer, m_newIlops);
    buffer += (codeSize + 3) & ~3;

    for(section = m_decoder.EH; section != 0; section = section->Next())
        EmitEHSection(&buffer, section, m_counterBlocks, m_modifiedBranches);

    for(section = m_decoder.Sect; section != 0; section = section->Next())
        EmitSection(&buffer, section);
}

#pragma warning(pop)
