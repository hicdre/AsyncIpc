// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_IPC_LISTENER_H_
#define IPC_IPC_LISTENER_H_

#include "ipc/ipc_common.h"

namespace IPC {

class Message;

// Implemented by consumers of a Channel to receive messages.
class Listener {
 public:
  // Called when a message is received.  Returns true iff the message was
  // handled.
  virtual bool OnMessageReceived(Message* message) = 0;

  // Called when the channel is connected and we have received the internal
  // Hello message from the peer.
  virtual void OnChannelConnected(int32 peer_pid) {}

  // Called when an error is detected that causes the channel to close.
  // This method is not called when a channel is closed normally.
  virtual void OnChannelError() {}

 protected:
  virtual ~Listener() {}
};

}  // namespace IPC

#endif  // IPC_IPC_LISTENER_H_
