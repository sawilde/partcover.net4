#pragma once
// look on http://blogs.msdn.com/davbr/articles/480857.aspx

typedef unsigned char sig_byte;
typedef unsigned char sig_elem_type;
typedef unsigned char sig_index_type;
typedef unsigned int sig_index;
typedef unsigned int sig_count;
typedef unsigned int sig_mem_number;

class SigParser
{
private:
	sig_byte *pbBase;
	sig_byte *pbCur;
	sig_byte *pbEnd;

public:    

	bool Parse(sig_byte *blob, sig_count len);

private:
	bool ParseByte(sig_byte *pbOut);
	bool ParseNumber(sig_count *pOut);
	bool ParseTypeDefOrRefEncoded(mdToken *token);

	bool ParseMethod(sig_elem_type);
	bool ParseField(sig_elem_type);
	bool ParseProperty(sig_elem_type);
	bool ParseLocals(sig_elem_type);
	bool ParseLocal();
	bool ParseOptionalCustomMods();
	bool ParseOptionalCustomModsOrConstraint();
	bool ParseCustomMod();
	bool ParseRetType();
	bool ParseType();
	bool ParseParam();
	bool ParseArrayShape();

protected:

	// subtype these methods to create your parser side-effects

	//----------------------------------------------------

	// a method with given elem_type
	virtual void NotifyBeginMethod(sig_elem_type elem_type)  {}
	virtual void NotifyEndMethod()  {}

	// total parameters for the method
	virtual void NotifyParamCount(sig_count) {}

	// starting a return type
	virtual void NotifyBeginRetType() {}
	virtual void NotifyEndRetType() {}

	// starting a parameter
	virtual void NotifyBeginParam() {}
	virtual void NotifyEndParam() {}

	// sentinel indication the location of the "..." in the method signature
	virtual void NotifySentinal() {}

	// number of generic parameters in this method signature (if any)
	virtual void NotifyGenericParamCount(sig_count) {}

	//----------------------------------------------------

	// a field with given elem_type
	virtual void NotifyBeginField(sig_elem_type elem_type)  {}
	virtual void NotifyEndField()  {}

	//----------------------------------------------------

	// a block of locals with given elem_type (always just LOCAL_SIG for now)
	virtual void NotifyBeginLocals(sig_elem_type elem_type)  {}
	virtual void NotifyEndLocals()  {}

	// count of locals with a block
	virtual void NotifyLocalsCount(sig_count) {}

	// starting a new local within a local block
	virtual void NotifyBeginLocal()  {}
	virtual void NotifyEndLocal()  {}

	// the only constraint available to locals at the moment is ELEMENT_TYPE_PINNED
	virtual void NotifyConstraint(sig_elem_type elem_type) {}

	//----------------------------------------------------

	// a property with given element type
	virtual void NotifyBeginProperty(sig_elem_type elem_type)  {}
	virtual void NotifyEndProperty()  {}

	//----------------------------------------------------

	// starting array shape information for array types
	virtual void NotifyBeginArrayShape() {}
	virtual void NotifyEndArrayShape() {}

	// array rank (total number of dimensions)
	virtual void NotifyRank(sig_count) {}

	// number of dimensions with specified sizes followed by the size of each
	virtual void NotifyNumSizes(sig_count) {}
	virtual void NotifyNextSize() {}
	virtual void NotifySize(sig_count) {}

	// BUG BUG lower bounds can be negative, how can this be encoded?
	// number of dimensions with specified lower bounds followed by lower bound of each 
	virtual void NotifyNumLoBounds(sig_count) {}  
	virtual void NotifyNextLoBound() {}
	virtual void NotifyLoBound(sig_count) {}

	//----------------------------------------------------


	// starting a normal type (occurs in many contexts such as param, field, local, etc)
	virtual void NotifyBeginType() {};
	virtual void NotifyEndType() {};

	virtual void NotifyTypedByref() {}

	// the type has the 'byref' modifier on it -- this normally proceeds the type definition in the context
	// the type is used, so for instance a parameter might have the byref modifier on it
	// so this happens before the BeginType in that context
	virtual void NotifyByref() {}

	// the type is "VOID" (this has limited uses, function returns and void pointer)
	virtual void NotifyVoid() {}

	// the type has the indicated custom modifiers (which can be optional or required)
	virtual void NotifyCustomMod(sig_elem_type cmod, mdToken token) {}

	// the type is a simple type, the elem_type defines it fully
	virtual void NotifyTypeSimple(sig_elem_type  elem_type) {}

