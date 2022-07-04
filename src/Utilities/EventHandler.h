#pragma once

#include <vector>

/*
	Author : secsome
*/

template<typename T>
class EventQueue
{
public:
	using function_type = void(*)(T*);
	using container_type = std::vector<function_type>;

	template<typename TFunc>
	EventQueue(std::initializer_list<TFunc> items)
	{
		for (auto item : items)
			insert(item);
	}

	EventQueue() : _events { }
		, curIdx { 0 }
		, AfterLoadGame { false }
	{ }

	bool operator() (function_type item)
	{
		insert(item);
		return true;
	}

	void insert(function_type value)
	{
		_events.push_back(value);
	}

	void operator +=(function_type value)
	{
		_events.push_back(value);
	}

	bool empty() const
	{
		return _events.empty();
	}

	void clear()
	{
		return _events.clear();
	}

	size_t size() const
	{
		return _events.size();
	}

	typename container_type::const_iterator begin() const
	{
		return _events.begin();
	}

	typename container_type::const_iterator end() const
	{
		return _events.end();
	}

	void run_each(T* pointer) const
	{

		//make everything concistent
		// so if the game loaded
		if (!AfterLoadGame) {
			for (size_t i = 0; i < _events.size(); ++i) {
				curIdx = i;
				_events.at(i)(pointer);
			}
		} else {
			for (size_t i = curIdx; i < _events.size(); ++i) {
				_events.at(i)(pointer);
			}

			AfterLoadGame = false;
		}
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		Debug::Log("Processing Element From EventQueue ! \n");

		if(Stm
			.Process(curIdx)
			.Success()){

			AfterLoadGame = true;
			return true;
		}

		return false;
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		Debug::Log("Processing Element From EventQueue ! \n");
		return
			Stm
			.Process(curIdx)
			.Success()
			;
	}

	mutable bool AfterLoadGame;
	mutable size_t curIdx;

private:
	container_type _events;
};
