#pragma once

#include <Syringe.h>

// In this file: Alternatives to CTRL + H...

// macros DCoder uses and pd dislikes :)

#define GET(clsname, var, reg) \
	clsname var = R->reg<clsname>();

#define GET8(clsname , var , reg) \
	clsname var = R->reg();

// it's really not a good idea to GET_STACK(not_a_pointer)
// no, really
#define LEA_STACK(clsname, var, offset) \
	clsname var = R->lea_Stack<clsname>(offset);

#define REF_STACK(clsname, var, offset) \
	clsname& var = R->ref_Stack<clsname>(offset);

#define GET_STACK(clsname, var, offset) \
	clsname var = R->Stack<clsname>(offset);

#define GET_BASE(clsname, var, offset) \
	clsname var = R->Base<clsname>(offset);

#define STACK_OFFS(cur_offset, wanted_offset) \
		(cur_offset - wanted_offset)

#define STACK_OFFSET(cur_offset, wanted_offset) \
		(cur_offset + wanted_offset)

// swizzle shorthand
#define SWIZZLE(var) \
	SwizzleManagerClass::Instance->Swizzle((void **)&var)


#include <cmath>
// float cmp
#define CLOSE_ENOUGH(x, y) \
	(fabs(x - y) < 0.001)

#define LESS_EQUAL(x, y) \
	((x - y) <= 0.001)

template<typename T>
__forceinline T &Make_Global(const uintptr_t address)
{
	return *reinterpret_cast<T *>(address);
}

template<typename T>
__forceinline T *Make_Pointer(const uintptr_t address)
{
	return reinterpret_cast<T *>(address);
}

template<typename T>
__forceinline auto Make_Pointer_B(const uintptr_t address)
{
	return *reinterpret_cast<T**>(address);
}

#define ARR_H(type , size ) ArrayHelper<type, size>
#define ARR_H_2(type , x , y ) ArrayHelper2D<type, x, y>
#define ARRAY_DEC(type, var, size) extern ARR_H(type,size) &var
#define ARRAY_DEF(address, type, var, size) ARR_H(type,size) &var = Make_Global<ARR_H(type,size)>(address);
#define ARRAY2D_DEC(type, var, x, y) extern ARR_H_2(type , x , y) &var
#define ARRAY2D_DEF(address, type, var, x, y) ARR_H_2(type , x , y) &var = Make_Global<ARR_H_2(type , x , y)>(address);

#define ALIAS_N(Base ,Type , Obj ,Addr)\
	Base::Type &Base::Obj = Make_Global<Base::Type>(Addr);

#define ALIAS(Type, Obj, Addr) \
	Type &Obj = Make_Global<Type>(Addr);

#define DECL(cls, adr) \
	DynamicVectorClass<cls*>* const cls::Array = \
		reinterpret_cast<DynamicVectorClass<cls*>*>(adr);

#define DECL_N(cls,Obj,adr) \
	DynamicVectorClass<cls*>* const cls::Obj = \
		reinterpret_cast<DynamicVectorClass<cls*>*>(adr);

#define ALIAS_O(Type, Obj, Addr) \
	Type Obj = Make_Pointer<Type>(Addr);