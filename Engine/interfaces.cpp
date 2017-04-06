#include "gui_interfaces.h"

#include <algorithm>

std::vector<RendererInterface*> RendererInterface::renderers;

std::vector<GuiInterface*> GuiInterface::guiInterfacesPool;
std::vector<Interface*> Interface::interfacesPool;

Interface::Interface()
{
	interfacesPool.push_back(this);
}

Interface::~Interface()
{
	interfacesPool.erase(std::remove(interfacesPool.begin(),interfacesPool.end(),this)/*,interfacesPool.end()*/);
}


GuiInterface::GuiInterface():
tab(0)
{
	guiInterfacesPool.push_back(this);
}

GuiInterface::~GuiInterface()
{
	guiInterfacesPool.erase(std::remove(guiInterfacesPool.begin(),guiInterfacesPool.end(),this),guiInterfacesPool.end());
}