	// the type is specified by the given index of the given index type (normally a type index in the type metadata)
	// this callback is normally qualified by other ones such as NotifyTypeClass or NotifyTypeValueType

	//virtual void NotifyTypeDefOrRef(sig_index_type  indexType, int index) {}
	// replaced by 
	virtual void NotifyTypeDefOrRef(mdToken token) {}

	// the type is an instance of a generic
	// elem_type indicates value_type or class
	// indexType and index indicate the metadata for the type in question
	// number indicates the number of type specifications for the generic types that will follow
	virtual void NotifyTypeGenericInst(sig_elem_type elem_type, mdToken token, sig_mem_number number) {}

	// the type is the type of the nth generic type parameter for the class
	virtual void NotifyTypeGenericTypeVariable(sig_mem_number number) {}  

	// the type is the type of the nth generic type parameter for the member
	virtual void NotifyTypeGenericMemberVariable(sig_mem_number number) {}  

	// the type will be a value type
	virtual void NotifyTypeValueType() {}

	// the type will be a class
	virtual void NotifyTypeClass() {}

	// the type is a pointer to a type (nested type notifications follow)
	virtual void NotifyTypePointer() {}

	// the type is a function pointer, followed by the type of the function
	virtual void NotifyTypeFunctionPointer() {}

	// the type is an array, this is followed by the array shape, see above, as well as modifiers and element type
	virtual void NotifyTypeArray() {}

	// the type is a simple zero-based array, this has no shape but does have custom modifiers and element type
	virtual void NotifyTypeSzArray() {}

	// method parameter list
	virtual void NotifyBeginMethodParamList() {}
	virtual void NotifyHasMoreParameters() {}
	virtual void NotifyEndMethodParamList() {}
};


class MethodSigWriter : public SigParser
{
	String* _value;
	IMetaDataImport* _md;

public:
	MethodSigWriter(String* value, IMetaDataImport* md) : _value(value), _md(md) {}

protected:
	void Append(const String& w) 
	{ 
		_value->append(w);
	}

	void AppendNumber(int i) 
	{ 
		TCHAR buffer[10];
		int written = _stprintf_s(buffer, 9, _T("%d"), i);
		if (written >= 0) 
		{
			Append(String(buffer, written)); 
		}
	}


	virtual void NotifyByref() 
	{ 
		Append(_T("ref "));
	}

	virtual void NotifyVoid() 
	{ 
		Append(_T("void")); 
	}

	virtual void NotifyEndRetType() 
	{ 
		Append(_T(" ")); 
	}

	virtual void NotifySentinal() 
	{
		Append(_T(" ...")); 
	}

	virtual void NotifyBeginMethodParamList() 
	{
		Append(_T(" (")); 
	}

	virtual void NotifyHasMoreParameters() 
	{
		Append(_T(", ")); 
	}

	virtual void NotifyEndMethodParamList() 
	{
		Append(_T(")")); 
	}

	virtual void NotifyTypeDefOrRef(mdToken token);
	virtual void NotifyTypeSimple(sig_elem_type elem_type);

	virtual void NotifyGenericParamCount(sig_count count) 
	{
		Append(_T("<")); 
		AppendNumber(count); 
		Append(_T(">")); 
	}

	virtual void NotifyCustomMod(sig_elem_type cmod, mdToken token) 
	{
		Append(_T("const ")); 
	}

		// starting array shape information for array types
	virtual void NotifyBeginArrayShape() 
	{
		Append(_T("[")); 
	}

	virtual void NotifyEndArrayShape() 
	{
		Append(_T("]")); 
	}

	// array rank (total number of dimensions)
	virtual void NotifyRank(sig_count count) 
	{
		AppendNumber(count); 
		Append(_T(":")); 
	}

	// number of dimensions with specified sizes followed by the size of each
	virtual void NotifyNumSizes(sig_count count) {}
	virtual void NotifyNextSize() 
	{
		Append(_T(" ")); 
	};
	virtual void NotifySize(sig_count count) {}

	// BUG BUG lower bounds can be negative, how can this be encoded?
	// number of dimensions with specified lower bounds followed by lower bound of each 
	virtual void NotifyNumLoBounds(sig_count count) 
	{
		Append(_T(":")); 
	}  

	virtual void NotifyNextLoBound() 
	{
		Append(_T(" ")); 
	}

	virtual void NotifyLoBound(sig_count count) {}

	virtual void NotifyTypeSzArray() 
	{
		Append(_T("[]")); 
	}
};