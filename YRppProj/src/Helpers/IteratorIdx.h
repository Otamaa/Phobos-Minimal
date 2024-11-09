#pragma once
#include <concepts>

template<std::integral T>
class IterateIdx
{
private:
	bool forwardDir;
	T current;
	bool complete;
	T s;
public:

	IterateIdx(T size, bool forward) :
		s { size }
		, complete { false }
		, forwardDir { forwardDir }
		, current { forward ? 0 : size - 1 }
	{ }

	T operator ++(T)
	{

		if (forwardDir)
		{
			++current;
			if (current == s)
				complete = true;

			return current;
		}
		else
		{
			--current;

			if (current == -1)
				complete = true;

			return current;
		}
	}

	bool notComplete()
	{
		return !complete;
	}

	explicit operator T() const { return current; }
};