// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ipc/ipc_platform_file.h"

namespace IPC {

PlatformFileForTransit GetFileHandleForProcess(HANDLE handle,
	HANDLE process,
                                               bool close_source_handle) {
  IPC::PlatformFileForTransit out_handle;
  DWORD options = DUPLICATE_SAME_ACCESS;
  if (close_source_handle)
    options |= DUPLICATE_CLOSE_SOURCE;
  if (handle == INVALID_HANDLE_VALUE ||
      !::DuplicateHandle(::GetCurrentProcess(),
                         handle,
                         process,
                         &out_handle,
                         0,
                         FALSE,
                         options)) {
    out_handle = IPC::InvalidPlatformFileForTransit();
  }
  return out_handle;
}

}  // namespace IPC
