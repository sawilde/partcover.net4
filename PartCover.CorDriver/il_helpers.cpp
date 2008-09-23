#include "StdAfx.h"
#include "corhdr.h"
#include "corhlpr.h"
#include "il_instrumentedbody.h"
#include "il_helpers.h"
#include "logging.h"

//#define DUMP_FIXUP_BRACHES

//#undef METHOD_INNER
//#define METHOD_INNER eNull

namespace ILHelpers {

#define IsSmallBranch(ilop) ((ilop).IsBranch() && NULL != _tcsstr((ilop).mnemonic, _T(".s")))
#define IsBigOffset(offset) ((SDWORD)(offset) < (SDWORD)(SBYTE)MINCHAR || (SDWORD)(offset) > (SDWORD)(SBYTE)MAXCHAR)

    DWORD ILop::GetSize() const {
        return (code == CEE_SWITCH)
            ? stdlen + inlineParameterType + inlineParameter.opDword * sizeof(SDWORD)
            : stdlen + inlineParameterType;
    }

#define NEXT                eNext
#define META                eMeta
#define THROW               eThrow
#define RETURN              eReturn
#define BRANCH              eBranch
#define CALL                eCall
#define BREAK               eBreak
#define COND_BRANCH         eCondBranch

#define InlineNone           eInlineNull
#define ShortInlineVar       eInlineByte
#define ShortInlineI         eInlineByte
#define InlineI              eInlineDword
#define InlineI8             eInlineQword
#define ShortInlineR         eInlineDword
#define InlineR              eInlineQword
#define InlineMethod         eInlineDword
#define InlineSig            eInlineDword
#define InlineBrTarget       eInlineDword
#define InlineType           eInlineDword
#define InlineVar            eInlineWord
#define ShortInlineBrTarget  eInlineByte
#define InlineTok            eInlineDword
#define InlineField          eInlineDword
#define InlineSwitch         eInlineDword
#define InlineString         eInlineDword

#define Pop0 0
#define Push0 0

#define Pop1 -1
#define PopI -1
#define PopI8 -1
#define PopR4 -1
#define PopR8 -1
#define PopRef -1
#define VarPop -1

#define Push1 1
#define PushI 1
#define PushI8 1
#define PushR4 1
#define PushR8 1
#define PushRef 1
#define VarPush 1

    typedef stdext::hash_map<WORD, ILop> ILopMap;

    const DWORD IlopDefaultPos = -1;

    struct ILopMapContainerStruct {
        ILopMap m_map;
        ILop    m_noop;

        ILopMapContainerStruct() {
            m_noop.code = CEE_NO_OP;
#define OPDEF(name, str, decs, incs, args, optp, stdlen, stdop1, stdop2, flow) \
            { \
            ILop ilop = {IlopDefaultPos, name, _T(str), stdop1, stdop2, stdlen, args, flow, decs, incs}; \
            ilop.InitializeBranches(); \
            m_map[MAKEWORD(stdop1,stdop2)] = ilop;  \
            }
#include "opcode.def"
#undef OPDEF
        }

        ILop FindILOpByCode(BYTE stdop1, BYTE stdop2) {
            ILopMap::iterator mapIt = m_map.find(MAKEWORD(stdop1,stdop2));
            if (mapIt != m_map.end()) return mapIt->second;
            return m_noop;
        }

        ILop FindILOpByCode(const ILopCode code) {
            ILopMap::iterator mapIt = m_map.begin();
            while(mapIt != m_map.end()) {
                ILop& ilop = (mapIt++)->second;
                if (ilop.code == code) return ilop;
            }
            return m_noop;
        }

        ILop FindILOpByMnemonic(const TCHAR* mnemonic) {
            ILopMap::iterator mapIt = m_map.begin();
            while(mapIt != m_map.end()) {
                ILop& ilop = (mapIt++)->second;
                if (_tcsicmp(ilop.mnemonic, mnemonic) == 0) return ilop;
            }
            return m_noop;
        }

    } ILopMapContainer;

    ILop FindILOpByCode(const ILopCode code) {
        return ILopMapContainer.FindILOpByCode(code);
    }

