/////////////////////////////////////////////////////////////////////////////
/*
DESCRIPTION:
	CFileVersionInfo - Class for getting file version information
	http://www.codeproject.com/file/VersionInfo.asp

NOTES:
	Copyright(C) Armen Hakobyan, 2003
	mailto:armen.h@web.am

VERSION HISTORY:
	25 Jul 2003 - Posted the article
	27 Jul 2003 - Added DLLVERSIONINFO2 support to DllGetVersion
	21 Jan 2004 - Added GetFileVersionMajor, GetFileVersionMinor,
				  GetFileVersionBuild, GetFileVersionQFE functions
	29 Jan 2004 - Added GetProductVersionMajor, GetProductVersionMinor,
				  GetProductVersionBuild, GetProductVersionQFE functions
*/
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "FileVersionInfo.h"

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // _DEBUG

static bool IsBadMemPtr(bool write, void* ptr, size_t size)
{
	if (size == 0)
	{
		return false;
	}

	if (ptr == nullptr)
	{
		return true;
	}

	DWORD mask = PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
	if (!write)
	{
		mask |= PAGE_READONLY | PAGE_EXECUTE_READ;
	}

	auto current = reinterpret_cast<BYTE*>(ptr);
	const auto last = current + size;

	// So we are considering the region:
	// [ptr, ptr+size)

	while (current < last)
	{
		MEMORY_BASIC_INFORMATION mbi;

		if (VirtualQuery(LPCVOID(current), &mbi, sizeof mbi) == 0)
		{
			// We couldn't get any information on this region.
			// Let's not risk any read/write operation.
			return true;
		}

		if ((mbi.Protect & mask) == 0)
		{
			// We can't perform our desired read/write operations in this region.
			return true;
		}

		if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
		{
			// We can't access this region.
			return true;
		}

		// Let's consider the next region.
		current = reinterpret_cast<BYTE*>(mbi.BaseAddress) + mbi.RegionSize;
	}

	return false;
}

