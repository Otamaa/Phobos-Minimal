#pragma once

#include <cstdio>

#ifdef _MSC_VER
#define _MSVC

/*
MSVC++
*/

#define PROLOG_THISCALL \
    _asm { push ecx } \
    _asm { push esp, ebp } \
    _asm { push ebp }

#define PROLOG_STDCALL \
    _asm { mov esp, ebp } \
    _asm { push ebp }

//Jump
#define JMP(address) \
	_asm{mov eax, address} \
	_asm{jmp eax}

#define JMP_REG(reg, address) \
    _asm { mov reg, address } \
    _asm { jmp reg }

#define JMP_REG_THIS(pThis,reg, address)\
	{_asm{mov ecx, pThis}\
	JMP_REG(reg,address);}

//MSVC++ stackframes
#define EPILOG_THISCALL \
	_asm{pop ecx} \
	_asm{mov esp, ebp} \
	_asm{pop ebp}

#define JMP_THIS(address) \
	{ EPILOG_THISCALL; \
	JMP(address); }

#define JMP_REG_THIS_(athis,address) \
	{ EPILOG_THISCALL; \
	JMP_REG_THIS(athis,eax,address); }

#define EPILOG_STDCALL \
	_asm{mov esp, ebp} \
	_asm{pop ebp}

#define JMP_STD(address) \
	{ EPILOG_STDCALL; \
	JMP(address); }

//Get/set register to a variable
#define GET_REG32(dst,reg) _asm{mov dst, reg}
#define SET_REG32(reg,src) _asm{mov reg, src}
#define GET_REGISTER_STATIC(dst, reg) static _asm { mov dst, reg }


#define GET_REGISTER_TYPE(type, dst, reg) type dst; _asm { mov dst, reg }
#define GET_REGISTER_STATIC_TYPE(type, dst, reg) static type dst; _asm { mov dst, reg }
#define GET_STACK_TYPE(type, dst, reg, off) type dst; _asm { mov eax, [reg+off] } _asm { mov dst, eax }
#define GET_STACK_STATIC_TYPE(type, dst, reg, off) static type dst; _asm { mov eax, [reg+off] } _asm { mov dst, eax }
#define LEA_STACK_STATIC_TYPE(type, dst, reg, off) static type dst; _asm { lea eax, [reg+off] } _asm { mov dst, eax }
#define GET_STACK_STATIC8_TYPE(type, dst, reg, off) static type dst; _asm { mov al, byte ptr [reg+off] } _asm { mov dst, al }
#define GET_STACK_STATIC16_TYPE(type, dst, reg, off) static type dst; _asm { mov ax, word ptr [reg+off] } _asm { mov dst, ax }

#define GET_STACKM(dst, reg, off) _asm { mov eax, [reg+off] } _asm { mov dst, eax }
#define GET_STACK_STATIC(dst, reg, off) static _asm { mov eax, [reg+off] } _asm { mov dst, eax }
#define GET_STACK_STATIC8(dst, reg, off) static _asm { mov al, byte ptr [reg+off] } _asm { mov dst, al }
#define GET_STACK_STATIC16(dst, reg, off) static _asm { mov ax, word ptr [reg+off] } _asm { mov dst, ax }

#define ZERO_REG(reg) _asm{xor reg, reg}

#define GET_REG8(dst,reg) _asm{mov dst, reg}
#define SET_REG8(reg,src) _asm{mov reg, src}

//Stack pointer operations
#define ADD_ESP(i) _asm{add esp, i}
#define SUB_ESP(i) _asm{sub esp, i}

//Push immediate value
#define PUSH_IMM(i) _asm{push i}

//Push / Pop register
#define PUSH_REG(r) _asm{push r}
#define POP_REG(r) _asm{pop r}

//Push a pointer to a variable
#define PUSH_PTR(v) \
		_asm{lea eax, v}\
		_asm{push eax}

//Push a variable
#define PUSH_VAR32(v) \
		_asm{mov eax, v}\
		_asm{push eax}

//Push 16bit variable
#define PUSH_VAR16(v) \
		_asm{movzx eax, v}\
		_asm{push eax}

//Push 8bit variable
#define PUSH_VAR8(v) \
		_asm{movzx eax, v}\
		_asm{push eax}

//Push 64bit variable
#define PUSH_VAR64(pv) \
		_asm{mov eax, pv}\
		_asm{mov ecx, [eax+4]}\
		_asm{mov eax, [eax]}\
		_asm{push ecx}\
		_asm{push eax}

