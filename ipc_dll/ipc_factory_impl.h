#pragma once
#include "ipc/ipc_interface.h"
#include "ipc_endpoint_impl.h"
#include <unordered_map>

namespace IPC
{
	class FactoryImpl
	{
	public:
		FactoryImpl();
		~FactoryImpl();
		IEndpoint* GetEndPoint(const char* name, IListener* listener);
	private:
		std::unordered_map<std::string, EndpointImpl*> endpoint_map_;
	};
}