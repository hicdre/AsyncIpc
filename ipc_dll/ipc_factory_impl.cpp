#include "stdafx.h"
#include "ipc_factory_impl.h"

namespace IPC
{
	FactoryImpl::FactoryImpl()
	{

	}

	FactoryImpl::~FactoryImpl()
	{
		for (auto iter : endpoint_map_)
		{
			iter.second->Release();
		}
	}


	IEndpoint* FactoryImpl::GetEndPoint(const char* name, IListener* listener)
	{
		if (name == NULL)
			return NULL;
		auto iter = endpoint_map_.find(name);
		if (iter != endpoint_map_.end())
			return iter->second;
		EndpointImpl* p = new EndpointImpl(name, listener);
		p->AddRef();
		endpoint_map_[name] = p;
		return p;
	}

}