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


#include "DllEntry.h"
IBFPlugin ibfplugin;
ibfMenu ibfmenu;
BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD reasonForCall, LPVOID /*lpReserved*/ )
{
	switch ( reasonForCall ) {
	case DLL_PROCESS_ATTACH:
		ibfplugin.OnDllProcessAttach( hModule );
		break;
	case DLL_PROCESS_DETACH:
		ibfplugin.OnDllProcessDetach();
		break;
	}
	return TRUE;
}

extern "C" {

#ifdef UNICODE
	__declspec( dllexport ) BOOL isUnicode()
	{
		return TRUE;
	}
#endif

	__declspec( dllexport ) const TCHAR * getName()
	{
		return PLUGIN_NAME.c_str();
	}
	__declspec( dllexport ) void setInfo( NppData notpadPlusData )
	{
		ibfplugin.nppSetInfo( notpadPlusData );
		ibfmenu.setNppData( notpadPlusData );
	}

	__declspec( dllexport ) FuncItem * getFuncsArray( int *nbF )
	{

		return ibfplugin.nppGetFuncsArray( nbF );

	}

	__declspec( dllexport ) void beNotified( SCNotification * notifyCode )
	{
		ibfplugin.nppBeNotified( notifyCode );

	}
	__declspec( dllexport ) LRESULT messageProc( UINT Message, WPARAM wParam, LPARAM lParam )
	{
		return ibfplugin.nppMessageProc( Message, wParam, lParam );
	}


} // End extern "C"
