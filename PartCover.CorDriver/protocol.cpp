#include "stdafx.h"
#include "protocol.h"
#include "interface.h"
#include "rules.h"
#include "instrumented_results.h"

IntercommunicationProxy IntercommunicationProxy::instance;

namespace bidi
{
	template<class T>
	NamedPipe& operator << (NamedPipe& ar, const std::list<T>& a) 
	{
		typedef std::list<T> container;

		container::size_type count = a.size();
		ar << count;
		for(std::list<T>::const_iterator it = a.begin(), it_end = a.end(); it != it_end; ++it)
			ar << *it;
		return ar;
	}

	template<class T>
	NamedPipe& operator >> (NamedPipe& ar, std::list<T>& a) 
	{
		typedef std::list<T> container;

		container::size_type count;
		ar >> count;

		a.swap(container());
		
		for(container::size_type i = 0, i_end = count; i < count; ++i) 
		{
			container::value_type value;
			ar >> value;
			a.push_back(value);
		}
		return ar;
	}

	template<class T>
	NamedPipe& operator << (NamedPipe& ar, const std::vector<T>& a) {
		typedef std::vector<T> container;

		container::size_type count = a.size();
		ar << count;
		for(container::const_iterator it = a.begin(), it_end = a.end(); it != it_end; ++it)
			ar << *it;
		return ar;
	}

	template<class T>
	NamedPipe& operator >> (NamedPipe& ar, std::vector<T>& a) {
		typedef std::vector<T> container;

		container::size_type count;
		ar >> count;

		a.swap(container());
		a.reserve(count);

		for(container::size_type i = 0, i_end = count; i < count; ++i) 
		{
			container::value_type value;
			ar >> value;
			a.push_back(value);
		}
		return ar;
	}

	NamedPipe& operator << (NamedPipe& ar, const Rules& a) {
		ar << a.enable_coverage_profile << a.enable_call_tree_profile << a.profiler_level;
		ar << a.m_excludeItems << a.m_includeItems << a.m_includeRules << a.m_excludeRules;
		return ar;
	}

	NamedPipe& operator >> (NamedPipe& ar, Rules &a) {
		ar >> a.enable_coverage_profile >> a.enable_call_tree_profile >> a.profiler_level;
		ar >> a.m_excludeItems >> a.m_includeItems >> a.m_includeRules >> a.m_excludeRules;
		return ar;
	}

	NamedPipe& operator << (NamedPipe& ar, const FunctionMap& a) {
		return ar;
	}

	NamedPipe& operator >> (NamedPipe& ar, FunctionMap &a) {
		return ar;
	}

	NamedPipe& operator << (NamedPipe& ar, const InstrumentResults::FileItem &a) {
		return ar << a.fileId << a.fileUrl;
	}

	NamedPipe& operator >> (NamedPipe& ar, InstrumentResults::FileItem &a) {
		return ar >> a.fileId >> a.fileUrl;
	}

	NamedPipe& operator << (NamedPipe& ar, const InstrumentResults::MethodBlock &a) {
		ar << a.haveSource << a.sourceFileId;
		ar << a.startLine << a.startColumn << a.endLine << a.endColumn;
		ar << a.position << a.blockLength << a.visitCount;
		return ar;
	}

	NamedPipe& operator >> (NamedPipe& ar, InstrumentResults::MethodBlock &a) {
		ar >> a.haveSource >> a.sourceFileId;
		ar >> a.startLine >> a.startColumn >> a.endLine >> a.endColumn;
		ar >> a.position >> a.blockLength >> a.visitCount;
		return ar;
	}

	NamedPipe& operator << (NamedPipe& ar, const InstrumentResults::MethodResult &a) {
		return ar << a.name << a.sig << a.flags << a.implFlags << a.blocks << a.bodySize;
	}

	NamedPipe& operator >> (NamedPipe& ar, InstrumentResults::MethodResult &a) {
		return ar >> a.name >> a.sig >> a.flags >> a.implFlags >> a.blocks >> a.bodySize;
	}

	NamedPipe& operator << (NamedPipe& ar, const InstrumentResults::SkippedItem &a) {
		return ar << a.assemblyName << a.typedefName;
	}

	NamedPipe& operator >> (NamedPipe& ar, InstrumentResults::SkippedItem &a) {
		return ar >> a.assemblyName >> a.typedefName;
	}

	NamedPipe& operator << (NamedPipe& ar, const InstrumentResults::AssemblyResult &a) {
		return ar << a.name << a.module << a.domainIndex << a.domain << a.types;
	}

	NamedPipe& operator >> (NamedPipe& ar, InstrumentResults::AssemblyResult &a) {
		return ar >> a.name >> a.module >> a.domainIndex >> a.domain >> a.types;
	}

	NamedPipe& operator << (NamedPipe& ar, const InstrumentResults::TypedefResult &a) {
		return ar << a.fullName << a.flags << a.methods;
	}

	NamedPipe& operator >> (NamedPipe& ar, InstrumentResults::TypedefResult &a) {
		return ar >> a.fullName >> a.flags >> a.methods;
	}

	NamedPipe& operator << (NamedPipe& ar, const InstrumentResults& a) {
		return ar << a.m_results << a.m_fileTable << a.m_skippedItems;
	}

	NamedPipe& operator >> (NamedPipe& ar, InstrumentResults &a) {
		return ar >> a.m_results >> a.m_fileTable >> a.m_skippedItems;
	}
}