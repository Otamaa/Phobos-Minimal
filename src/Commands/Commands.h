#pragma once

#include <CommandClass.h>
#include <StringTable.h>
#include <MessageListClass.h>
#include <Phobos.h>

class PhobosCommandClass : public CommandClass
{
public:
	PhobosCommandClass() : CommandClass(), IsDeveloper(false), IsMultiplayerOnly(false) { }
	virtual ~PhobosCommandClass() { }

	//virtual KeyNumType Default_Key() const = 0;

	bool Developer_Only() const { return IsDeveloper; }
	bool Multiplayer_Only() const { return IsMultiplayerOnly; }

public:
	/**
	 *  Is this command only available in developer mode?
	 */
	bool IsDeveloper;

	/**
	 *  Is this command only available in multiplayer games?
	 */
	bool IsMultiplayerOnly;

protected:
	bool CheckDebugDeactivated() const {
		if (!Phobos::Config::DevelopmentCommands && !IsDeveloper && !Phobos::Otamaa::IsAdmin) {
			if (const wchar_t* text = StringTable::LoadString("TXT_COMMAND_DISABLED")) {
				wchar_t msg[0x100] = L"\0";
				wsprintfW(msg, text, this->GetUIName());
				MessageListClass::Instance->PrintMessage(msg);
			}
			return true;
		}
		return false;
	}
};

// will the templates ever stop? :D
template <typename T>
void Make() {
	T* command = GameCreate<T>();
	CommandClass::Array->AddItem(command);
};
