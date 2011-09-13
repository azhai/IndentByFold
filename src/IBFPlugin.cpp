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

void IBFPlugin::OnLangChanged( uptr_t idFrom ) 
{
	LangType langType = L_TEXT;
	useNextLine = false;
	m_nppMsgr.SendNppMsg( NPPM_GETCURRENTLANGTYPE, (WPARAM) 0, (LPARAM) & langType );
	if ( langType == L_RUBY || langType == L_HTML || langType == L_LISP || langType == L_LUA  || langType == L_PASCAL || langType == L_XML ) {
		useNextLine = true;
	}
}
void IBFPlugin::nppBeNotified( SCNotification* notifyCode )
{
	if ( notifyCode->nmhdr.hwndFrom == m_nppMsgr.getNppWnd() ) {
		// >>> notifications from Notepad++
		switch ( notifyCode->nmhdr.code ) {
		case NPPN_BUFFERACTIVATED:
		case NPPN_FILESAVED:
		case NPPN_LANGCHANGED:
			OnLangChanged( notifyCode->nmhdr.idFrom );
			break;


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
			if ( notifyCode->modificationType & SC_MOD_CHANGEFOLD ) {
				int foldlevelNowMask = notifyCode->foldLevelNow;
				bool foldlevelnowishead = false;
				if ( foldlevelNowMask & SC_FOLDLEVELWHITEFLAG ) {
					foldlevelNowMask = foldlevelNowMask &~SC_FOLDLEVELWHITEFLAG;
				}
				if ( foldlevelNowMask & SC_FOLDLEVELHEADERFLAG ) {
					foldlevelnowishead = true;
					foldlevelNowMask = foldlevelNowMask &~SC_FOLDLEVELHEADERFLAG;
				}
				int foldlevelNowMask2 = foldlevelNowMask & SC_FOLDLEVELNUMBERMASK ;

				int foldlevelPrevMask = notifyCode->foldLevelPrev;
				if ( foldlevelPrevMask & SC_FOLDLEVELWHITEFLAG ) {
					foldlevelPrevMask = foldlevelPrevMask &~SC_FOLDLEVELWHITEFLAG;
				}
				if ( foldlevelPrevMask & SC_FOLDLEVELHEADERFLAG ) {
					foldlevelPrevMask = foldlevelPrevMask &~SC_FOLDLEVELHEADERFLAG;
				}
				int foldlevelPrevMask2 = foldlevelPrevMask  & SC_FOLDLEVELNUMBERMASK;
				bool shifted = false;
				if (foldlevelNowMask2 == foldlevelPrevMask2 && foldlevelNowMask != foldlevelPrevMask) {
					shifted = true;
					foldlevelNowMask = ( notifyCode->foldLevelNow >> 16);
					foldlevelPrevMask = ( notifyCode->foldLevelPrev >> 16);
				} else {
					foldlevelNowMask = foldlevelNowMask2;
					foldlevelPrevMask = foldlevelPrevMask2;
				}
				
				if ( foldlevelNowMask < foldlevelPrevMask ) {
					CSciMessager sciMsgr( m_nppMsgr.getCurrentScintillaWnd() );
					int curline =  sciMsgr.getLineFromPos(sciMsgr.getCurrentPos());
					if ( curline == notifyCode->line || ( useNextLine && curline == notifyCode->line - 1 ) ) {
						int actualline = notifyCode->line;
						if ( curline == notifyCode->line - 1) {
							actualline--;
						}
						
						int foldparentline = sciMsgr.getFoldParent( actualline );
						toggleDownUpLine = -1;
						if (foldlevelnowishead) {
							foldparentline = sciMsgr.getFoldParent( actualline -1 );
						}
						int foldlevelparent = sciMsgr.getFoldLevel( foldparentline );
					
						if ( shifted ) {
							foldlevelparent = foldlevelparent >> 16;
						} else {
							foldlevelparent = foldlevelparent & SC_FOLDLEVELNUMBERMASK;
						}
						// The or here is for nppCF with cfelse/cfelseif
						if (	foldlevelparent == foldlevelPrevMask || 
								( foldlevelNowMask == foldlevelparent && foldlevelnowishead ) ||
								( actualline == notifyCode->line -1 && foldlevelparent == foldlevelPrevMask -1 )
								) {
							int indent = sciMsgr.getLineIndentation( foldparentline );
							sciMsgr.setLineIndentation( actualline, indent );
							lastFoldDownLine = curline;
						}
					}
				} else if ( isNewLine == true && newLine == notifyCode->line ) {
					CSciMessager sciMsgr( m_nppMsgr.getCurrentScintillaWnd() );
					isNewLine = false;
					newLine = -1;
					indentLine( notifyCode->line, false );
					int indentpos = sciMsgr.getLineIndentPosition( notifyCode->line );
					sciMsgr.goToPos( indentpos );
				} else if ( lastFoldDownLine > -1 && foldlevelNowMask > foldlevelPrevMask ) {
					CSciMessager sciMsgr( m_nppMsgr.getCurrentScintillaWnd() );
					int curline =  sciMsgr.getLineFromPos(sciMsgr.getCurrentPos());
					if ( curline == lastFoldDownLine ) {
						toggleDownUpLine = curline;
					}
				}
			}
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
	int eol = sciMsgr.getEOLMode();
	if ( ch == '\n' || (eol == SC_EOL_CR && ch == '\r' ) ) {
		indentLine( line, false );
		isNewLine = true;
		newLine = line;
		int indentpos = sciMsgr.getLineIndentPosition( line );
		sciMsgr.goToPos( indentpos );
		
	} else {
		isNewLine = false;
		newLine = -1;
	
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
	if ( !islastlinefoldparent && toggleDownUpLine == line -1 ) {
		islastlinefoldparent = true;
	}
	// If the Fold Parent is the line above the new line then we indent
	if ( islastlinefoldparent && foldparentline != line && lastlinelength > lastlineindent ) {
		int indent = sciMsgr.getLineIndentation( foldparentline );
		sciMsgr.setLineIndentation( line, indent + getwidth );
	} else if ( doingwholefile && foldlevellast > foldlevel ) {
		int foldparentline = sciMsgr.getFoldParent( line -1 );
		int indent = sciMsgr.getLineIndentation( foldparentline );
		sciMsgr.setLineIndentation( line -1, indent );
		sciMsgr.setLineIndentation( line, indent );
	} else {
		int indent = sciMsgr.getLineIndentation( line -1 );
		sciMsgr.setLineIndentation( line, indent );
	}
}


void IBFPlugin::OnNppSetInfo( const NppData& notpadPlusData )
{
	m_nppMsgr.setNppData( notpadPlusData );
	isNppWndUnicode = ::IsWindowUnicode( notpadPlusData._nppHandle ) ? true : false;
	lastFoldDownLine = -1;
	toggleDownUpLine = -1;
	newLine = -1;
	isNewLine = false;

}
void IBFPlugin::aboutDlg()
{
	::MessageBox( m_nppMsgr.getNppWnd(),
	              TEXT( "IndentByFold by Ben Bluemel\n" )
	              TEXT( "http://bitbucket.org/bbluemel/indentbyfold/\n\n" ),
	              TEXT( "IndentByFold v0.6" ),
	              MB_OK );
				  
}