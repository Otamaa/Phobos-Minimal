#include "Body.h"
#include <CCINIClass.h>

AbstractTypeExt::ExtContainer AbstractTypeExt::ExtMap;

AbstractTypeExt::ExtContainer::ExtContainer() : ExtensionWrapperAbract<AbstractTypeExt>("AbstractTypeClass") { }
AbstractTypeExt::ExtContainer::~ExtContainer() = default;