    ILop GetFullForm(const ILop& ilop) {
        TCHAR buffer[64] = {0};
        size_t mnsize = _tcsclen(ilop.mnemonic) - 2;
        _tcsncpy_s(buffer, 64, ilop.mnemonic, mnsize);
        buffer[mnsize] = 0;
        return ILopMapContainer.FindILOpByMnemonic(buffer);
    }

    ULONG GetCodeSize(const ILopCodes& ilops) {
        ULONG size = 0;
        for(ILopConstPtr ilit = ilops.begin(); ilit != ilops.end(); ++ilit)
            size += ilit->GetSize();
        return size;
    }

    void PopInstructionInlineArgument(LPCBYTE* buffer, ILop* op) {
        switch(op->inlineParameterType) {
            case eInlineNull: return;
            case eInlineByte:  op->inlineParameter.opByte = Read<BYTE>(buffer); break;
            case eInlineWord:  op->inlineParameter.opWord = Read<WORD>(buffer); break;
            case eInlineDword: op->inlineParameter.opDword = Read<DWORD>(buffer); break;
            case eInlineQword: op->inlineParameter.opQword = Read<QWORD>(buffer); return;
        }

        if (CEE_SWITCH == op->code) {
            op->ClearBranches();
            DWORD switchTableSize = op->inlineParameter.opDword;
            while(switchTableSize-- != 0)
                op->branchOffsets.push_back(Read<SDWORD>(buffer));
        } else if (op->IsBranch()) {
            switch(op->inlineParameterType) {
                case eInlineByte:  op->SetBranchOffset((SDWORD)(SBYTE)op->inlineParameter.opByte); break;
                case eInlineWord:  op->SetBranchOffset((SDWORD)(SWORD)op->inlineParameter.opWord); break;
                case eInlineDword: op->SetBranchOffset((SDWORD)op->inlineParameter.opDword); break;
            }
        }
    }

    ILop PopFrontIlop(LPCBYTE* buffer) {
        BYTE opCode2 = Read<BYTE>(buffer);
        BYTE opCode1 = (opCode2 == STP1) ? STP1 : REFPRE;
        if (opCode1 == STP1) 
            opCode2 = Read<BYTE>(buffer);

        ILop ilop = ILopMapContainer.FindILOpByCode(opCode1, opCode2);
        PopInstructionInlineArgument(buffer, &ilop);
        return ilop;
    }

    bool DisasmCode(LPCBYTE start, LPCBYTE end, ILopCodes *res) {
        while(start < end) {
            ILop newIlop = PopFrontIlop(&start);
            if (newIlop.code == ILHelpers::CEE_NO_OP)
                return false;
            res->push_back(newIlop);
        }
        ILHelpers::ReenumerateIlops(*res);
        return true;
    }

    void EmitIlop(LPBYTE* buffer, const ILop& ilop) {
        if (ilop.stdop1 == STP1) Write<BYTE>(buffer, ilop.stdop1);
        Write<BYTE>(buffer, ilop.stdop2);
    }

    void EmitIlopInlineArgument(LPBYTE* buffer, const ILop& op) {
        if (CEE_SWITCH == op.code) {
            Write<DWORD>(buffer, op.inlineParameter.opDword);
            for(BranchOffsetTable::const_iterator it = op.branchOffsets.begin(); it != op.branchOffsets.end(); ++it)
                Write<DWORD>(buffer, (DWORD) *it);
        } else if (op.IsBranch()) {
            switch(op.inlineParameterType) {
                case eInlineByte:  Write<BYTE>(buffer, (BYTE)(SBYTE)op.GetBranchOffset()); break;
                case eInlineWord:  Write<WORD>(buffer, (WORD)(SWORD)op.GetBranchOffset()); break;
                case eInlineDword: Write<DWORD>(buffer, (DWORD)op.GetBranchOffset()); break;
            }
        } else {
            switch(op.inlineParameterType) {
				case eInlineNull: return;
				case eInlineByte:  Write<BYTE>(buffer, op.inlineParameter.opByte); break;
				case eInlineWord:  Write<WORD>(buffer, op.inlineParameter.opWord); break;
				case eInlineDword: Write<DWORD>(buffer, op.inlineParameter.opDword); break;
				case eInlineQword: Write<QWORD>(buffer, op.inlineParameter.opQword); return;
            }
        }
    }

