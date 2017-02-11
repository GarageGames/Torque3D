#ifndef CRYPTOPP_ALGPARAM_H
#define CRYPTOPP_ALGPARAM_H

#include "cryptlib.h"
#include "smartptr.h"
#include "secblock.h"

#pragma warning (disable : 4189)

NAMESPACE_BEGIN(CryptoPP)

//! used to pass byte array input as part of a NameValuePairs object
/*! the deepCopy option is used when the NameValuePairs object can't
	keep a copy of the data available */
class ConstByteArrayParameter
{
public:
	ConstByteArrayParameter(const char *data = NULL, bool deepCopy = false)
	{
		Assign((const byte *)data, data ? strlen(data) : 0, deepCopy);
	}
	ConstByteArrayParameter(const byte *data, size_t size, bool deepCopy = false)
	{
		Assign(data, size, deepCopy);
	}
	template <class T> ConstByteArrayParameter(const T &string, bool deepCopy = false)
	{
        CRYPTOPP_COMPILE_ASSERT(sizeof(CPP_TYPENAME T::value_type) == 1);
		Assign((const byte *)string.data(), string.size(), deepCopy);
	}

	void Assign(const byte *data, size_t size, bool deepCopy)
	{
		if (deepCopy)
			m_block.Assign(data, size);
		else
		{
			m_data = data;
			m_size = size;
		}
		m_deepCopy = deepCopy;
	}

	const byte *begin() const {return m_deepCopy ? m_block.begin() : m_data;}
	const byte *end() const {return m_deepCopy ? m_block.end() : m_data + m_size;}
	size_t size() const {return m_deepCopy ? m_block.size() : m_size;}

private:
	bool m_deepCopy;
	const byte *m_data;
	size_t m_size;
	SecByteBlock m_block;
};

class ByteArrayParameter
{
public:
	ByteArrayParameter(byte *data = NULL, unsigned int size = 0)
		: m_data(data), m_size(size) {}
	ByteArrayParameter(SecByteBlock &block)
		: m_data(block.begin()), m_size(block.size()) {}

	byte *begin() const {return m_data;}
	byte *end() const {return m_data + m_size;}
	size_t size() const {return m_size;}

private:
	byte *m_data;
	size_t m_size;
};

class CRYPTOPP_DLL CombinedNameValuePairs : public NameValuePairs
{
public:
	CombinedNameValuePairs(const NameValuePairs &pairs1, const NameValuePairs &pairs2)
		: m_pairs1(pairs1), m_pairs2(pairs2) {}

	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const;

private:
	const NameValuePairs &m_pairs1, &m_pairs2;
};

template <class T, class BASE>
class GetValueHelperClass
{
public:
	GetValueHelperClass(const T *pObject, const char *name, const std::type_info &valueType, void *pValue, const NameValuePairs *searchFirst)
		: m_pObject(pObject), m_name(name), m_valueType(&valueType), m_pValue(pValue), m_found(false), m_getValueNames(false)
	{
		if (strcmp(m_name, "ValueNames") == 0)
		{
			m_found = m_getValueNames = true;
			NameValuePairs::ThrowIfTypeMismatch(m_name, typeid(std::string), *m_valueType);
			if (searchFirst)
				searchFirst->GetVoidValue(m_name, valueType, pValue);
			if (typeid(T) != typeid(BASE))
				pObject->BASE::GetVoidValue(m_name, valueType, pValue);
			((*reinterpret_cast<std::string *>(m_pValue) += "ThisPointer:") += typeid(T).name()) += ';';
		}

		if (!m_found && strncmp(m_name, "ThisPointer:", 12) == 0 && strcmp(m_name+12, typeid(T).name()) == 0)
		{
			NameValuePairs::ThrowIfTypeMismatch(m_name, typeid(T *), *m_valueType);
			*reinterpret_cast<const T **>(pValue) = pObject;
			m_found = true;
			return;
		}

		if (!m_found && searchFirst)
			m_found = searchFirst->GetVoidValue(m_name, valueType, pValue);
		
		if (!m_found && typeid(T) != typeid(BASE))
			m_found = pObject->BASE::GetVoidValue(m_name, valueType, pValue);
	}

	operator bool() const {return m_found;}

	template <class R>
	GetValueHelperClass<T,BASE> & operator()(const char *name, const R & (T::*pm)() const)
	{
		if (m_getValueNames)
			(*reinterpret_cast<std::string *>(m_pValue) += name) += ";";
		if (!m_found && strcmp(name, m_name) == 0)
		{
			NameValuePairs::ThrowIfTypeMismatch(name, typeid(R), *m_valueType);
			*reinterpret_cast<R *>(m_pValue) = (m_pObject->*pm)();
			m_found = true;
		}
		return *this;
	}

