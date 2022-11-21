#pragma once

#include <windows.h>
#include <atomic>

/*******************************************************************************
 *  @class   SimpleCriticalSectionClass
 *
 *  @brief   Simple critical sections are a thin cross platform wrapper around WinAPI
 *           critical sections or pthreads recursive mutexes which perform the same
 *           function and behave similarly. Create a SimpleCriticalSectionClass object
 *           that can be access by the threads that need shared access to some resource
 *           and have them call Enter to get a lock before manipulating the shared
 *           resource and then call Leave after it is done. Try_Enter can be used to
 *           test for a lock and selectively manipulate shared data based on if the lock
 *           is available.
 */
class SimpleCriticalSectionClass
{
public:
	SimpleCriticalSectionClass() : Handle() {
		InitializeCriticalSection(&Handle);
	}

	virtual ~SimpleCriticalSectionClass() {
		DeleteCriticalSection(&Handle);
	}

	void Enter() {
		EnterCriticalSection(&Handle);
	}

	bool Try_Enter() {
		return TryEnterCriticalSection(&Handle) != FALSE;
	}

	void Leave() {
		LeaveCriticalSection(&Handle);
	}

protected:
	SimpleCriticalSectionClass &operator=(const SimpleCriticalSectionClass &that) { return *this; }

private:
	CRITICAL_SECTION Handle;
};

class ScopedCriticalSectionClass
{
public:
	ScopedCriticalSectionClass(SimpleCriticalSectionClass *cs) :
		CritSection(cs)
	{
		if (cs != nullptr) {
			CritSection->Enter();
		}
	}

	virtual ~ScopedCriticalSectionClass()
	{
		if (CritSection != nullptr) {
			CritSection->Leave();
		}
	}

private:
	SimpleCriticalSectionClass *CritSection;
};

/*******************************************************************************
 *  @class   CriticalSectionClass
 *
 *  @brief   Critical section wraps WinAPI critical sections or pthreads recursive
 *           mutexes to create an automatically unlocking lock that can allow safe
 *           sharing of data between threads. To obtain a lock, a lock object is created
 *           with the a shared critical section object being passed to it. The lock
 *           object constructor will perform the lock and then the destructor will unlock
 *           it automatically when the lock object goes out of scope or the destructor
 *           is called manually as per RAII programming methodology.
 */
class CriticalSectionClass
{
public:
	CriticalSectionClass() :
		Handle(),
		Locked(0)
	{
		InitializeCriticalSection(&Handle);
	}
	~CriticalSectionClass() {
		DeleteCriticalSection(&Handle);
	}

	class LockClass
	{
	public:
		// In order to enter a critical section create a local instance of LockClass with critical section as a parameter.
		LockClass(CriticalSectionClass &critical_section) : CriticalSection(critical_section) { CriticalSection.Lock(); }
		~LockClass() { CriticalSection.Unlock(); }

	private:
		LockClass &operator=(const LockClass &that) { return *this; }
		CriticalSectionClass &CriticalSection;
	};

	friend class LockClass;

private:
	// Lock and unlock are private, you have to create a
	// CriticalSectionClass::LockClass object to call them instead.

	/**
	*  Performs the lock when entering a critical section of code. Private and is only called from the Lock object.
	*/
	void Lock() {
		EnterCriticalSection(&Handle);

		++Locked;
	}
	/**
	*  Removes the lock when leaving a critical section of code. Private and is only called from the Lock object.
	*/
	void Unlock() {
		LeaveCriticalSection(&Handle);

		--Locked;
	}


	bool Is_Locked() { return Locked > 0; }

private:
	CRITICAL_SECTION Handle;
	unsigned int Locked;
};

/*******************************************************************************
 *  @class   FastCriticalSectionClass
 *
 *  @brief   Fast critical section uses compiler intrinsics to create an automatically
 *           unlocking lock that can allow safe sharing of data between threads. It is
 *           used the same as the normal CriticalSectionClass but can only lock once
 *           and is an acquisition barrier only.
 */
class FastCriticalSectionClass
{
public:
	FastCriticalSectionClass() { Flag.clear(); }
	~FastCriticalSectionClass() {}

	class LockClass
	{
	public:
		LockClass(FastCriticalSectionClass &critical_section) : CriticalSection(critical_section)
		{
			CriticalSection.Thread_Safe_Set_Flag();
		}

		~LockClass() { CriticalSection.Thread_Safe_Clear_Flag(); }

	private:
		LockClass &operator=(const LockClass &that) { return *this; }
		FastCriticalSectionClass &CriticalSection;
	};

	friend class LockClass;

private:

	/**
	*  Performs the lock when entering a critical section of code. Private and is only called from the Lock object.
	*/
	void Thread_Safe_Set_Flag()
	{
		while (Flag.test_and_set(std::memory_order_seq_cst)) {

			/**
			 *  Yield the thread if no lock acquired.
			 */
			Sleep(1);
		}
	}

	/**
	*  Removes the lock when leaving a critical section of code. Private and is only called from the Lock object.
	*/
	void Thread_Safe_Clear_Flag() {
		Flag.clear();
	}

private:
	std::atomic_flag Flag;
};