HRESULT STDAPICALLTYPE DllGetVersion( IN  HMODULE hModule,
									  OUT DLLVERSIONINFO* lpDVI )
{
	if( hModule == nullptr ||
	   IsBadMemPtr(false, lpDVI, sizeof( DLLVERSIONINFO* ) ) )
	{
		ASSERT_RETURN( S_FALSE );
	}

	CONST DWORD cbSize = lpDVI->cbSize;

	if(
#ifdef DLLVERSIONINFO2
		(
#endif
		cbSize != sizeof( DLLVERSIONINFO  )
#ifdef DLLVERSIONINFO2
		&& cbSize != sizeof( DLLVERSIONINFO2 ) )
#endif
		|| IsBadMemPtr(true, lpDVI, cbSize ) )
	{
		ASSERT_RETURN( S_FALSE );
	}

	SecureZeroMemory( lpDVI, cbSize );
	lpDVI->cbSize = cbSize;

	CFileVersionInfo fvi;
	if( fvi.Open( hModule ) )
	{
		VS_FIXEDFILEINFO vsffi = fvi.GetVSFFI();

		if( vsffi.dwFileType == VFT_DLL ||
			vsffi.dwFileType == VFT_STATIC_LIB )
		{
			switch( vsffi.dwFileOS )
			{
			case VOS__WINDOWS32:
			case VOS_NT_WINDOWS32:
				lpDVI->dwPlatformID = DLLVER_PLATFORM_WINDOWS;
				break;
			case VOS_NT:
				lpDVI->dwPlatformID = DLLVER_PLATFORM_NT;
				break;
			default:
				return ( S_FALSE );
			}

			lpDVI->dwMajorVersion = HIWORD( vsffi.dwFileVersionMS );
			lpDVI->dwMinorVersion = LOWORD( vsffi.dwFileVersionMS );
			lpDVI->dwBuildNumber  = HIWORD( vsffi.dwFileVersionLS );

		#ifdef DLLVERSIONINFO2

			if( cbSize == sizeof( DLLVERSIONINFO2 ) )
			{
				DLLVERSIONINFO2* lpDVI2 = (DLLVERSIONINFO2*)lpDVI;
				lpDVI2->ullVersion = MAKEDLLVERULL(
					lpDVI->dwMajorVersion,
					lpDVI->dwMinorVersion,
					lpDVI->dwBuildNumber ,
					LOWORD( vsffi.dwFileVersionLS )
				);
			}

		#endif

			return ( S_OK );
		}
	#ifdef _DEBUG
		ASSERT( 0 );
	#endif

		fvi.Close();
	}

	return ( S_FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// HIWORD( ffi.dwFileVersionMS ) - major
// LOWORD( ffi.dwFileVersionMS ) - minor
// HIWORD( ffi.dwFileVersionLS ) - build
// LOWORD( ffi.dwFileVersionLS ) - QFE
/////////////////////////////////////////////////////////////////////////////

CFileVersionInfo::CFileVersionInfo()
{
	m_lpbyVIB = nullptr;
	Close();
}

CFileVersionInfo::~CFileVersionInfo()
{
	Close();
}

LPCTSTR CFileVersionInfo::s_ppszStr[] = {
	_T( "Comments" ),			_T( "CompanyName" ),
	_T( "FileDescription" ),	_T( "FileVersion" ),
	_T( "InternalName" ),		_T( "LegalCopyright" ),
	_T( "LegalTrademarks" ),	_T( "OriginalFilename" ),
	_T( "PrivateBuild" ),		_T( "ProductName" ),
	_T( "ProductVersion" ),		_T( "SpecialBuild" ),
	_T( "OLESelfRegister" )
};

////////////////////////////////////////////////////////////////////////////////
// Implementation

BOOL CFileVersionInfo::Open( IN HINSTANCE hInstance )
{
	if( hInstance == nullptr )
		ASSERT_RETURN( FALSE );

	TCHAR szFileName[ MAX_PATH ] = { 0 };
	if( ::GetModuleFileName( hInstance, szFileName, MAX_PATH ) )
		return Open( szFileName );

	return FALSE;
};

BOOL CFileVersionInfo::Open( IN LPCTSTR lpszFileName )
{
	if( lpszFileName == nullptr )
		ASSERT_RETURN( FALSE );

	Close();
	if( !GetVersionInfo( lpszFileName ) || !QueryVersionTrans() )
		Close();

	return m_bValid;
};

BOOL CFileVersionInfo::GetVersionInfo( IN LPCTSTR lpszFileName )
{
	DWORD dwDummy = 0;
	DWORD dwSize  = ::GetFileVersionInfoSize(
		const_cast< LPTSTR >( lpszFileName ), &dwDummy // Set to 0
	);

	if ( dwSize > 0 )
	{
		m_lpbyVIB = (LPBYTE)malloc( dwSize );

		if ( m_lpbyVIB != nullptr &&
			::GetFileVersionInfo( const_cast< LPTSTR >( lpszFileName ),
			0, dwSize, m_lpbyVIB ) )
		{
			UINT   uLen    = 0;
			LPVOID lpVSFFI = nullptr;

			if ( ::VerQueryValue( m_lpbyVIB, _T( "\\" ), (LPVOID*)&lpVSFFI, &uLen ) )
			{
				::CopyMemory( &m_vsffi, lpVSFFI, sizeof( VS_FIXEDFILEINFO ) );
				m_bValid = ( m_vsffi.dwSignature == VS_FFI_SIGNATURE );
			}
		}
	}

	return m_bValid;
}

BOOL CFileVersionInfo::QueryVersionTrans()
{
	if( m_bValid == FALSE )
		ASSERT_RETURN( FALSE );

	UINT   uLen  = 0;
	LPVOID lpBuf = nullptr;

	if( ::VerQueryValue( m_lpbyVIB, _T( "\\VarFileInfo\\Translation" ), (LPVOID*)&lpBuf, &uLen ) )
	{
		m_lpdwTrans = (LPDWORD)lpBuf;
		m_nTransCnt =(DWORD)(uLen / sizeof(DWORD));
	}
	return (m_lpdwTrans != nullptr);
}

void CFileVersionInfo::Close()
{
	m_nTransCnt  = 0;
	m_nTransCur  = 0;
	m_bValid	 = FALSE;
	m_lpdwTrans  = nullptr;

	SecureZeroMemory( &m_vsffi, sizeof( VS_FIXEDFILEINFO ) );
	_free( m_lpbyVIB );
}

BOOL CFileVersionInfo::QueryStringValue(IN LPCTSTR lpszItem, OUT LPTSTR lpszValue, IN INT nBuf ) const
{
	if( !m_bValid || !lpszItem) ASSERT_RETURN( FALSE );
	if( !lpszValue) ASSERT_RETURN( FALSE );
	if( lpszValue && nBuf <= 0 ) ASSERT_RETURN( FALSE );

	SecureZeroMemory( lpszValue, nBuf * sizeof( TCHAR ) );

	const auto& csSFI = fmt::format(L"\\StringFileInfo\\{:04x}{:04x}\\{:s}", GetCurLID(), GetCurCP(), lpszItem);

	BOOL   bRes    = FALSE;
	UINT   uLen    = 0;
	LPTSTR lpszBuf = nullptr;

	if( ::VerQueryValueW( m_lpbyVIB, csSFI.c_str(), (LPVOID*)&lpszBuf, &uLen ) )
	{
		if(uLen > 0 )
		{
			StrCpyN( lpszValue, lpszBuf, uLen);
			bRes = TRUE;
		}
		else
		{
			bRes = FALSE;
		}
	}

	return bRes;
}

BOOL CFileVersionInfo::QueryStringValue(IN INT nIndex, OUT LPTSTR lpszValue, IN INT nBuf) const
{
	if( nIndex < VI_STR_COMMENTS ||
		nIndex > VI_STR_OLESELFREGISTER )
	{
		ASSERT_RETURN( FALSE );
	}
	return QueryStringValue( s_ppszStr[ nIndex ], lpszValue, nBuf );
}

LPCTSTR CFileVersionInfo::GetVerStringName( IN INT nIndex )
{
	if( nIndex < VI_STR_COMMENTS ||
		nIndex > VI_STR_OLESELFREGISTER )
	{
		ASSERT_RETURN( FALSE );
	}
	return (LPCTSTR)s_ppszStr[ nIndex ];
}

INT CFileVersionInfo::FindTrans( IN LANGID wLID,
								 IN WORD   wCP ) const
{
	if( m_bValid == FALSE )
		ASSERT_RETURN( -1 );

	for( UINT n = 0; n < m_nTransCnt; n++ )
	{
		if( LOWORD( m_lpdwTrans[ n ] ) == wLID &&
			HIWORD( m_lpdwTrans[ n ] ) == wCP  )
		{
			return n;
		}
	}
	return -1;
}

BOOL CFileVersionInfo::SetTrans( IN LANGID wLID /*LANG_NEUTRAL*/,
								 IN WORD   wCP  /*WSLVI_CP_UNICODE*/ )
{
	if( m_bValid == FALSE )
		ASSERT_RETURN( FALSE );

	if( GetCurLID() == wLID && GetCurCP() == wCP )
		return TRUE;

	INT nPos = FindTrans( wLID, wCP );
	if( nPos != -1 ) m_nTransCur = nPos;

	return ( m_nTransCur == (UINT)nPos );
}

DWORD CFileVersionInfo::GetTransByIndex( IN UINT nIndex ) const
{
	if( m_bValid == FALSE || nIndex > m_nTransCnt )
		ASSERT_RETURN( 0 );

	return m_lpdwTrans[ nIndex ];
}

BOOL CFileVersionInfo::SetTransIndex( IN UINT nIndex /*0*/ )
{
	if( m_bValid == FALSE )
		ASSERT_RETURN( FALSE );

	if( m_nTransCur == nIndex )
		return TRUE;

	if(nIndex < m_nTransCnt )
		m_nTransCur = nIndex;

	return ( m_nTransCur == nIndex );
}

/////////////////////////////////////////////////////////////////////////////
// Static members

// If the LID identifier is unknown, it returns a
// default string ("Language Neutral"):

BOOL CFileVersionInfo::GetLIDName( IN  WORD   wLID,
								   OUT LPTSTR lpszName,
								   IN  INT    nBuf )
{
	if( lpszName == nullptr || nBuf <= 0 )
		ASSERT_RETURN( FALSE );

	return (BOOL)::VerLanguageName( wLID, lpszName, nBuf );
}

// If the CP identifier is unknown, it returns a
// default string ("Unknown"):

BOOL CFileVersionInfo::GetCPName( IN  WORD	   wCP,
								  OUT LPCTSTR* ppszName )
{
	if( ppszName == nullptr )
		ASSERT_RETURN( FALSE );

	BOOL bRes = TRUE;
	*ppszName  = nullptr;

	switch ( wCP )
	{
		case VI_CP_ASCII:	 *ppszName = _T( "7-bit ASCII" );				break;
		case VI_CP_JAPAN:	 *ppszName = _T( "Japan (Shift – JIS X-0208)" );break;
		case VI_CP_KOREA:	 *ppszName = _T( "Korea (Shift – KSC 5601)" );	break;
		case VI_CP_TAIWAN:	 *ppszName = _T( "Taiwan (Big5)" );				break;
		case VI_CP_UNICODE:	 *ppszName = _T( "Unicode" );					break;
		case VI_CP_LATIN2:	 *ppszName = _T( "Latin-2 (Eastern European)" );break;
		case VI_CP_CYRILLIC: *ppszName = _T( "Cyrillic" );					break;
		case VI_CP_MULTILNG: *ppszName = _T( "Multilingual" );				break;
		case VI_CP_GREEK:	 *ppszName = _T( "Greek" );						break;
		case VI_CP_TURKISH:	 *ppszName = _T( "Turkish" );					break;
		case VI_CP_HEBREW:	 *ppszName = _T( "Hebrew" );					break;
		case VI_CP_ARABIC:	 *ppszName = _T( "Arabic" );					break;
		default:			 *ppszName = _T( "Unknown" ); bRes = FALSE;		break;
	}
	return bRes;
}


/////////////////////////////////////////////////////////////////////////////