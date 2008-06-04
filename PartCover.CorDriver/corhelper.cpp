#include "stdafx.h"
#include "interface.h"
#include "helpers.h"
#include "corhelper.h"

namespace CorHelper {
    std::wstring GetModuleName(ICorProfilerInfo* info, ModuleID module) {
        ULONG buffSize = 0;
        if (FAILED(info->GetModuleInfo(module, NULL, 0, &buffSize, NULL, NULL))) 
            return std::wstring();
        DynamicArray<WCHAR> buffer(buffSize + 1);
        if (FAILED(info->GetModuleInfo(module, NULL, buffSize + 1, &buffSize, buffer.data, NULL)))
            return std::wstring();
        return std::wstring(buffer);
    }

    std::wstring GetAssemblyName(ICorProfilerInfo* info, AssemblyID assembly) {
        ULONG bufferSize = 0;
        if (FAILED(info->GetAssemblyInfo(assembly, 0, &bufferSize, NULL, NULL, NULL))) 
            return std::wstring();
        DynamicArray<WCHAR> buffer(bufferSize + 1);
        if (FAILED(info->GetAssemblyInfo(assembly, bufferSize + 1, &bufferSize, buffer, NULL, NULL)))
            return std::wstring();
        return std::wstring(buffer);
    }

    std::wstring GetTypedefFullName(IMetaDataImport* mdImport, mdTypeDef typeDef, DWORD *p_typeDefFlags, LPCWSTR connectStr, const std::wstring& innerTypeDefName) {
        ULONG bufferSize = 0;
        DWORD typeDefFlags = 0;
        if(SUCCEEDED(mdImport->GetTypeDefProps(typeDef, NULL, 0, &bufferSize, &typeDefFlags, NULL))) {
            if (p_typeDefFlags != 0) *p_typeDefFlags = typeDefFlags;

            LPWSTR buffer = new WCHAR[bufferSize + 1];
            mdImport->GetTypeDefProps(typeDef, buffer, bufferSize+1, &bufferSize, NULL, NULL);
            std::wstring typedefName = buffer;
            delete[] buffer;
            if (innerTypeDefName.length() > 0)
                typedefName += std::wstring(connectStr) + innerTypeDefName;

            if (!IsTdNested(typeDefFlags)) 
                return typedefName;

            mdTypeDef enclosingTypeDef;
            if (SUCCEEDED(mdImport->GetNestedClassProps(typeDef, &enclosingTypeDef)))
                return GetTypedefFullName(mdImport, enclosingTypeDef, NULL, connectStr, typedefName);
        }
        return std::wstring();
    }

    std::wstring GetClassName(ICorProfilerInfo* info, ClassID classId) {
        ModuleID module;
        mdTypeDef typeDef;
        if (FAILED(info->GetClassIDInfo(classId, &module, &typeDef)))
            return std::wstring();

        CComPtr<IMetaDataImport> mdImport;
        if (FAILED(info->GetModuleMetaData(module, ofRead, IID_IMetaDataImport, (IUnknown**) &mdImport)))
            return std::wstring();

        ULONG bufferSize;
        if (FAILED(mdImport->GetTypeDefProps(typeDef, NULL, 0, &bufferSize, NULL, NULL)))
            return std::wstring();

        DynamicArray<WCHAR> buffer(bufferSize + 1);
        if (FAILED(mdImport->GetTypeDefProps(typeDef, buffer.data, bufferSize + 1, &bufferSize, NULL, NULL)))
            return std::wstring();

        return GetTypedefFullName(mdImport, typeDef, 0, L"+", buffer.data);
    }

    const wchar_t* StrCalling[] = {
        L"default", L"C", L"stdcall", L"thiscall", L"fastcall", L"vararg", L"field", L"localsig", L"property", L"unmanaged"
    };

    const wchar_t* MapElementType[] = 
    {
        L"End", L"Void", L"Boolean", L"Char", L"I1", L"UI1", L"I2", L"UI2", L"I4", L"UI4", L"I8", L"UI8", L"R4",
            L"R8", L"String", L"Ptr", L"ByRef", L"ValueClass", L"Class", L"CopyCtor", L"MDArray", L"GENArray", L"TypedByRef",
            L"VALUEARRAY", L"I", L"U", L"R", L"FNPTR", L"Object", L"SZArray", L"GENERICArray", L"CMOD_REQD", L"CMOD_OPT", L"INTERNAL",
    };

