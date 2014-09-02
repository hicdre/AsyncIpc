// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_IPC_PLATFORM_FILE_H_
#define IPC_IPC_PLATFORM_FILE_H_

#include "ipc/ipc_common.h"


namespace IPC {

typedef HANDLE PlatformFileForTransit;

inline PlatformFileForTransit InvalidPlatformFileForTransit() {
  return INVALID_HANDLE_VALUE;
}

inline HANDLE PlatformFileForTransitToPlatformFile(
    const PlatformFileForTransit& transit) {
  return transit;
}

// Returns a file handle equivalent to |file| that can be used in |process|.
PlatformFileForTransit GetFileHandleForProcess(
	HANDLE file,
    HANDLE process,
    bool close_source_handle);

}  // namespace IPC

#endif  // IPC_IPC_PLATFORM_FILE_H_
