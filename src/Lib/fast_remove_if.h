#pragma once

#include <vector>
#include <utility>
#include <Base/Always.h>

template<typename T, typename Tmem, typename Func>
COMPILETIMEEVAL bool FORCEINLINE fast_remove_if(std::vector<T , Tmem>& v, Func&& act)
{
	auto iter = std::ranges::remove_if(v, act);

	if(iter.begin() != v.end()){
		v.erase(iter.begin(), v.end());
		return true;
	}

	return false;
}

template<typename T, typename Tmem, typename Func>
COMPILETIMEEVAL bool FORCEINLINE fast_remove_if(std::vector<T, Tmem>* v, Func&& act)
{
	auto iter = std::ranges::remove_if(*v , act);

	if(iter.begin() != v->end()){
		v->erase(iter.begin(), v->end());
		return true;
	}

	return false;
}