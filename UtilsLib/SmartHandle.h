/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include <concrt.h>
#include <wtypes.h>

//////////////////////////////////////////////////////////////////////////
// SmartPtr for HANDLE types
//
template <typename HandleType,
	template <class> class CloseFunction, HandleType NULL_VALUE = nullptr>
class CSmartHandle : public CloseFunction<HandleType>
{
public:
	CSmartHandle()
	{
		m_Handle = NULL_VALUE;
	}

	CSmartHandle(const HandleType& h)
	{
		m_Handle = h;
	}

	~CSmartHandle()
	{
		CleanUp();
	}

	HandleType operator=(const HandleType& h)
	{
		if (m_Handle != h)
		{
			CleanUp();
			m_Handle = h;
		}

		return(*this);
	}

	bool CloseHandle()
	{
		return CleanUp();
	}

	HandleType Detach()
	{
		HandleType hTmp = m_Handle;
		m_Handle = NULL_VALUE;
		return hTmp;
	}

	operator HandleType() const
	{
		return m_Handle;
	}

	HandleType* operator&() throw()
	{
		ASSERT(m_Handle == NULL_VALUE);
		return &m_Handle;
	}

	operator bool() const
	{
		return IsValid();
	}

	bool IsValid() const
	{
		return (m_Handle != NULL_VALUE);
	}

	bool IsNotValid() const
	{
		return (m_Handle == NULL_VALUE);
	}

protected:
	bool CleanUp()
	{
		bool b = false;
		if (m_Handle != NULL_VALUE)
		{
			b = Close(m_Handle);
			m_Handle = NULL_VALUE;
		}
		return b;
	}

	HandleType m_Handle;
};

// Specialization

template <typename T>
class CCloseHandle
{
public:
	~CCloseHandle() = default;

	static bool Close(T handle)
	{
		return FALSE != ::CloseHandle(handle);
	}
};

template <typename T>
class CCloseRegKey
{
public:
	~CCloseRegKey() = default;

	static bool Close(T handle)
	{
		return (RegCloseKey(handle) == ERROR_SUCCESS);
	}
};


template <typename T>
class CCloseLibrary
{
public:
	~CCloseLibrary() = default;

	static bool Close(T handle)
	{
		return FALSE != ::FreeLibrary(handle);
	}
};


template <typename T>
class CCloseViewOfFile
{
public:
	~CCloseViewOfFile() = default;

	static bool Close(T handle)
	{
		return FALSE != ::UnmapViewOfFile(handle);
	}
};

template <typename T>
class CCloseFindFile
{
	~CCloseFindFile() = default;

	static bool Close(T handle)
	{
		return FALSE != ::FindClose(handle);
	}
};

// Client code (definitions of standard Windows handles).
using CAutoHandle = CSmartHandle<HANDLE, CCloseHandle>;
using CAutoRegKey = CSmartHandle<HKEY, CCloseRegKey>;
using CAutoViewOfFile = CSmartHandle<PVOID, CCloseViewOfFile>;
using CAutoLibrary = CSmartHandle<HMODULE, CCloseLibrary>;
using CAutoFile = CSmartHandle<HANDLE, CCloseHandle, INVALID_HANDLE_VALUE>;
using CAutoFindFile = CSmartHandle<HANDLE, CCloseFindFile, INVALID_HANDLE_VALUE>;