    std::wstring TypeRefName(IMetaDataImport* mdImport, mdTypeRef tr)
    {
        ULONG bufferSize;
        if(FAILED(mdImport->GetTypeRefProps(tr, NULL, NULL, 0, &bufferSize)))
            return std::wstring();
        DynamicArray<WCHAR> buffer(bufferSize + 1);
        if(FAILED(mdImport->GetTypeRefProps(tr, NULL, buffer.data, bufferSize + 1, &bufferSize)))
            return std::wstring();
        return buffer.data;
    }

    struct UncompressHelper {
        IMetaDataImport* import;
        void AddString(const std::wstring& str) { AddString(str.c_str()); }

        virtual void AddString(LPCWSTR str) = 0;
    };

    std::wstring GetTypeRefOrDefName(mdToken inToken, UncompressHelper& hlp)
    {
        if (!IsNilToken(inToken)) {
            switch(TypeFromToken(inToken)) {
                case mdtTypeDef: return GetTypedefFullName(hlp.import, (mdTypeDef) inToken, NULL, L".");
                case mdtTypeRef: return TypeRefName(hlp.import, (mdTypeRef) inToken);
                default: return L"[invalidReference]";
            }
        }
        return L"";
    }

    std::wstring GetTokenAssembly(mdToken inToken, UncompressHelper& hlp) {
        return L"";
    }

    ULONG UncompressCustomMod(PCCOR_SIGNATURE pSigBlob, UncompressHelper& hlp) {
		mdToken token;
		ULONG ulData;
        ULONG cb = CorSigUncompressData(pSigBlob, &ulData);
		switch(ulData) {
			case ELEMENT_TYPE_CMOD_OPT:
			case ELEMENT_TYPE_CMOD_REQD:
                hlp.AddString(L"const ");
				cb += CorSigUncompressToken(&pSigBlob[cb], &token);
				break;
			default:
				cb = 0;
		}
		return cb;
    }

    const LPCWSTR PrimitiveTypes[] = {
        L"end", L"void", L"boolean", L"char", L"byte", L"unsigned byte", L"short", L"unsigned short", L"int",
        L"unsigned int", L"long", L"unsigned long", L"float", L"double", L"string"
    };

    ULONG UncompressMethodDefOrRefSig(PCCOR_SIGNATURE pSigBlob, UncompressHelper& hlp);

    ULONG UncompressType(PCCOR_SIGNATURE pSigBlob, UncompressHelper& hlp) {
        ULONG cb = 0;
        ULONG ulData;
        cb += CorSigUncompressData(pSigBlob, &ulData);
        if (CorIsPrimitiveType((CorElementType)ulData)) {
            hlp.AddString(PrimitiveTypes[(CorElementType)ulData]);
            return cb;
        }
        mdToken token;
        ULONG rank, numSizes, numLoBounds;
        switch(ulData) {
            case ELEMENT_TYPE_VALUETYPE:
            case ELEMENT_TYPE_CLASS:
                cb += CorSigUncompressToken(&pSigBlob[cb], &token);
                //hlp.AddString(L"[");
                hlp.AddString(GetTokenAssembly(token, hlp));
                //hlp.AddString(L"]");
                hlp.AddString(GetTypeRefOrDefName(token, hlp));
                break;
            case ELEMENT_TYPE_OBJECT:
                hlp.AddString(L"object");
                break;
            case ELEMENT_TYPE_PTR:
                cb += UncompressType(&pSigBlob[cb], hlp);
                break;
            case ELEMENT_TYPE_SZARRAY:
                cb += UncompressType(&pSigBlob[cb], hlp);
                hlp.AddString(L"[]");
                break;
            case ELEMENT_TYPE_ARRAY: {
                cb += UncompressType(&pSigBlob[cb], hlp);
                cb += CorSigUncompressData(&pSigBlob[cb], &rank);

                cb += CorSigUncompressData(&pSigBlob[cb], &numSizes);
                DynamicArray<ULONG> sizes(numSizes);
                for(ULONG i = 0; i < numSizes; ++i)
                    cb += CorSigUncompressData(&pSigBlob[cb], &sizes[i]);

                cb += CorSigUncompressData(&pSigBlob[cb], &numLoBounds);
                DynamicArray<ULONG> loBounds(numLoBounds);
                for(ULONG i = 0; i < numLoBounds; ++i)
                    cb += CorSigUncompressData(&pSigBlob[cb], &loBounds[i]);
                break;
                                     }
            case ELEMENT_TYPE_FNPTR:
                cb += UncompressMethodDefOrRefSig(&pSigBlob[cb], hlp);
            default:
                __asm int 3;
        }
        return cb;
    }

