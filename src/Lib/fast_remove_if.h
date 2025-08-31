#pragma once

#include <vector>
#include <utility>
#include <Base/Always.h>

template<typename T, typename Tmem, typename Func>
COMPILETIMEEVAL bool FORCEINLINE fast_remove_if(std::vector<T , Tmem>& v, Func&& act)
{

	if (!v.size()) return false;

	typedef typename std::vector<T , Tmem>::value_type* ValuePtr;

	ValuePtr i = &v.front();
	ValuePtr last = &v.back();

	while (i <= last)
	{
		if (act(*i))
		{

			while (last != i && act(*last))
			{
				--last;
			}

			if (last != i) //do not move self into self, crash
				*i = std::move(*last);

			--last;
		}
		++i;
	}

	v.resize(last + 1 - &v[0]);
	return true;
}

template<typename T, typename Tmem, typename Func>
COMPILETIMEEVAL bool FORCEINLINE fast_remove_if(std::vector<T, Tmem>* v, Func&& act)
{

	if (!v->size()) return false;

	typedef typename std::vector<T>::value_type* ValuePtr;

	ValuePtr i = &v->front();
	ValuePtr last = &v->back();

	while (i <= last)
	{
		if (act(*i))
		{

			while (last != i && act(*last))
			{
				--last;
			}

			if (last != i) //do not move self into self, crash
				*i = std::move(*last);

			--last;
		}
		++i;
	}

	v->resize(last + 1 - &((*v)[0]));
	return true;
}