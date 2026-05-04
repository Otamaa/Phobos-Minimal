/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

#include "opengl_impl_device.hpp"
#include "opengl_impl_device_context.hpp"
#include "opengl_impl_type_convert.hpp"
#include "opengl_hooks.hpp"

#include "../../Hooks/hook_manager.hpp"

#include <cstring> // std::memset, std::strlen

#define gl static_cast<reshade::opengl::device_impl *>(g_opengl_context->get_device())->_dispatch_table

// Initialize thread local variable in this translation unit, to avoid the compiler generating calls to '__dyn_tls_on_demand_init' on every use in the frequently called functions below
thread_local reshade::opengl::device_context_impl *g_opengl_context = nullptr;

void APIENTRY glBindProgramARB(GLenum target, GLuint program)
{
	static const auto trampoline = reshade::hooks::call(glBindProgramARB);
	trampoline(target, program);
}
void APIENTRY glProgramStringARB(GLenum target, GLenum format, GLsizei length, const GLvoid *string)
{
	static const auto trampoline = reshade::hooks::call(glProgramStringARB);
	trampoline(target, format, length, string);
}
void APIENTRY glDeleteProgramsARB(GLsizei n, const GLuint *programs)
{
	static const auto trampoline = reshade::hooks::call(glDeleteProgramsARB);
	trampoline(n, programs);
}

void APIENTRY glBindFramebufferEXT(GLenum target, GLuint framebuffer)
{
	static const auto trampoline = reshade::hooks::call(glBindFramebufferEXT);
	trampoline(target, framebuffer);
}

void APIENTRY glBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture)
{
	static const auto trampoline = reshade::hooks::call(glBindMultiTextureEXT);
	trampoline(texunit, target, texture);
}
