#include "stdafx.h"
#include "interface.h"
#include "helpers.h"
#include "corhelper.h"
#include "il_sigparser.h"

namespace CorHelper {
	String GetModuleName(ICorProfilerInfo* info, ModuleID module) {
		ULONG buffSize = 0;
		if (S_OK != info->GetModuleInfo(module, NULL, 0, &buffSize, NULL, NULL)) 
			return String();
		DynamicArray<WCHAR> buffer(buffSize + 1);
		if (S_OK != info->GetModuleInfo(module, NULL, buffSize + 1, &buffSize, buffer, NULL))
			return String();
		return String(buffer);
	}

	String GetAssemblyName(ICorProfilerInfo* info, AssemblyID assembly) {
		ULONG bufferSize = 0;
		if (S_OK != info->GetAssemblyInfo(assembly, 0, &bufferSize, NULL, NULL, NULL)) 
			return String();
		DynamicArray<WCHAR> buffer(bufferSize + 1);
		if (S_OK != info->GetAssemblyInfo(assembly, bufferSize + 1, &bufferSize, buffer, NULL, NULL))
			return String();
		return String(buffer);
	}

	String GetTypedefFullName(IMetaDataImport* mdImport, mdTypeDef typeDef, DWORD *p_typeDefFlags, const String& connectStr, const String& innerTypeDefName) {
		ULONG bufferSize = 0;
		DWORD typeDefFlags = 0;
		if(S_OK == mdImport->GetTypeDefProps(typeDef, NULL, 0, &bufferSize, &typeDefFlags, NULL)) {
			if (p_typeDefFlags != 0) *p_typeDefFlags = typeDefFlags;

			DynamicArray<TCHAR> buffer(bufferSize + 1);
			mdImport->GetTypeDefProps(typeDef, buffer, bufferSize+1, &bufferSize, NULL, NULL);
			String typedefName = buffer;
			
			if (innerTypeDefName.length() > 0)
				typedefName += String(connectStr) + innerTypeDefName;

			if (!IsTdNested(typeDefFlags)) 
				return typedefName;

			mdTypeDef enclosingTypeDef;
			if (S_OK == mdImport->GetNestedClassProps(typeDef, &enclosingTypeDef))
				return GetTypedefFullName(mdImport, enclosingTypeDef, NULL, connectStr, typedefName);
		}
		return String();
	}

	String GetClassName(ICorProfilerInfo* info, ClassID classId) {
		ModuleID module;
		mdTypeDef typeDef;
		if (S_OK != info->GetClassIDInfo(classId, &module, &typeDef))
			return String();

		CComPtr<IMetaDataImport> mdImport;
		if (S_OK != info->GetModuleMetaData(module, ofRead, IID_IMetaDataImport, (IUnknown**) &mdImport))
			return String();

		ULONG bufferSize;
		if (S_OK != mdImport->GetTypeDefProps(typeDef, NULL, 0, &bufferSize, NULL, NULL))
			return String();

		DynamicArray<WCHAR> buffer(bufferSize + 1);
		if (S_OK != mdImport->GetTypeDefProps(typeDef, buffer, bufferSize + 1, &bufferSize, NULL, NULL))
			return String();

		return GetTypedefFullName(mdImport, typeDef, 0, _T("+"), String(buffer));
	}

	const wchar_t* StrCalling[] = {
		_T("default"), _T("C"), _T("stdcall"), _T("thiscall"), _T("fastcall"), _T("vararg"), _T("field"), _T("localsig"), _T("property"), _T("unmanaged")
	};

	const wchar_t* MapElementType[] = 
	{
		_T("End"), _T("Void"), _T("Boolean"), _T("Char"), _T("I1"), _T("UI1"), _T("I2"), _T("UI2"), _T("I4"), _T("UI4"), _T("I8"), _T("UI8"), _T("R4"),
		_T("R8"), _T("String"), _T("Ptr"), _T("ByRef"), _T("ValueClass"), _T("Class"), _T("CopyCtor"), _T("MDArray"), _T("GENArray"), _T("TypedByRef"),
		_T("VALUEARRAY"), _T("I"), _T("U"), _T("R"), _T("FNPTR"), _T("Object"), _T("SZArray"), _T("GENERICArray"), _T("CMOD_REQD"), _T("CMOD_OPT"), _T("INTERNAL"),
	};

	String TypeRefName(IMetaDataImport* mdImport, mdTypeRef tr)
	{
		ULONG bufferSize;
		if(S_OK != mdImport->GetTypeRefProps(tr, NULL, NULL, 0, &bufferSize))
			return String();
		DynamicArray<WCHAR> buffer(bufferSize + 1);
		if(S_OK != mdImport->GetTypeRefProps(tr, NULL, buffer, (ULONG)buffer.size(), &bufferSize))
			return String();
		return String(buffer.ptr(), buffer.size());
	}

	struct UncompressHelper 
	{
		IMetaDataImport* import;
		void AddString(const String& str) { AddString(str.c_str()); }

