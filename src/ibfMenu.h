#ifndef _ibf_npp_plugin_menu_h_
#define _ibf_npp_plugin_menu_h_
//---------------------------------------------------------------------------
#include "core/NppPluginMenu.h"

#include "IBFPlugin.h"
class ibfMenu : public CNppPluginMenu
{
public:
	enum NMenuItems {
	    N_REINDENTFILE = 0,
	    N_SEPARATOR1,
	    N_ABOUT,

	    N_NBFUNCITEMS
	};
	static FuncItem arrFuncItems[N_NBFUNCITEMS];

public:
	static void		reIndentFile();
	static void		aboutDlg();
};
//---------------------------------------------------------------------------
#endif
