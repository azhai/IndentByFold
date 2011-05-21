//this file is part of IndentByFold
//Copyright (C)2011 Ben Bluemel ( ben1982@gmail.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "IBFPlugin.h"

extern IBFPlugin ibfplugin;

WNDPROC IBFPlugin::nppOriginalWndProc = NULL;

IBFPlugin::IBFPlugin() {}

IBFPlugin::~IBFPlugin() {}


FuncItem* IBFPlugin::nppGetFuncsArray( int* pnbFuncItems )
{
	*pnbFuncItems = ibfMenu::N_NBFUNCITEMS;
	return ibfMenu::arrFuncItems;
}
bool IBFPlugin::isNppWndUnicode = true;


LRESULT IBFPlugin::nppCallWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return ::CallWindowProcW( nppOriginalWndProc, hWnd, uMsg, wParam, lParam );
}


LRESULT CALLBACK IBFPlugin::nppNewWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return nppCallWndProc( hWnd, uMsg, wParam, lParam );
}


const TCHAR* IBFPlugin::nppGetName()
{
	//return PLUGIN_NAME;
	return 0;
}

void IBFPlugin::nppBeNotified( SCNotification* notifyCode )
{
	if ( notifyCode->nmhdr.hwndFrom == m_nppMsgr.getNppWnd() ) {
		// >>> notifications from Notepad++
		switch ( notifyCode->nmhdr.code ) {
		case NPPN_READY:
			OnNppReady();
			break;
		case NPPN_SHUTDOWN:
			OnNppShutdown();
			break;
		default:
			break;
		}
		
		// <<< notifications from Notepad++
	} else {
		// >>> notifications from Scintilla
		switch ( notifyCode->nmhdr.code ) {
		case SCN_UPDATEUI:
			break;
		case SCN_CHARADDED:
			OnSciCharAdded( notifyCode->ch );
			break;
		case SCN_AUTOCCANCELLED:
			break;
		case SCN_AUTOCSELECTION:
			break;
		case SCN_MODIFIED:
			break;
		default:
			break;
		}
		// <<< notifications from Scintilla
	}

}

void IBFPlugin::OnNppReady()
{
	if ( isNppWndUnicode ) {
		nppOriginalWndProc = ( WNDPROC ) SetWindowLongPtrW(
		                         m_nppMsgr.getNppWnd(), GWLP_WNDPROC, ( LONG_PTR ) nppNewWndProc );
	}
}

void IBFPlugin::OnNppShutdown()
{
	if ( nppOriginalWndProc ) {
		::SetWindowLongPtrW( m_nppMsgr.getNppWnd(),
			                    GWLP_WNDPROC, ( LONG_PTR ) nppOriginalWndProc );
	}
}



void IBFPlugin::OnSciCharAdded( int ch )
{
	
	CSciMessager sciMsgr( m_nppMsgr.getCurrentScintillaWnd() );
	// lets see if we can find one
	int currentpos = sciMsgr.getCurrentPos();
	int line = sciMsgr.getLineFromPos( currentpos );
	//int pos = sciMsgr.getCaretInLine();
	//int state = sciMsgr.getLineState( line -1 );
	//int wordstart = sciMsgr.SendSciMsg( SCI_WORDSTARTPOSITION,( WPARAM )currentpos, ( LPARAM ) true );
	int eol = sciMsgr.getEOLMode();
	if ( ch == '\n' || (eol == SC_EOL_CR && ch == '\r' ) ) {
		indentLine( line, false );
		int indentpos = sciMsgr.getLineIndentPosition( line );
		sciMsgr.goToPos( indentpos );
	}

}
void IBFPlugin::reindentFile()
{
	CSciMessager sciMsgr( m_nppMsgr.getCurrentScintillaWnd() );
	int linecount = sciMsgr.getLineCount();
	sciMsgr.setLineIndentation( 0, 0 );
	for ( int a = 1; a <= linecount; a++ ) {
		indentLine( a, true );
	}
}
void IBFPlugin::indentLine( int line, bool doingwholefile )
{
	CSciMessager sciMsgr( m_nppMsgr.getCurrentScintillaWnd() );
	int getwidth = sciMsgr.getTabWidth();
	int foldparentline = sciMsgr.getFoldParent( line );
	int foldlevellastline = sciMsgr.getFoldLevel( line -1 );
	int foldlevellast = foldlevellastline & SC_FOLDLEVELNUMBERMASK;
	int foldlevelcurline = sciMsgr.getFoldLevel( line );
	int foldlevel = foldlevelcurline & SC_FOLDLEVELNUMBERMASK;
	bool islastlinefoldparent = ( foldlevellastline & SC_FOLDLEVELHEADERFLAG) ? true : false;
	int lastlinelength = sciMsgr.getLineEndPos( line - 1 ) -  sciMsgr.getPosFromLine( line -1 );
	int lastlineindent = sciMsgr.getLineIndentation( line -1 ) / getwidth;

	// If the Fold Parent is the line above the new line then we indent
	if ( islastlinefoldparent && foldparentline != line && lastlinelength > lastlineindent ) {
		int indent = sciMsgr.getLineIndentation( foldparentline );
		sciMsgr.setLineIndentation( line, indent + getwidth );
	} else {
		
		int foldlevelprevtmp = sciMsgr.getFoldLevel( line -2 );
		int foldlevelprev = foldlevelprevtmp & SC_FOLDLEVELNUMBERMASK;
		bool isprevlinefoldparent = ( foldlevelprevtmp & SC_FOLDLEVELHEADERFLAG ) ? true : false;
		if ( isprevlinefoldparent && foldlevel == foldlevelprev && foldlevel != foldlevellast ) {
			int indent = sciMsgr.getLineIndentation( line -2 );
			sciMsgr.setLineIndentation( line -1, indent );
			sciMsgr.setLineIndentation( line, indent );
		} else if ( !isprevlinefoldparent && foldlevel < foldlevelprev && foldlevel != foldlevellast ) {
			int foldparentline = sciMsgr.getFoldParent( line -1 );
			int indent = sciMsgr.getLineIndentation( foldparentline );
			sciMsgr.setLineIndentation( line -1, indent );
			sciMsgr.setLineIndentation( line, indent );
		} else {
			int indent = sciMsgr.getLineIndentation( line -1 );
			sciMsgr.setLineIndentation( line, indent );
		}
	}
}


void IBFPlugin::OnNppSetInfo( const NppData& notpadPlusData )
{
	m_nppMsgr.setNppData( notpadPlusData );
	isNppWndUnicode = ::IsWindowUnicode( notpadPlusData._nppHandle ) ? true : false;
	

}
void IBFPlugin::aboutDlg()
{
	::MessageBox( m_nppMsgr.getNppWnd(),
	              TEXT( "IndentByFold by Ben Bluemel\n" )
	              TEXT( "http://bitbucket.org/bbluemel/indentbyfold/\n\n" ),
	              TEXT( "IndentByFold v0.5" ),
	              MB_OK );
				  
}