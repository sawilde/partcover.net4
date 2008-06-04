#pragma once

namespace CorHelper {
    std::wstring GetModuleName(ICorProfilerInfo* info, ModuleID module);
    std::wstring GetAssemblyName(ICorProfilerInfo* info, AssemblyID assembly);

    std::wstring GetTypedefFullName(IMetaDataImport* mdImport, mdTypeDef typeDef, DWORD *p_typeDefFlags, LPCWSTR connectStr = L"+", const std::wstring& innerTypeDefName = L"");
	std::wstring TypeRefName(IMetaDataImport* mdImport, mdTypeRef tr);

    std::wstring GetClassName(ICorProfilerInfo* info, ClassID classId);
    void GetMethodSig(ICorProfilerInfo* info, IMetaDataImport* mdImport, mdMethodDef methodDef, std::wstring* sigVal);
}
