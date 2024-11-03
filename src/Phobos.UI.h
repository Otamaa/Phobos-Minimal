#pragma once

struct PhobosWindowClass
{
#ifdef EXPERIMENTAL_IMGUI
	static bool Create();
	static bool Destroy();

	static void Callback()
	{
		MessageHandler();
		Loop();
	}

	static bool TriggerList();
	static bool TeamList();
	static bool ScriptTypeList();

private:
	static void MessageHandler();
	static void Loop();
#endif
};