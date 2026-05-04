/*
 * Copyright (C) 2022 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

HRESULT STDMETHODCALLTYPE ID3D12Resource_GetDevice(ID3D12Resource *pResource, REFIID riid, void **ppvDevice);