//call
#define CALL(pFunction) \
		_asm{mov eax, pFunction}\
		_asm{call eax}

//THISCALL macros
#define THISCALL(pFunction) \
		_asm{mov ecx, this}\
		_asm{mov eax, pFunction}\
		_asm{call eax}

#define THISCALL_EX(pThis,pFunction) \
		_asm{mov ecx, pThis}\
		_asm{mov eax, pFunction}\
		_asm{call eax}

#define THISCALL_EXX(pFunction)\
		_asm{mov eax, pFunction}\
		_asm{call eax}

#define THISCALL_VT(vt_offs) \
		_asm{mov ecx, this}\
		_asm{mov eax, [ecx]}\
		_asm{call dword ptr [eax+vt_offs]}

#define CALL_VT(vt_offs) \
		_asm{mov eax, [ecx]}\
		_asm{call dword ptr [eax+vt_offs]}

#define THISCALL_EX_VT(pThis,vt_offs) \
		_asm{mov ecx, pThis}\
		_asm{mov edx, [ecx]}\
		_asm{call dword ptr [edx+vt_offs]}

//read or write memory
#define MEM_READ8(dst,mem) \
		_asm{mov dl, byte ptr ds:mem}\
		_asm{mov dst, dl}
#define MEM_WRITE8(mem,src) \
		_asm{mov dl, src}\
		_asm{mov byte ptr ds:mem, dl}

#define MEM_READ16(dst,mem) \
		_asm{mov dx, word ptr ds:mem}\
		_asm{mov dst, dx}
#define MEM_WRITE16(mem,src) \
		_asm{mov dx, src}\
		_asm{mov word ptr ds:mem, dx}

#define MEM_READ32(dst,mem) \
		_asm{mov edx, dword ptr ds:mem}\
		_asm{mov dst, edx}
#define MEM_WRITE32(mem,src) \
		_asm{mov edx, src}\
		_asm{mov dword ptr ds:mem, edx}

#define MEM_WRITEIMM8(mem,imm)	_asm{mov byte ptr ds:mem, imm}
#define MEM_WRITEIMM16(mem,imm)	_asm{mov word ptr ds:mem, imm}
#define MEM_WRITEIMM32(mem,imm)	_asm{mov dword ptr ds:mem, imm}

#define VAR32_REG(type,name,reg) type name;_asm{mov name, reg}
#define VAR8_REG(type,name,reg) type name;_asm{mov name, reg}

#define CONSTRUCTOR_EPILOG \
	_asm { pop edi } \
	_asm { pop esi } \
	_asm { pop ebx } \
	_asm { mov esp, ebp } \
	_asm { pop ebp }

#define DEFINE_IMPLEMENTATION(prototype, address, ...) \
    __declspec(noinline) __declspec(naked) \
    prototype \
    { \
        _asm { mov eax, address } \
        _asm { jmp eax } \
    }

#define DEFINE_IMPLEMENTATION_INLINE(prototype, address, ...) \
    inline __declspec(naked) \
    prototype \
    { \
        _asm { mov eax, address } \
        _asm { jmp eax } \
    }

#define DEFINE_IMPLEMENTATION_UNWIND(prototype, address, ...) \
    prototype \
    { \
        _asm { mov eax, address } \
        _asm { jmp eax } \
    }

#define DEFINE_IMPLEMENTATION_CONSTRUCTOR(prototype, address, ...) \
    __declspec(noinline) \
    prototype \
    { \
        _asm { mov ecx, this } \
	    _asm { mov esp, ebp } \
	    _asm { pop ebp } \
        _asm { mov eax, address } \
        _asm { jmp eax } \
    }

#define DEFINE_IMPLEMENTATION_CONSTRUCTOR_BASE(prototype, base, address, ...) \
   __declspec(noinline) \
    prototype : base(NoInitClass()) \
    { \
        _asm { mov ecx, this } \
	    _asm { mov esp, ebp } \
	    _asm { pop ebp } \
        _asm { mov eax, address } \
        _asm { jmp eax } \
    }

#define DESTRUCTOR_EPILOG \
	_asm { pop edi } \
	_asm { pop esi } \
	_asm { pop ebx } \
	_asm { mov esp, ebp } \
	_asm { pop ebp }

#define DEFINE_IMPLEMENTATION_DESTRUCTOR(prototype, address, ...) \
    __declspec(noinline) \
    prototype \
    { \
        DESTRUCTOR_EPILOG; \
        _asm { mov eax, address } \
        _asm { jmp eax } \
    }

#endif