	GetValueHelperClass<T,BASE> &Assignable()
	{
#ifndef __INTEL_COMPILER	// ICL 9.1 workaround: Intel compiler copies the vTable pointer for some reason
		if (m_getValueNames)
			((*reinterpret_cast<std::string *>(m_pValue) += "ThisObject:") += typeid(T).name()) += ';';
		if (!m_found && strncmp(m_name, "ThisObject:", 11) == 0 && strcmp(m_name+11, typeid(T).name()) == 0)
		{
			NameValuePairs::ThrowIfTypeMismatch(m_name, typeid(T), *m_valueType);
			*reinterpret_cast<T *>(m_pValue) = *m_pObject;
			m_found = true;
		}
#endif
		return *this;
	}

private:
	const T *m_pObject;
	const char *m_name;
	const std::type_info *m_valueType;
	void *m_pValue;
	bool m_found, m_getValueNames;
};

template <class BASE, class T>
GetValueHelperClass<T, BASE> GetValueHelper(const T *pObject, const char *name, const std::type_info &valueType, void *pValue, const NameValuePairs *searchFirst=NULL, BASE *dummy=NULL)
{
	return GetValueHelperClass<T, BASE>(pObject, name, valueType, pValue, searchFirst);
}

template <class T>
GetValueHelperClass<T, T> GetValueHelper(const T *pObject, const char *name, const std::type_info &valueType, void *pValue, const NameValuePairs *searchFirst=NULL)
{
	return GetValueHelperClass<T, T>(pObject, name, valueType, pValue, searchFirst);
}

// ********************************************************

template <class R>
R Hack_DefaultValueFromConstReferenceType(const R &)
{
	return R();
}

template <class R>
bool Hack_GetValueIntoConstReference(const NameValuePairs &source, const char *name, const R &value)
{
	return source.GetValue(name, const_cast<R &>(value));
}

template <class T, class BASE>
class AssignFromHelperClass
{
public:
	AssignFromHelperClass(T *pObject, const NameValuePairs &source)
		: m_pObject(pObject), m_source(source), m_done(false)
	{
		if (source.GetThisObject(*pObject))
			m_done = true;
		else if (typeid(BASE) != typeid(T))
			pObject->BASE::AssignFrom(source);
	}

	template <class R>
	AssignFromHelperClass & operator()(const char *name, void (T::*pm)(R))	// VC60 workaround: "const R &" here causes compiler error
	{
		if (!m_done)
		{
			R value = Hack_DefaultValueFromConstReferenceType(reinterpret_cast<R>(*(int *)NULL));
			if (!Hack_GetValueIntoConstReference(m_source, name, value))
				throw InvalidArgument(std::string(typeid(T).name()) + ": Missing required parameter '" + name + "'");
			(m_pObject->*pm)(value);
		}
		return *this;
	}

	template <class R, class S>
	AssignFromHelperClass & operator()(const char *name1, const char *name2, void (T::*pm)(R, S))	// VC60 workaround: "const R &" here causes compiler error
	{
		if (!m_done)
		{
			R value1 = Hack_DefaultValueFromConstReferenceType(reinterpret_cast<R>(*(int *)NULL));
			if (!Hack_GetValueIntoConstReference(m_source, name1, value1))
				throw InvalidArgument(std::string(typeid(T).name()) + ": Missing required parameter '" + name1 + "'");
			S value2 = Hack_DefaultValueFromConstReferenceType(reinterpret_cast<S>(*(int *)NULL));
			if (!Hack_GetValueIntoConstReference(m_source, name2, value2))
				throw InvalidArgument(std::string(typeid(T).name()) + ": Missing required parameter '" + name2 + "'");
			(m_pObject->*pm)(value1, value2);
		}
		return *this;
	}

private:
	T *m_pObject;
	const NameValuePairs &m_source;
	bool m_done;
};

template <class BASE, class T>
AssignFromHelperClass<T, BASE> AssignFromHelper(T *pObject, const NameValuePairs &source, BASE *dummy=NULL)
{
	return AssignFromHelperClass<T, BASE>(pObject, source);
}

template <class T>
AssignFromHelperClass<T, T> AssignFromHelper(T *pObject, const NameValuePairs &source)
{
	return AssignFromHelperClass<T, T>(pObject, source);
}

// ********************************************************

// to allow the linker to discard Integer code if not needed.
typedef bool (CRYPTOPP_API * PAssignIntToInteger)(const std::type_info &valueType, void *pInteger, const void *pInt);
CRYPTOPP_DLL extern PAssignIntToInteger g_pAssignIntToInteger;

CRYPTOPP_DLL const std::type_info & CRYPTOPP_API IntegerTypeId();

class CRYPTOPP_DLL AlgorithmParametersBase
{
public:
	class ParameterNotUsed : public Exception
	{
	public: 
		ParameterNotUsed(const char *name) : Exception(OTHER_ERROR, std::string("AlgorithmParametersBase: parameter \"") + name + "\" not used") {}
	};

	// this is actually a move, not a copy
	AlgorithmParametersBase(const AlgorithmParametersBase &x)
		: m_name(x.m_name), m_throwIfNotUsed(x.m_throwIfNotUsed), m_used(x.m_used)
	{
		m_next.reset(const_cast<AlgorithmParametersBase &>(x).m_next.release());
		x.m_used = true;
	}