    ULONG UncompressRetType(PCCOR_SIGNATURE pSigBlob, UncompressHelper& hlp) {
        ULONG cbTemp = 0;
        ULONG cb = 0;
        while((cbTemp = UncompressCustomMod(&pSigBlob[cb], hlp)) > 0)
            cb += cbTemp;
        ULONG ulData;
        cbTemp = CorSigUncompressData(&pSigBlob[cb], &ulData);
		switch(ulData) {
			case ELEMENT_TYPE_VOID: 
				hlp.AddString(L"void"); break;
			case ELEMENT_TYPE_TYPEDBYREF: 
				hlp.AddString(L"refany"); break;
			case ELEMENT_TYPE_BYREF: 
				hlp.AddString(L"ref ");
                cb += cbTemp;
			default: 
				cbTemp = UncompressType(&pSigBlob[cb], hlp);
		}
		return cb + cbTemp;
    }

    ULONG UncompressParam(PCCOR_SIGNATURE pSigBlob, UncompressHelper& hlp) {
        ULONG cbTemp = 0;
        ULONG cb = 0;
        while((cbTemp = UncompressCustomMod(&pSigBlob[cb], hlp)) > 0)
            cb += cbTemp;
        ULONG ulData;
        cbTemp = CorSigUncompressData(&pSigBlob[cb], &ulData);
        switch(ulData) {
            case ELEMENT_TYPE_TYPEDBYREF: 
                hlp.AddString(L"refany"); break;
            case ELEMENT_TYPE_BYREF: 
                hlp.AddString(L"ref ");
                cb += cbTemp;
            default: 
                cbTemp = UncompressType(&pSigBlob[cb], hlp);
        }
        return cb + cbTemp;
    }

    ULONG UncompressMethodDefOrRefSig(PCCOR_SIGNATURE pSigBlob, UncompressHelper& hlp) {
        ULONG cb = 0;
        ULONG ulData;
        cb += CorSigUncompressData(pSigBlob, &ulData);
        if (IMAGE_CEE_CS_CALLCONV_HASTHIS & ulData) {}
        if (IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS & ulData) {}
        if (IMAGE_CEE_CS_CALLCONV_VARARG & ulData) {}

        ULONG paramCount;
        cb += CorSigUncompressData(&pSigBlob[cb], &paramCount);

        cb += UncompressRetType(&pSigBlob[cb], hlp);
        hlp.AddString(L" (");
        while(paramCount-- > 0) {
            ULONG cbTemp = CorSigUncompressData(&pSigBlob[cb], &ulData);
            if (ulData == ELEMENT_TYPE_SENTINEL) {
                hlp.AddString(L"params ");
                cb += cbTemp;
            }
            cb += UncompressParam(&pSigBlob[cb], hlp);
            if (paramCount)
                hlp.AddString(L", ");
        }
        hlp.AddString(L")");
		return cb;
    }

    struct UncompressHelperGetSig : public UncompressHelper {
        std::wstring *val;
        void AddString(LPCWSTR str) { *val += str; }
    };

    void GetMethodSig(ICorProfilerInfo* info, IMetaDataImport* mdImport, mdMethodDef methodDef, std::wstring* sigVal) {
        ULONG sigSize = 0;
        PCCOR_SIGNATURE pSigBlob;
        if(FAILED(mdImport->GetMethodProps(methodDef, NULL, NULL, 0, NULL, NULL, &pSigBlob, &sigSize, NULL, NULL)))
            return;

        UncompressHelperGetSig getSig;
        getSig.val = sigVal;
        getSig.import = mdImport;
        UncompressMethodDefOrRefSig(pSigBlob, getSig);
    }
}
