#include "Body.h"
#include <CCINIClass.h>

ObjectTypeExt::ExtContainer ObjectTypeExt::ExtMap;

ObjectTypeExt::ExtContainer::ExtContainer() : ExtensionWrapperAbract<ObjectTypeExt>("ObjectTypeClass") { }
ObjectTypeExt::ExtContainer::~ExtContainer() = default;