    void EmitCode(LPBYTE start, const ILopCodes& codes) {
        for(ILopConstPtr ilit = codes.begin(); ilit != codes.end(); ++ilit) {
            EmitIlop(&start, *ilit);
            EmitIlopInlineArgument(&start, *ilit);
        }
    }

    void ReenumerateIlops(ILopCodes& codes) {
        DWORD pos = 0;
        for(ILopPtr ilit = codes.begin(); ilit != codes.end(); ++ilit) {
            ilit->pos = pos;
            pos += ilit->GetSize();
        }
    }

    typedef std::set<DWORD>     BlockPoints;

    struct BlockPointGatherer {
        BlockPoints points;
        void operator () ( const ILop& ilop ) {
            if (!ilop.IsControl() && !ilop.IsBranch())
                return;
            DWORD offsetStart = ilop.pos + ilop.GetSize();
            if (ilop.IsBranch()) {
                if (ilop.code == CEE_SWITCH) {
                    for(BranchOffsetTable::const_iterator it = ilop.branchOffsets.begin(); it != ilop.branchOffsets.end(); ++it) {
                        LOGINFO1(METHOD_INNER, "    add block point %X", offsetStart + *it);
                        points.insert(offsetStart + *it);
                    }
                } else {
                    LOGINFO1(METHOD_INNER, "    add block point %X", offsetStart + ilop.GetBranchOffset());
                    points.insert(offsetStart + ilop.GetBranchOffset());
                }
            }
            LOGINFO1(METHOD_INNER, "    add block point %X", offsetStart);
            points.insert(offsetStart);
        }
    };

    struct ContinuousBlocksGatherer {
        ContinuousBlocks* blocks;
        ContinuousBlocksGatherer(ContinuousBlocks* _blocks) : blocks(_blocks) {}
        void operator () ( const DWORD& blockPoint ) {
            LOGINFO1(METHOD_INNER, "    add block at point %X", blockPoint);
            ContinuousBlock block = { blockPoint };
            blocks->push_back(block);
        }
    };

    ContinuousBlocks GetContinuousBlocks(const ILopCodes& ilopArray) {
        ContinuousBlocks result;
        LOGINFO(METHOD_INNER, "get continuous points");
        BlockPointGatherer points = std::for_each(ilopArray.begin(), ilopArray.end(), BlockPointGatherer());
        LOGINFO(METHOD_INNER, "  add first point");
        points.points.insert(ilopArray.begin()->pos);
        LOGINFO(METHOD_INNER, "  remove last point");
        points.points.erase(ilopArray.rbegin()->pos + ilopArray.rbegin()->GetSize());
        LOGINFO(METHOD_INNER, "  get continuous block form points");
        std::for_each(points.points.begin(), points.points.end(), ContinuousBlocksGatherer(&result));
        return result;
    }

    typedef std::vector<ILopPtr> ILopPtrArray;
    struct BranchILopTag {
        ILopPtr      ilop;
        ILopPtrArray branches;
    };
    typedef std::vector<BranchILopTag> BranchILopTags;

    BranchILopTags StoreBranches(ILopCodes& codes) {
        BranchILopTags result;
        for(ILopPtr ilit = codes.begin(); ilit != codes.end(); ++ilit) {
            if (ilit->IsBranch()) {
                BranchILopTag tag = { ilit };

                DWORD brStart = ilit->GetBranchStart();
                for(BranchOffsetTable::const_iterator sit = ilit->branchOffsets.begin(); sit != ilit->branchOffsets.end(); ++sit) {
                    SDWORD  offset = *sit;
                    ILopPtr ilit2 = ilit;
                    ilit2++;
                    if (offset >= 0) {
                        while(offset != 0) offset -= (ilit2++)->GetSize();
                    } else {
                        while(offset != 0) offset += (--ilit2)->GetSize();
                    }
                    tag.branches.push_back(ilit2);
                }

                result.push_back(tag);
            }
        }
        return result;
    }

