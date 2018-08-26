#include <vgui/ILocalize.h>
#include <vgui_controls/HTML.h>
#include <vgui_controls/Label.h>
#include "FileSystem.h"
#include "strtools.h"

#include "VGUI2Paths.h"

#include "IGameUIFuncs.h"
#include "CClientVGUI.h"
#include "vgui2/CBaseViewport.h"

#include "CClientMOTD.h"

using namespace vgui2;

class CClientMOTDHTML : public vgui2::HTML
{
public:
	using vgui2::HTML::HTML;
};

CClientMOTD::CClientMOTD( IViewport* pParent )
	: BaseClass( nullptr, "ClientMOTD" )
	, m_pViewport( pParent )
	, m_bFileWritten( false )
	, m_iScoreBoardKey( 0 )
{
	//Sanity check.
	Assert( ARRAYSIZE( m_szTempFileName ) > strlen( "motd_temp.html" ) );
	strcpy( m_szTempFileName, "motd_temp.html" );

	SetTitle( "", true );
	SetScheme( "ClientScheme" );
	SetMoveable( false );
	SetProportional( true );

	m_pMessage = new CClientMOTDHTML( this, "Message" );

	LoadControlSettings( UI_RESOURCE_DIR "/MOTD.res" );
	InvalidateLayout();

	m_pServerName = new vgui2::Label( this, "serverName", "" );

	SetVisible( false );
}

CClientMOTD::~CClientMOTD()
{
	RemoveTempFile();
}

void CClientMOTD::SetLabelText( const char* textEntryName, const wchar_t* text )
{
	vgui2::Panel* pChild = FindChildByName( textEntryName );

	if( pChild )
	{
		auto pLabel = dynamic_cast<vgui2::Label*>( pChild );

		if( pLabel )
			pLabel->SetText( text );
	}
}

bool CClientMOTD::IsURL( const char* str )
{
	//TODO: https support
	return strncmp( str, "http://", 7 ) == 0;
}

void CClientMOTD::OnKeyCodeTyped( vgui2::KeyCode key )
{
	if( key == KEY_PAD_ENTER || key == KEY_ENTER )
	{
		OnCommand( "okay" );
	}
	else
	{
		if( m_iScoreBoardKey != KEY_NONE && m_iScoreBoardKey == key )
		{
			//TODO
			//if( !gViewPort->IsScoreBoardVisible() )
			{
				g_pViewport->ShowBackGround( false );
				//g_pViewport->ShowScoreBoard();
				SetVisible( false );
			}
		}
		else
		{
			BaseClass::OnKeyCodeTyped( key );
		}
	}
}

void CClientMOTD::OnCommand( const char* command )
{
	if( !stricmp( command, "okay" ) )
	{
		RemoveTempFile();

		Close();
	}

	BaseClass::OnCommand( command );
}

void CClientMOTD::Close()
{
	BaseClass::Close();

	m_pViewport->ShowBackGround( false );
}

void CClientMOTD::Activate( const char* title, const char* msg )
{
	char localURL[ MAX_HTML_FILENAME_LENGTH + 7 ];

	BaseClass::Activate();

	SetTitle( title, false );
	SetControlString( "serverName", title );

	const char* pszURL = msg;

	if( !IsURL( msg ) )
	{
		pszURL = nullptr;
	
		RemoveTempFile();
	
		if( !strstr( msg, "img src=\"view-source:" ) && !strstr( msg, "<style>;@/*" ) )
		{
			FileHandle_t hFile = filesystem()->Open( m_szTempFileName, "w+", "GAMECONFIG" );
	
			if( hFile != FILESYSTEM_INVALID_HANDLE )
			{
				filesystem()->Write( msg, strlen( msg ), hFile );
				filesystem()->Close( hFile );
	
				strcpy( localURL, "file:///" );
	
				const size_t uiURLLength = strlen( localURL );
				filesystem()->GetLocalPath( m_szTempFileName, localURL + uiURLLength, sizeof( localURL ) - uiURLLength );
	
				pszURL = localURL;
			}
		}
	}

	if( pszURL )
		m_pMessage->OpenURL( pszURL, nullptr );

	if( m_iScoreBoardKey == KEY_NONE )
		m_iScoreBoardKey = gameUIFuncs()->GetVGUI2KeyCodeForBind( "showscores" );
}

void CClientMOTD::Activate( const wchar_t* title, const wchar_t* msg )
{
	char localURL[ MAX_HTML_FILENAME_LENGTH + 7 ];
	char ansiURL[ MAX_PATH ];

	BaseClass::Activate();

	SetTitle( title, false );
	m_pServerName->SetText( title );

	localize()->ConvertUnicodeToANSI( msg, ansiURL, sizeof( ansiURL ) );

	if( IsURL( ansiURL ) )
	{
		m_pMessage->OpenURL( ansiURL, nullptr );
	}
	else
	{
		RemoveTempFile();

		FileHandle_t hFile = filesystem()->Open( m_szTempFileName, "w+", "GAMECONFIG" );

		if( hFile )
		{
			//Note: CZero doesn't multiply by sizeof( wchar_t ) - Solokiller
			filesystem()->Write( msg, wcslen( msg ) * sizeof( wchar_t ), hFile );
			filesystem()->Close( hFile );

			strcpy( localURL, "file:///" );
			const size_t uiURLLength = strlen( localURL );

			filesystem()->GetLocalPath( m_szTempFileName, localURL + uiURLLength, sizeof( localURL ) - uiURLLength );
			m_pMessage->OpenURL( localURL, nullptr );
		}
	}

	SetVisible( true );
}

void CClientMOTD::Reset()
{
	m_pMessage->OpenURL( "", nullptr );

	RemoveTempFile();

	m_pServerName->SetText( "" );
}

void CClientMOTD::ShowPanel( bool state )
{
	if( BaseClass::IsVisible() == state )
		return;

	m_pViewport->ShowBackGround( state );

	if( state )
	{
		Reset();
		Update();

		BaseClass::Activate();
	}
	else
	{
		BaseClass::SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}
}

void CClientMOTD::RemoveTempFile()
{
	if( filesystem()->FileExists( m_szTempFileName ) )
	{
		filesystem()->RemoveFile( m_szTempFileName, "GAMECONFIG" );
	}
}
