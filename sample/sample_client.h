#pragma once

#include "ipc/ipc_endpoint.h"

#include "common.h"

#include <iostream>
#include <thread>

class SampleClient : public IPC::Listener
{
public:
	virtual bool OnMessageReceived(IPC::Message* msg);

	virtual void OnChannelConnected(int32 peer_pid);

	virtual void OnChannelError();
protected:
	int32 id_;
};
