#pragma once

#include <vector>
#include <utility>
#include <Base/Always.h>

template<typename T, typename Tmem, typename Func>
COMPILETIMEEVAL bool FORCEINLINE fast_remove_if(std::vector<T , Tmem>& v, Func&& act)
{
	auto iter = std::remove_if(v.begin(), v.end() , act);

	if(iter != v.end()){
		v.erase(iter, v.end());
		return true;
	}

	return false;
}

template<typename T, typename Tmem, typename Func>
COMPILETIMEEVAL bool FORCEINLINE fast_remove_if(std::vector<T, Tmem>* v, Func&& act)
{
	auto iter = std::remove_if(v->begin(), v->end() , act);

	if(iter != v->end()){
		v->erase(iter, v->end());
		return true;
	}

	return false;
}