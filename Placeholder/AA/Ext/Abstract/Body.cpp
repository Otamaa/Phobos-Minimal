#include "Body.h"
#include <CCINIClass.h>

AbstractExt::ExtContainer AbstractExt::ExtMap;

AbstractExt::ExtContainer::ExtContainer() : ExtensionWrapperAbract<AbstractExt>("AbstractClass") { }
AbstractExt::ExtContainer::~ExtContainer() = default;