	AlgorithmParametersBase(const char *name, bool throwIfNotUsed)
		: m_name(name), m_throwIfNotUsed(throwIfNotUsed), m_used(false) {}

	virtual ~AlgorithmParametersBase()
	{
#ifdef CRYPTOPP_UNCAUGHT_EXCEPTION_AVAILABLE
		if (!std::uncaught_exception())
#else
		try
#endif
		{
			if (m_throwIfNotUsed && !m_used)
				throw ParameterNotUsed(m_name);
		}
#ifndef CRYPTOPP_UNCAUGHT_EXCEPTION_AVAILABLE
		catch(...)
		{
		}
#endif
	}

	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const;
	
protected:
	friend class AlgorithmParameters;
	void operator=(const AlgorithmParametersBase& rhs);	// assignment not allowed, declare this for VC60

	virtual void AssignValue(const char *name, const std::type_info &valueType, void *pValue) const =0;
	virtual void MoveInto(void *p) const =0;	// not really const

	const char *m_name;
	bool m_throwIfNotUsed;
	mutable bool m_used;
	member_ptr<AlgorithmParametersBase> m_next;
};

template <class T>
class AlgorithmParametersTemplate : public AlgorithmParametersBase
{
public:
	AlgorithmParametersTemplate(const char *name, const T &value, bool throwIfNotUsed)
		: AlgorithmParametersBase(name, throwIfNotUsed), m_value(value)
	{
	}

	void AssignValue(const char *name, const std::type_info &valueType, void *pValue) const
	{
		// special case for retrieving an Integer parameter when an int was passed in
		if (!(g_pAssignIntToInteger != NULL && typeid(T) == typeid(int) && g_pAssignIntToInteger(valueType, pValue, &m_value)))
		{
			NameValuePairs::ThrowIfTypeMismatch(name, typeid(T), valueType);
			*reinterpret_cast<T *>(pValue) = m_value;
		}
	}

	void MoveInto(void *buffer) const
	{
		AlgorithmParametersTemplate<T>* p = new(buffer) AlgorithmParametersTemplate<T>(*this);
	}

protected:
	T m_value;
};

CRYPTOPP_DLL_TEMPLATE_CLASS AlgorithmParametersTemplate<bool>;
CRYPTOPP_DLL_TEMPLATE_CLASS AlgorithmParametersTemplate<int>;
CRYPTOPP_DLL_TEMPLATE_CLASS AlgorithmParametersTemplate<ConstByteArrayParameter>;

class CRYPTOPP_DLL AlgorithmParameters : public NameValuePairs
{
public:
	AlgorithmParameters();

#ifdef __BORLANDC__
	template <class T>
	AlgorithmParameters(const char *name, const T &value, bool throwIfNotUsed=true)
		: m_next(new AlgorithmParametersTemplate<T>(name, value, throwIfNotUsed))
		, m_defaultThrowIfNotUsed(throwIfNotUsed)
	{
	}
#endif

	AlgorithmParameters(const AlgorithmParameters &x);

	AlgorithmParameters & operator=(const AlgorithmParameters &x);

	template <class T>
	AlgorithmParameters & operator()(const char *name, const T &value, bool throwIfNotUsed)
	{
		member_ptr<AlgorithmParametersBase> p(new AlgorithmParametersTemplate<T>(name, value, throwIfNotUsed));
		p->m_next.reset(m_next.release());
		m_next.reset(p.release());
		m_defaultThrowIfNotUsed = throwIfNotUsed;
		return *this;
	}

	template <class T>
	AlgorithmParameters & operator()(const char *name, const T &value)
	{
		return operator()(name, value, m_defaultThrowIfNotUsed);
	}

	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const;

protected:
	member_ptr<AlgorithmParametersBase> m_next;
	bool m_defaultThrowIfNotUsed;
};

//! Create an object that implements NameValuePairs for passing parameters
/*! \param throwIfNotUsed if true, the object will throw an exception if the value is not accessed
	\note throwIfNotUsed is ignored if using a compiler that does not support std::uncaught_exception(),
	such as MSVC 7.0 and earlier.
	\note A NameValuePairs object containing an arbitrary number of name value pairs may be constructed by
	repeatedly using operator() on the object returned by MakeParameters, for example:
	AlgorithmParameters parameters = MakeParameters(name1, value1)(name2, value2)(name3, value3);
*/
#ifdef __BORLANDC__
typedef AlgorithmParameters MakeParameters;
#else
template <class T>
AlgorithmParameters MakeParameters(const char *name, const T &value, bool throwIfNotUsed = true)
{
	return AlgorithmParameters()(name, value, throwIfNotUsed);
}
#endif

#define CRYPTOPP_GET_FUNCTION_ENTRY(name)		(Name::name(), &ThisClass::Get##name)
#define CRYPTOPP_SET_FUNCTION_ENTRY(name)		(Name::name(), &ThisClass::Set##name)
#define CRYPTOPP_SET_FUNCTION_ENTRY2(name1, name2)	(Name::name1(), Name::name2(), &ThisClass::Set##name1##And##name2)

NAMESPACE_END

#endif
