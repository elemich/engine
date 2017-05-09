#include "gui_interfaces.h"
#include "win32.h"

#include <algorithm>

bool Gui::IsSelected(){return (tab && tab->GetSelected()==this);}

