#pragma once

#include "ipc/ipc_interface.h"
#include "ipc/ipc_thread.h"
#include "ipc/ipc_listener.h"

#include "common.h"

#include <iostream>

class SampleDllClient : public IPC::IListener
{
public:
	SampleDllClient();
	~SampleDllClient();

	virtual void AddRef() const override;

	virtual void Release() const override;

	virtual void OnMessageReceived(const char* message, size_t len) override;

	virtual void OnDestConnected(int peer_pid) override;

	virtual void OnDestDisconnected() override;

	void Send(const std::string& cmd);
private:
	void Init();
	int id_;
	mutable LONG ref_count_;
	IPC::IEndpoint* endpoint_;
	HMODULE module_;
};

