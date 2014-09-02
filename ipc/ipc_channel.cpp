// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ipc/ipc_channel.h"

#include <limits>

#include "ipc/ipc_utils.h"

namespace {

// Global atomic used to guarantee channel IDs are unique.
StaticAtomicSequenceNumber g_last_id;

}  // namespace

namespace IPC {

// static
std::string Channel::GenerateUniqueRandomChannelID() {
  // Note: the string must start with the current process id, this is how
  // some child processes determine the pid of the parent.
  //
  // This is composed of a unique incremental identifier, the process ID of
  // the creator, an identifier for the child instance, and a strong random
  // component. The strong random component prevents other processes from
  // hijacking or squatting on predictable channel names.
  char buffer[64] = { 0 }; //10*3 + 2 + 1
  int process_id = ::GetCurrentProcessId();
  sprintf(buffer, "%d.%u.%d", process_id, g_last_id.GetNext(), RandInt(0, (std::numeric_limits<int32>::max)()));
  return std::string(buffer);
}

}  // namespace IPC

