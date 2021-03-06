//
// ServiceRefWrapper.cpp
//
// $Id: //poco/1.4/OSP/JS/src/ServiceRefWrapper.cpp#5 $
//
// Copyright (c) 2013-2014, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier: Apache-2.0
//


#include "Poco/OSP/JS/ServiceRefWrapper.h"
#include "Poco/OSP/ServiceRef.h"
#include "Poco/JS/Bridge/BridgeWrapper.h"
#include "Poco/JS/Core/PooledIsolate.h"
#include "Poco/RemotingNG/ORB.h"
#include "Poco/RemotingNG/RemoteObject.h"
#include "Poco/Format.h"


namespace Poco {
namespace OSP {
namespace JS {


ServiceRefWrapper::ServiceRefWrapper()
{
}


ServiceRefWrapper::~ServiceRefWrapper()
{
}


v8::Handle<v8::ObjectTemplate> ServiceRefWrapper::objectTemplate(v8::Isolate* pIsolate)
{
	v8::EscapableHandleScope handleScope(pIsolate);
	Poco::JS::Core::PooledIsolate* pPooledIso = Poco::JS::Core::PooledIsolate::fromIsolate(pIsolate);
	poco_check_ptr (pPooledIso);
	v8::Persistent<v8::ObjectTemplate>& pooledObjectTemplate(pPooledIso->objectTemplate("OSP.ServiceRef"));
	if (pooledObjectTemplate.IsEmpty())
	{
		v8::Handle<v8::ObjectTemplate> objectTemplate = v8::ObjectTemplate::New();
		objectTemplate->SetInternalFieldCount(1);
		objectTemplate->SetNamedPropertyHandler(getProperty);
		objectTemplate->Set(v8::String::NewFromUtf8(pIsolate, "instance"), v8::FunctionTemplate::New(pIsolate, instance));
		pooledObjectTemplate.Reset(pIsolate, objectTemplate);
	}
	v8::Local<v8::ObjectTemplate> serviceRefTemplate = v8::Local<v8::ObjectTemplate>::New(pIsolate, pooledObjectTemplate);
	return handleScope.Escape(serviceRefTemplate);
}


void ServiceRefWrapper::getProperty(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	Poco::OSP::ServiceRef* pServiceRef = Poco::JS::Core::Wrapper::unwrapNative<Poco::OSP::ServiceRef>(info);

	try
	{
		std::string nm(toString(name));
		if (pServiceRef->properties().has(nm))
		{
			returnString(info, pServiceRef->properties().get(nm));
		}
	}
	catch (Poco::Exception& exc)
	{
		returnException(info, exc);
	}
}


void ServiceRefWrapper::instance(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Poco::OSP::ServiceRef* pServiceRef = Poco::JS::Core::Wrapper::unwrapNative<Poco::OSP::ServiceRef>(args);
	Poco::JS::Bridge::BridgeHolder::Ptr pHolder;
	try
	{
		if (pServiceRef->properties().has("jsbridge"))
		{
			pHolder = new Poco::JS::Bridge::BridgeHolder(args.GetIsolate(), pServiceRef->properties().get("jsbridge"));
			Poco::JS::Bridge::BridgeWrapper wrapper;
			v8::Persistent<v8::Object>& bridgeObject(wrapper.wrapNativePersistent(args.GetIsolate(), pHolder));
			pHolder->setPersistent(bridgeObject);
			args.GetReturnValue().Set(bridgeObject);
			return;
		}
		returnException(args, Poco::format("service instance for %s not accessible from JavaScript", pServiceRef->name()));
	}
	catch (Poco::Exception& exc)
	{
		returnException(args, exc);
	}
}


} } } // namespace Poco::OSP::JS
