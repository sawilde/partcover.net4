#pragma once

class DriverLog;

typedef unsigned char       BYTE;
typedef unsigned char       *LPBYTE;
typedef const unsigned char *LPCBYTE;
typedef signed char         SBYTE;
typedef signed short        SWORD;
typedef signed int          SDWORD;
typedef unsigned long long  QWORD;

namespace ILHelpers {

    //////////////////////////////////////////////////////////////////////////
    // Read & write stuff
    
    template<typename value_type> value_type Read(LPCBYTE* buffer) {
        value_type value = *(value_type*)(*buffer);
        *buffer += sizeof(value_type);
        return value;
    }

    template<typename value_type> void Write(LPBYTE* buffer, value_type value) {
        *(value_type*)(*buffer) = value;
        *buffer += sizeof(value_type);
    }

    // Read & write stuff
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // ILop stuff

    //////////////////////////////////////////////////////////////////////////
    // ILop structures

    enum InlineParameter {
        eInlineNull  = 0,
        eInlineByte  = 1,
        eInlineWord  = 2,
        eInlineDword = 4,
        eInlineQword = 8
    };

    enum ILflow {
        eNext,
        eBreak,
        eCall,
        eReturn,
        eBranch,
        eCondBranch,
        eThrow, 
        eMeta
    };

    enum ILopCode {
        #define OPDEF(name, str, decs, incs, args, optp, stdlen, stdop1, stdop2, flow) name = (stdop1 << 16) | stdop2,
        #include "opcode.def"
        #undef OPDEF
        CEE_NO_OP = -1
    };

    typedef std::vector<SDWORD> BranchOffsetTable;

    struct ILop {
        DWORD           pos;

        ILopCode        code;
        const TCHAR*    mnemonic;
        
        BYTE            stdop1;
        BYTE            stdop2;

        BYTE            stdlen : 3;
        InlineParameter inlineParameterType : 5;

        ILflow          flow;

        signed char stackDec;
        signed char stackInc;

        union {
            BYTE            opByte;
            WORD            opWord;
            DWORD           opDword;
            QWORD           opQword;
        } inlineParameter;

        BranchOffsetTable branchOffsets; // in bytes
        
        bool  IsBranch() const { return flow == eBranch || flow == eCondBranch; }
        bool  IsControl() const { return flow == eCall || flow == eReturn || flow == eThrow; }
        DWORD GetSize() const;

        const SDWORD& GetBranchOffset() const { return branchOffsets[0]; }
        SDWORD& GetBranchOffset() { return branchOffsets[0]; }
        void    SetBranchOffset(SDWORD value) { branchOffsets[0] = value; }
        DWORD   GetBranchStart() const { return pos + GetSize(); }

        void    InitializeBranches() { branchOffsets.push_back(0); }
        void    ClearBranches() { branchOffsets.clear(); }
    };

    typedef std::list<ILop>             ILopCodes;
    typedef ILopCodes::iterator         ILopPtr;
    typedef ILopCodes::const_iterator   ILopConstPtr;

    //////////////////////////////////////////////////////////////////////////
    // ILop functions

    ILop    FindILOpByCode(const ILopCode code);
    
    bool DisasmCode(LPCBYTE start, LPCBYTE end, ILopCodes *res);
    void EmitCode(LPBYTE start, const ILopCodes& ilops);
    void ReenumerateIlops(ILopCodes& ilops);

    ULONG GetCodeSize(const ILopCodes& ilops);

    // ILop stuff
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Sequence block stuff

    //////////////////////////////////////////////////////////////////////////
    // Sequence block structures

    struct ContinuousBlock {
        DWORD byteOffset;
    };
    typedef std::vector<ContinuousBlock> ContinuousBlocks;

    //////////////////////////////////////////////////////////////////////////
    // Sequence block functions

    ContinuousBlocks GetContinuousBlocks(const ILopCodes& ilops);

    // Sequence block stuff
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Instrumented code stuff

    //////////////////////////////////////////////////////////////////////////
    // Instrumented code structures

    struct ChangeBlock {
        DWORD original;
        ULONG originalSize;
        ILopCodes modifiedCode;

        ChangeBlock() {
            original = 0;
            originalSize = 0;
        }
    };
    typedef std::vector<ChangeBlock> ChangeBlocks;
    typedef ChangeBlocks::iterator ChangeBlockIt;

    //////////////////////////////////////////////////////////////////////////
    // Instrumented code functions

    void BuildCodeWithChanges(ILopCodes *result, const ILopCodes& originalIlops, const ChangeBlocks& changes, ILopCodes *fixups);

    // Instrumented code stuff
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Section staff

    unsigned EmitEHSection(LPBYTE *buffer, const COR_ILMETHOD_SECT* section, const ChangeBlocks& changes, const ILopCodes& fixups);
    unsigned EmitSection(LPBYTE *buffer, const COR_ILMETHOD_SECT* section);

    // Section staff
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Dump stuff

    void DumpIlop(DriverLog& log, const ILop& ilop, LPCTSTR prefix = _T(""));
    void DumpCode(DriverLog& log, const ILopCodes& ilops, LPCTSTR prefix = _T(""));
    void DumpContinuousBlocks(DriverLog& log, const ContinuousBlocks& blocks, LPCTSTR prefix = _T(""));
    void DumpChangeBlocks(DriverLog& log, const ChangeBlocks& blocks, LPCTSTR prefix = _T(""));
    // Dump stuff
    //////////////////////////////////////////////////////////////////////////
}
