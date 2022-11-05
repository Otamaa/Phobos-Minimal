#include "Body.h"


ObjectExt::ExtContainer ObjectExt::ExtMap;

ObjectExt::ExtContainer::ExtContainer() : ExtensionWrapperAbract<ObjectExt>("ObjectClass") { }
ObjectExt::ExtContainer::~ExtContainer() = default;
