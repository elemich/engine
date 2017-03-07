#include "gui_interfaces.h"

std::vector<RendererInterface*> RendererInterface::renderers;

std::vector<GuiInterface*> GuiInterface::guiInterfacesPool;
std::vector<Interface*> Interface::interfacesPool;

std::vector<EntityInterface*> EntityInterface::entityInterfacesPool;

std::vector<ResourceInterface*> ResourceInterface::resourceInterfacesPool;