    void ChangeBranchEnd(BranchILopTags& tags, const ILopPtr& oldBranchEnd, const ILopPtr& newBranchEnd) {
        for(BranchILopTags::iterator tagIt = tags.begin(); tagIt != tags.end(); ++tagIt) {
            for(ILopPtrArray::iterator endIt = tagIt->branches.begin(); endIt != tagIt->branches.end(); ++endIt) {
                if (*endIt == oldBranchEnd) *endIt = newBranchEnd;
            }
        }
    }

    bool lt(ILopCodes& codes, ILopPtr startIlop, ILopPtr endILop) {
        return std::distance(codes.begin(), startIlop) < std::distance(codes.begin(), endILop);
    }

    bool le(ILopCodes& codes, ILopPtr startIlop, ILopPtr endILop) {
        return std::distance(codes.begin(), startIlop) <= std::distance(codes.begin(), endILop);
    }

    SDWORD GetAbsoluteCodeOffset(ILopCodes& codes, ILopPtr startIlop, ILopPtr endILop) {
        SDWORD offset = 0;
        bool forward = le(codes, startIlop, endILop);
        startIlop++;
        while(!forward && startIlop != endILop) 
            offset -= (--startIlop)->GetSize();
        while(forward && startIlop != endILop) 
            offset += (startIlop++)->GetSize();
        return offset;
    }

    struct BranchCorrectTag {
        ILopPtr oldBranch;
        ILop    newBranch;
    };
    typedef std::vector<BranchCorrectTag> BranchCorrectTags;

    void BuildCodeWithChanges(ILopCodes *newIlops, const ILopCodes& originalIlops, const ChangeBlocks& changes, ILopCodes *op_fixups) {
        BranchCorrectTags fixups;

		LOGINFO(METHOD_INNER, "insert original code into new code");
        *newIlops = originalIlops;

        LOGINFO(METHOD_INNER, "store branches");
        BranchILopTags branches = StoreBranches(*newIlops);

        LOGINFO(METHOD_INNER, "insert new blocks");
        ChangeBlocks::const_iterator changeIt = changes.begin();
        for(ILopPtr ilit = newIlops->begin(); changeIt != changes.end() && ilit != newIlops->end(); ++ilit) {
            if (ilit->pos == changeIt->original) {
                ILopPtr::difference_type ilitOffset = std::distance(newIlops->begin(), ilit);

                newIlops->insert(ilit, changeIt->modifiedCode.begin(), changeIt->modifiedCode.end());
                changeIt++;

                LOGINFO(METHOD_INNER, "repoint old branches to seq-block instead real code");
                // we need to repoint old branches to seq-block instead real code
                ILopPtr newBranchEnd = newIlops->begin();
                std::advance(newBranchEnd, ilitOffset);
                ChangeBranchEnd(branches, ilit, newBranchEnd);
            }
        }
        
        LOGINFO(METHOD_INNER, "find fixup branches");
        for(BranchILopTags::iterator bitIt = branches.begin(); bitIt != branches.end(); ++bitIt) {
            if (!IsSmallBranch(*bitIt->ilop)) continue;
            ILopPtr& endIlop = bitIt->branches[0];
            SDWORD branch = GetAbsoluteCodeOffset(*newIlops, bitIt->ilop, endIlop);
            for(BranchILopTags::iterator innerBitIt = branches.begin(); innerBitIt != branches.end(); ++innerBitIt) {
                if (innerBitIt->ilop == bitIt->ilop) continue;

                if (branch >= 0 && le(*newIlops, bitIt->ilop, innerBitIt->ilop) && lt(*newIlops, innerBitIt->ilop, endIlop))
                    branch += 5;
                else if (branch < 0 && le(*newIlops, endIlop, innerBitIt->ilop) && lt(*newIlops, innerBitIt->ilop, bitIt->ilop))
                    branch -= 5;
            }
            if (IsBigOffset(branch)) {
                BranchCorrectTag tag = { bitIt->ilop, GetFullForm(*bitIt->ilop) };
                fixups.push_back(tag);
                op_fixups->push_back(*bitIt->ilop);
            }
        }

        LOGINFO(METHOD_INNER, "fixup branches");
        for(BranchCorrectTags::iterator it = fixups.begin(); it != fixups.end(); ++it) 
            *it->oldBranch = it->newBranch;

        LOGINFO(METHOD_INNER, "restore branches");
        for(BranchILopTags::iterator bitIt = branches.begin(); bitIt != branches.end(); ++bitIt) {
            for(size_t i = 0; i < bitIt->branches.size(); ++i) {
                bitIt->ilop->branchOffsets[i] = GetAbsoluteCodeOffset(*newIlops, bitIt->ilop, bitIt->branches[i]);
            }
        }

		LOGINFO(METHOD_INNER, "ilcodes reenumerate");
		ReenumerateIlops(*newIlops);

#ifdef DUMP_FIXUP_BRACHES
        DumpChangeBlocks(DriverLog::get(), *fixups, "CO blocks ");
#endif 
    }

