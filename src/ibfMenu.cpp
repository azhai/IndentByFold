#include "ibfMenu.h"

extern IBFPlugin ibfplugin;

FuncItem ibfMenu::arrFuncItems[N_NBFUNCITEMS] = {
	{ _T( "Reindent File" ), reIndentFile, 0, false, NULL },
	{ _T( "" ), NULL, 0, false, NULL }, // separator
	{ _T( "About" ), aboutDlg, 0, false, NULL }
};

void ibfMenu::reIndentFile()
{
	ibfplugin.reindentFile();
}

void ibfMenu::aboutDlg()
{
	ibfplugin.aboutDlg();
}

