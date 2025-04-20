#include "BarTypeClass.h"

Enumerable<BarTypeClass>::container_t Enumerable<BarTypeClass>::Array;

template<>
const char* Enumerable<BarTypeClass>::GetMainSection()
{
	return "BarTypes";
}

void BarTypeClass::LoadFromINI(CCINIClass* pINI)
{
	//const char* section = this->Name.c_str();

	INI_EX exINI(pINI);


}