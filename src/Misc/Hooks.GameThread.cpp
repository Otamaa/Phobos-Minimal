#include <MouseClass.h>
#include <WWMouseClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

int __stdcall Mouse_Thread(MouseThreadParameter* lpThreadParameter)
{

	lpThreadParameter->SomeState18 = 1;
	lpThreadParameter->SkipSleep = 0;

	if (lpThreadParameter->SkipProcessing)
	{
		Debug::Log("MouseThread SkipProcess Mutex !\n");
		lpThreadParameter->SkipSleep = 1;
	}
	else
	{
		Debug::Log("MouseThread Process Mutex !\n");
		for (; !lpThreadParameter->SkipProcessing; ++lpThreadParameter->RefCount)
		{
			if (Imports::WaitForSingleObject.get()(MouseThreadParameter::Mutex(), 10000u) == 258)
				Debug::Log("Warning: Probable deadlock occurred on MouseMutex. %s \n", __FUNCTION__);

			if (WWMouseClass::Thread_Instance())
				WWMouseClass::Thread_Instance()->Process();

			Imports::ReleaseMutex.get()(MouseThreadParameter::Mutex());
			Imports::Sleep.get()(lpThreadParameter->SleepTime);

		}
		lpThreadParameter->SkipSleep = 1;
	}

	return 0;
}

void __fastcall Phobos_HandleMouseThread()
{

	if (!MouseThreadParameter::Mutex())
	{
		Debug::Log("MouseThread Creating Mutex !\n");
		MouseThreadParameter::Mutex = Imports::CreateMutexA.get()(NULL, FALSE, NULL);
	}

	if (!MouseThreadParameter::ThreadNotActive())
	{
		if (MouseThreadParameter::Mutex())
		{
			Debug::Log("MouseThread Creating Thread !\n");
			if (auto const nThread = Imports::CreateThread.get()(NULL, 0x1000u, (LPTHREAD_START_ROUTINE)Mouse_Thread, &MouseThreadParameter::Thread(), 0, &MouseThreadParameter::ThreadID()))
			{
				MouseThreadParameter::Thread_Handle() = nThread;
				MouseThreadParameter::ThreadNotActive() = true;
				if (!Imports::SetThreadPriority.get()(nThread, 15))
				{
					char Buffer[1024];
					Imports::FormatMessageA.get()(0x1000u, 0, Imports::GetLastError.get()(), 0, Buffer, 0x400u, 0);
					Debug::Log("MouseThread Unable to change the priority - %s \n", Buffer);
					while (!MouseThreadParameter::Thread().SkipSleep)
					{
						Imports::Sleep.get()(0);
					}

					Imports::WaitForSingleObject.get()(MouseThreadParameter::Thread_Handle(), 5000u);
					Imports::CloseHandle.get()(MouseThreadParameter::Thread_Handle());
					MouseThreadParameter::ThreadNotActive() = false;
				}
			}
		}
	}

	Debug::Log("MouseThread Finish Creating Thread !\n");
}

DEFINE_JUMP(CALL,0x6BD849, GET_OFFSET(Phobos_HandleMouseThread));

#ifdef SYNC_DISPATCH_RESOURCE
DEFINE_HOOK(0x5D4E3B, DispatchingMessage_ReloadResources, 0x5)
{
	GET_STACK(tagMSG, pMsg, STACK_OFFS(0x2C, 0x1C));
	GET_STACK(DWORD, nDW, STACK_OFFS(0x2C, 0x40));

	if ((nDW == 0x10 || nDW == 0x2 || nDW == 0x112) && pMsg.wParam == (WPARAM)0xF060)
		ExitProcess(1u);

	//if ((nDW == 0x104 || nDW == 0x100) && pMsg.wParam == (WPARAM)0xD && ((pMsg.lParam & 0x20000000) != 0))
	//{
		//set critical section here ?
	//}

	Imports::TranslateMessage.get()(&pMsg);
	Imports::DispatchMessageA.get()(&pMsg);

	return 0x5D4E4D;
}
#endif