    struct OffsetModificator {
        DWORD change;
        OffsetModificator(DWORD _offset) : originalOffset(_offset), change(0) {}

        void operator () (const ChangeBlock& block) {
            if (block.original < originalOffset)
                change += GetCodeSize(block.modifiedCode) - block.originalSize;
        }
        void operator () (const ILop& ilop) {
            if (ilop.pos < originalOffset)
                change += GetFullForm(ilop).GetSize() - ilop.GetSize();
        }
    private:
        DWORD originalOffset;
    };

    DWORD UpdateOffset(DWORD value, const ChangeBlocks& changes, const ILopCodes& fixups) {
        return value 
            + std::for_each(changes.begin(), changes.end(), OffsetModificator(value)).change
            + std::for_each(fixups.begin(), fixups.end(), OffsetModificator(value)).change;
    }

    unsigned EmitEHSection(LPBYTE *buffer, const COR_ILMETHOD_SECT* section, const ChangeBlocks& changes, const ILopCodes& fixups) {
        LOGINFO(METHOD_INNER, "        emit eh-section");
        DriverLog& log = DriverLog::get();

        const COR_ILMETHOD_SECT_EH* ehSection = (const COR_ILMETHOD_SECT_EH*)section;

        IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT *ehClauses = new IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT[ehSection->EHCount()];
        LOGINFO1(METHOD_INNER, "          copy %d clauses", ehSection->EHCount());
        for(unsigned i = 0; i < ehSection->EHCount(); ++i)
            memcpy(ehClauses + i, ehSection->EHClause(i, ehClauses + i), sizeof IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT);

        for(unsigned i = 0; i < ehSection->EHCount(); ++i) {
            IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT& ehClause = ehClauses[i];
            LOGINFO(METHOD_INNER, "          emit new eh-clause");
            LOGINFO4(METHOD_INNER, "            old values: try %X, length %X, handler %X, length %X", 
                ehClause.TryOffset, ehClause.TryLength, ehClause.HandlerOffset, ehClause.HandlerLength);

            DWORD tryOffset, tryLOffset, handlerOffset, handlerLOffset;

            tryOffset = UpdateOffset(ehClause.TryOffset, changes, fixups);
            tryLOffset = UpdateOffset(ehClause.TryOffset + ehClause.TryLength, changes, fixups);
            handlerOffset = UpdateOffset(ehClause.HandlerOffset, changes, fixups);
            handlerLOffset = UpdateOffset(ehClause.HandlerOffset + ehClause.HandlerLength, changes, fixups);

            ehClause.TryOffset = tryOffset;
            ehClause.TryLength = tryLOffset - tryOffset;
            ehClause.HandlerOffset = handlerOffset;
            ehClause.HandlerLength = handlerLOffset - handlerOffset;

            LOGINFO4(METHOD_INNER, "            new values: try %X, length %X, handler %X, length %X", 
                ehClause.TryOffset, ehClause.TryLength, ehClause.HandlerOffset, ehClause.HandlerLength);
        }

        unsigned size = COR_ILMETHOD_SECT_EH::Size(ehSection->EHCount(), ehClauses);

        if (buffer != 0) {
            COR_ILMETHOD_SECT_EH::Emit(size, ehSection->EHCount(), ehClauses, ehSection->More(), *buffer);
        }

        delete[] ehClauses;

        size += (size + 3) & ~3;
        if (buffer != 0)
            *buffer += size;
        return size;
    }

