#include <Common/EventSystems/EventSystem.h>

#include <Kratos.h>

class KratosHook
{
public:
	KratosHook()
	{
		EventSystems::General.AddHandler(Events::ExeRun, Kratos::ExeRun);
	}
};

static KratosHook _kratosHook;
