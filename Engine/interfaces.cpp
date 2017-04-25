#include "gui_interfaces.h"

#include <algorithm>


/*
std::vector<RendererInterface*> RendererInterface::renderers;



std::vector<Interface*> Interface::interfacesPool;

Interface::Interface()
{
	interfacesPool.push_back(this);
}

Interface::~Interface()
{
	interfacesPool.erase(std::remove(interfacesPool.begin(),interfacesPool.end(),this)/ *,interfacesPool.end()* /);
}
*/

std::vector<Gui*> Gui::guiPool;

Gui::Gui()
{
	guiPool.push_back(this);

	name="Generic Gui";
	tab=0;
}

Gui::~Gui()
{
	guiPool.erase(std::remove(guiPool.begin(),guiPool.end(),this),guiPool.end());
}

