//
// HumiditySensor.cpp
//
// $Id: //iot/Main/Tf/src/HumiditySensor.cpp#2 $
//
// Copyright (c) 2014, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#include "HumiditySensor.h"
#include "MasterConnectionImpl.h"


namespace IoT {
namespace Tf {


HumiditySensor::HumiditySensor(MasterConnection::Ptr pMasterConn, const std::string& uid):
	BrickletType("io.macchina.tf.humidity", "Tinkerforge Humidity Bricklet")
{
	IPConnection *ipcon = pMasterConn.cast<MasterConnectionImpl>()->ipcon();
	humidity_create(&_humidity, uid.c_str(), ipcon);
	
	char deviceUID[8];
	char masterUID[8];
	char position;
	Poco::UInt8 hardwareVersion[3];
	Poco::UInt8 firmwareVersion[3];
	Poco::UInt16 deviceType;
	if (humidity_get_identity(&_humidity, deviceUID, masterUID, &position, hardwareVersion, firmwareVersion, &deviceType) == E_OK)
	{
		setIdentity(deviceUID, masterUID, position, hardwareVersion, firmwareVersion, deviceType);
	}
	
	humidity_set_humidity_callback_period(&_humidity, 1000);
	humidity_register_callback(&_humidity, HUMIDITY_CALLBACK_HUMIDITY, reinterpret_cast<void*>(onHumidityChanged), this);
}

	
HumiditySensor::~HumiditySensor()
{
	humidity_destroy(&_humidity);
}


double HumiditySensor::value() const
{
	Poco::FastMutex::ScopedLock lock(_mutex);

	Poco::UInt16 humidity;
	int rc = humidity_get_humidity(&_humidity, &humidity);
	if (rc == E_OK)
	{
		return humidity/10.0;
	}
	else throw Poco::IOException();
}


Poco::Any HumiditySensor::getValueChangedPeriod(const std::string&) const
{
	Poco::UInt32 uperiod;
	humidity_get_humidity_callback_period(&_humidity, &uperiod);
	return Poco::Any(static_cast<int>(uperiod));
}


void HumiditySensor::setValueChangedPeriod(const std::string&, const Poco::Any& value)
{
	Poco::UInt32 period = static_cast<Poco::UInt32>(Poco::AnyCast<int>(value));
	humidity_set_humidity_callback_period(&_humidity, period);
}


void HumiditySensor::onHumidityChanged(Poco::UInt16 humidity, void* userData)
{
	try
	{
		HumiditySensor* pThis = reinterpret_cast<HumiditySensor*>(userData);
		pThis->valueChanged(humidity/10.0);
	}
	catch (...)
	{
	}
}


} } // namespace IoT::Tf