    unsigned EmitSection(LPBYTE *buffer, const COR_ILMETHOD_SECT* section) {
        unsigned size = (unsigned)(section->NextLoc() - section);
        if (buffer != 0 ) {
            memcpy(*buffer, section, size);
            *buffer += size;
        }
        return size;
    }

    //////////////////////////////////////////////////////////////////////////
    // Dump stuff

    void DumpIlop(DriverLog& log, const ILop& op, const char* prefix) {
        if (!log.CanWrite(DUMP_METHOD))
            return;

        DWORD pos = op.pos;
        if (!op.IsBranch()) {
            switch(op.inlineParameterType) {
            case eInlineNull:  LOGINFO3(DUMP_METHOD, "%s %8X %s", prefix, pos, op.mnemonic); break;
            case eInlineByte:  LOGINFO4(DUMP_METHOD, "%s %8X %s %1X", prefix, pos, op.mnemonic, op.inlineParameter.opByte); break;
            case eInlineWord:  LOGINFO4(DUMP_METHOD, "%s %8X %s %2X", prefix, pos, op.mnemonic, op.inlineParameter.opWord); break;
            case eInlineDword: LOGINFO4(DUMP_METHOD, "%s %8X %s %4X", prefix, pos, op.mnemonic, op.inlineParameter.opDword); break;
            case eInlineQword: LOGINFO3(DUMP_METHOD, "%s %8X %s <QWORD>", prefix, pos, op.mnemonic); break;
            }
        } else if (CEE_SWITCH == op.code) {
            LOGINFO4(DUMP_METHOD, "%s %8X %s size %l", prefix, pos, op.mnemonic, op.branchOffsets.size());
            pos += op.stdlen + op.inlineParameterType;

            BranchOffsetTable::const_iterator switchIt = op.branchOffsets.begin();
            while(switchIt != op.branchOffsets.end()) {
                SDWORD offset = *switchIt++;
                LOGINFO4(DUMP_METHOD, "%s %8X -> (%4X) %8X ", prefix, pos, offset, op.pos + op.GetSize() + offset); 
                pos += sizeof( BranchOffsetTable::value_type );
            }
        } else {
            LOGINFO5(DUMP_METHOD, "%s %8X %s %8X (%ld)", prefix, pos, op.mnemonic, pos + op.GetSize() + op.GetBranchOffset(), op.GetBranchOffset());
        }
    }

    void DumpCode(DriverLog& log, const ILopCodes& ilops, const char* prefix) {
        for(ILopConstPtr ilit = ilops.begin(); ilit != ilops.end(); ++ilit)
            DumpIlop(log, *ilit, prefix);
    }


    void DumpContinuousBlocks(DriverLog& log, const ContinuousBlocks& blocks, const char* prefix) {
        if (!log.CanWrite(DUMP_METHOD))
            return;

        LOGINFO2(DUMP_METHOD, "%sContinuous blocks (%d items)", prefix, blocks.size());
        ContinuousBlocks::const_iterator blocksIt = blocks.begin();
        while(blocksIt != blocks.end()) {
            const ContinuousBlock& block = *blocksIt++;
            LOGINFO2(DUMP_METHOD, "%s  block byteoffset - %X", prefix, block.byteOffset);
        }
    }

    void DumpChangeBlocks(DriverLog& log, const ChangeBlocks& blocks, const char* prefix) {
        if (!log.CanWrite(DUMP_METHOD))
            return;

        LOGINFO2(DUMP_METHOD, "%sChange blocks (%d items)", prefix, blocks.size());
        ChangeBlocks::const_iterator blocksIt = blocks.begin();
        while(blocksIt != blocks.end()) {
            const ChangeBlock& block = *blocksIt++;
            LOGINFO2(DUMP_METHOD, "%s  block %d", prefix, std::distance(blocks.begin(), blocksIt));
            LOGINFO3(DUMP_METHOD, "%s    original code, starts at %X, %d bytes length", prefix, block.original, block.originalSize);
            LOGINFO2(DUMP_METHOD, "%s    modified code, %d bytes length", prefix, GetCodeSize(block.modifiedCode));
            DumpCode(log, block.modifiedCode, prefix);
        }
    }

    // Dump stuff
    //////////////////////////////////////////////////////////////////////////
}