		virtual void AddString(LPCTSTR str) = 0;
	};

	String GetTypeRefOrDefName(mdToken inToken, UncompressHelper& hlp)
	{
		if (!IsNilToken(inToken)) {
			switch(TypeFromToken(inToken)) 
			{
				case mdtTypeDef: 
					return GetTypedefFullName(hlp.import, (mdTypeDef) inToken, NULL, _T("."));
				case mdtTypeRef: 
					return TypeRefName(hlp.import, (mdTypeRef) inToken);
				default: 
					return _T("[invalidReference]");
			}
		}
		return _T("");
	}

	String GetTokenAssembly(mdToken inToken, UncompressHelper& hlp) {
		return _T("");
	}

	ULONG UncompressCustomMod(PCCOR_SIGNATURE pSigBlob, UncompressHelper& hlp) {
		mdToken token;
		ULONG ulData;
		ULONG cb = CorSigUncompressData(pSigBlob, &ulData);
		switch(ulData) {
	case ELEMENT_TYPE_CMOD_OPT:
	case ELEMENT_TYPE_CMOD_REQD:
		hlp.AddString(_T("const "));
		cb += CorSigUncompressToken(&pSigBlob[cb], &token);
		break;
	default:
		cb = 0;
		}
		return cb;
	}

	const LPCTSTR PrimitiveTypes[] = {
		_T("end"), _T("void"), _T("boolean"), _T("char"), _T("byte"), _T("unsigned byte"), _T("short"), _T("unsigned short"), _T("int"),
		_T("unsigned int"), _T("long"), _T("unsigned long"), _T("float"), _T("double"), _T("string")
	};

	ULONG UncompressMethodDefOrRefSig(PCCOR_SIGNATURE pSigBlob, UncompressHelper& hlp);

	ULONG UncompressType(PCCOR_SIGNATURE pSigBlob, UncompressHelper& hlp)
	{
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
		//hlp.AddString(_T([");
		hlp.AddString(GetTokenAssembly(token, hlp));
		//hlp.AddString(_T(]");
		hlp.AddString(GetTypeRefOrDefName(token, hlp));
		break;
	case ELEMENT_TYPE_OBJECT:
		hlp.AddString(_T("object"));
		break;
	case ELEMENT_TYPE_PTR:
		cb += UncompressType(&pSigBlob[cb], hlp);
		break;
	case ELEMENT_TYPE_SZARRAY:
		cb += UncompressType(&pSigBlob[cb], hlp);
		hlp.AddString(_T("[]"));
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
		break;

	case ELEMENT_TYPE_GENERICINST:
		cb += UncompressType(&pSigBlob[cb], hlp);
		cb += CorSigUncompressData(&pSigBlob[cb], &numSizes);

		hlp.AddString(_T("["));
		for(ULONG i = 0; i < numSizes; ++i) {
			cb += UncompressType(&pSigBlob[cb], hlp);
			if (i < numSizes) hlp.AddString(_T(", "));
		}
		hlp.AddString(_T("]"));
		break;

	case ELEMENT_TYPE_MVAR:
	case ELEMENT_TYPE_VAR:

		cb += CorSigUncompressData(&pSigBlob[cb], 1, &rank, &numSizes);
		{
			WCHAR buffer[10];
			swprintf_s(buffer, 10, _T("!%d"), rank);
			hlp.AddString(buffer);
		}
		break;

	default:
		break;
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
		hlp.AddString(_T("void")); break;
	case ELEMENT_TYPE_TYPEDBYREF: 
		hlp.AddString(_T("refany")); break;
	case ELEMENT_TYPE_BYREF: 
		hlp.AddString(_T("ref "));
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
		hlp.AddString(_T("refany")); break;
	case ELEMENT_TYPE_BYREF: 
		hlp.AddString(_T("ref "));
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
		hlp.AddString(_T(" ("));
		while(paramCount-- > 0) {
			ULONG cbTemp = CorSigUncompressData(&pSigBlob[cb], &ulData);
			if (ulData == ELEMENT_TYPE_SENTINEL) {
				hlp.AddString(_T("params "));
				cb += cbTemp;
			}
			cb += UncompressParam(&pSigBlob[cb], hlp);
			if (paramCount)
				hlp.AddString(_T(", "));
		}
		hlp.AddString(_T(")"));
		return cb;
	}

	struct UncompressHelperGetSig : public UncompressHelper {
		String *val;
		void AddString(LPCTSTR str) { *val += str; }
	};

	void GetMethodSig(ICorProfilerInfo* info, IMetaDataImport* mdImport, mdMethodDef methodDef, String* sigVal) {
		ULONG sigSize = 0;
		PCCOR_SIGNATURE pSigBlob;
		if(FAILED(mdImport->GetMethodProps(methodDef, NULL, NULL, 0, NULL, NULL, &pSigBlob, &sigSize, NULL, NULL)))
			return;

		MethodSigWriter writer(sigVal, mdImport);

		writer.Parse((sig_byte*)pSigBlob, sigSize);
	}
}
