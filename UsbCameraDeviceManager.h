// UsbCameraDeviceManager.h
// Uses Windows SetupAPI to disable/enable camera device and/or camera's parent USB Composite Device in Device Manager
//
// Copyright (c) 2016-2019 Matthew Breit - matt.breit@baslerweb.com or matt.breit@gmail.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef USBCAMERADEVICEMANAGER_H
#define USBCAMERADEVICEMANAGER_H

#ifndef LINUX_BUILD
#define WIN_BUILD
#endif

#ifdef WIN_BUILD
#include <pylon\PylonIncludes.h>
#include <iostream>
#include <windows.h>
#include <powersetting.h>
#include <powrprof.h>
#include <setupapi.h>   
#include <tchar.h>   
#include <initguid.h>
#include <devguid.h>
#include <cfgmgr32.h>
#pragma comment(lib,"ws2_32.lib")   
#pragma comment(lib,"setupapi.lib")   
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "PowrProf.lib")
#define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof(arr[0]))

DEFINE_GUID(GUID_USB_SUBGROUP, 0x2a737441, 0x1930, 0x4402, 0x8d, 0x77, 0xb2, 0xbe, 0xbb, 0xa3, 0x08, 0xa3);
DEFINE_GUID(GUID_USB_SELECTIVE_SUSPEND, 0x48e6b7a6, 0x50f5, 0x4782, 0xa5, 0xd4, 0x53, 0xbb, 0x8f, 0x07, 0xe2, 0x26);
DEFINE_GUID(GUID_USB3_LINK_POWER_MANAGEMENT, 0xd4e98f31, 0x5ffe, 0x4ce1, 0xbe, 0x31, 0x1b, 0x38, 0xb3, 0x84, 0xc0, 0x09);

#ifdef DEFINE_DEVPROPKEY
#undef DEFINE_DEVPROPKEY
#endif
#ifdef INITGUID
#define DEFINE_DEVPROPKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const DEVPROPKEY DECLSPEC_SELECTANY name = { { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }, pid }
#else
#define DEFINE_DEVPROPKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const DEVPROPKEY name
#endif

DEFINE_DEVPROPKEY(DEVPKEY_Device_BusReportedDeviceDesc, 0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 4);     // DEVPROP_TYPE_STRING
DEFINE_DEVPROPKEY(DEVPKEY_Device_ContainerId, 0x8c7ed206, 0x3f8a, 0x4827, 0xb3, 0xab, 0xae, 0x9e, 0x1f, 0xae, 0xfc, 0x6c, 2);     // DEVPROP_TYPE_GUID
DEFINE_DEVPROPKEY(DEVPKEY_Device_FriendlyName, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 14);    // DEVPROP_TYPE_STRING
DEFINE_DEVPROPKEY(DEVPKEY_DeviceDisplay_Category, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 0x5a);  // DEVPROP_TYPE_STRING_LIST
DEFINE_DEVPROPKEY(DEVPKEY_Device_LocationInfo, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 15);    // DEVPROP_TYPE_STRING
DEFINE_DEVPROPKEY(DEVPKEY_Device_Manufacturer, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 13);    // DEVPROP_TYPE_STRING
DEFINE_DEVPROPKEY(DEVPKEY_Device_SecuritySDS, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 26);    // DEVPROP_TYPE_SECURITY_DESCRIPTOR_STRING


namespace UsbCameraDeviceManager
{
	class CUsbCameraDeviceManager
	{
	private:
		std::string m_serialNumber;
		std::string m_productID;
		std::string m_deviceInstance;
		std::string m_compositeDeviceInstance;
		std::string m_errorMessage;
		std::string m_activePowerSchemeName;
		std::string m_unknownDeviceInstance;
		std::string m_unknownDeviceDescription;
		std::string m_unknownDeviceParentInstance;
		std::string m_unknownDeviceParentDescription;
		int m_hiberBootEnabled;
		int m_UsbSelectiveSuspendIsEnabledAC;
		int m_Usb3LinkPowerManagmentIsEnabledAC;
		int m_UsbSelectiveSuspendIsEnabledDC;
		int m_Usb3LinkPowerManagmentIsEnabledDC;
		std::vector<std::string> m_deviceNames;
		std::vector<std::string> m_devicePowerStates;

	public:
		CUsbCameraDeviceManager();

		~CUsbCameraDeviceManager();

		// Get a list of all the attached usb devices
		static bool ListAllUsbDevices(std::vector<std::string> &deviceInstanceIDs, std::vector<std::string> &deviceDescriptions);

		static bool FindDeviceInstanceID(std::string &keyWordInDescription, std::string &deviceInstanceID, std::string &fullDeviceDescription);

		static bool GetParentDeviceInstanceID(std::string &deviceInstanceID, std::string &parentDeviceInstanceID);

		static bool GetDeviceDescription(std::string &deviceInstanceID, std::string &deviceDescription);

		// Enables the camera device like in Windows Device Manager
		static bool EnableDevice(std::string deviceInstanceID, std::string &errorMessage);

		// Disables the camera device like in Windows Device Manager
		static bool DisableDevice(std::string deviceInstanceID, std::string &errorMessage);

		bool EnableCamera();

		bool DisableCamera();

		// Collect the power settings of the host controller
		bool ReadPowerSchemeSettings();

		// Get the name of the active power scheme
		std::string GetActivePowerSchemeName();

		int UsbCameraDeviceManager::CUsbCameraDeviceManager::IsSelectiveSuspendEnabledOnBattery();

		int UsbCameraDeviceManager::CUsbCameraDeviceManager::IsSelectiveSuspendEnabledWhenPluggedIn();

		int UsbCameraDeviceManager::CUsbCameraDeviceManager::IsUsb3LinkPowerManagementEnabledOnBattery();

		int UsbCameraDeviceManager::CUsbCameraDeviceManager::IsUsb3LinkPowerManagementEnabledWhenPluggedIn();

		// Is windows "Fast Boot" enabled?
		int IsEnabledFastBoot();

		// Reads the camera's Full Name and constructs the needed ID tags for finding it in the system.
		bool InitializeFromCamera(std::string serialNumber);
		
		// Enables the camera device's parent USB Composite Device like in Windows Device Manager
		bool EnableCameraCompositeDevice();

		// Disables the camera device's parent USB Composite Device like in Windows Device Manager
		bool DisableCameraCompositeDevice();

		// This will check the status of the "allow computer to turn off this device..." field in device manager
		bool ReadDeviceTreePowerStates();

		// Get the list of device names in the device tree
		std::vector<std::string> GetCameraTreeDeviceNames();

		// Get the list of the device tree devices' power states
		std::vector<std::string> GetCameraTreeDevicePowerStates();

		// For reference, the user can see if the camera is currently connected as USB2 or USB3.
		std::string GetUsbConnectionType();

		// For reference, the user can see the camera device instance string
		std::string GetDeviceInstanceID();

		// For reference, the user can see the camera device's parent USB Composite Device instance string
		std::string GetCompositeDeviceInstanceID();

		// For reference, the user can see the camera device's product ID tag.
		std::string GetProductID();

		// For reference, the user can see the last error message.
		std::string GetLastErrorMessage();
	};
}


// *********************************************************************************************************
// DEFINITIONS

UsbCameraDeviceManager::CUsbCameraDeviceManager::CUsbCameraDeviceManager()
{
	m_serialNumber = "";
	m_productID = "";
	m_deviceInstance = "";
	m_compositeDeviceInstance = "";
	m_errorMessage = "";
	m_activePowerSchemeName = "UNKNOWN";
	m_hiberBootEnabled = -1;
	m_UsbSelectiveSuspendIsEnabledAC = -1;
	m_Usb3LinkPowerManagmentIsEnabledAC = -1;
	m_UsbSelectiveSuspendIsEnabledDC = -1;
	m_Usb3LinkPowerManagmentIsEnabledDC = -1;
	m_unknownDeviceDescription = "";
	m_unknownDeviceInstance = "";
	m_unknownDeviceParentDescription = "";
	m_unknownDeviceParentInstance = "";
}

UsbCameraDeviceManager::CUsbCameraDeviceManager::~CUsbCameraDeviceManager()
{
	// nothing
}

// List all USB devices on system
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::ListAllUsbDevices(std::vector<std::string> &deviceInstanceIDs, std::vector<std::string> &deviceDescriptions)
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	CHAR szDeviceInstanceID[MAX_DEVICE_ID_LEN];

	// List all connected USB devices
	//std::string enumerator = "USB";
	hDevInfo = SetupDiGetClassDevsA(NULL, "USB", NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	//hDevInfo = SetupDiGetClassDevs(NULL, pszEnumerator, NULL, pClassGuid != NULL ? DIGCF_PRESENT : DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return false;

	for (int i = 0;; i++)
	{
		DeviceInfoData.cbSize = sizeof(DeviceInfoData);
		if (!SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData))
			break;

		if (SetupDiGetDeviceInstanceIdA(hDevInfo, &DeviceInfoData, szDeviceInstanceID, MAX_DEVICE_ID_LEN, NULL))
		{
			char deviceDescription[1024] = { 0 };
			if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)deviceDescription, sizeof(deviceDescription), NULL))
			{
				std::string temp = szDeviceInstanceID;
				deviceInstanceIDs.push_back(temp);

				std::string temp2 = deviceDescription;
				deviceDescriptions.push_back(temp2);
			}
		}
	}
	return true;
}

// Search the USB devices for the first device which has a keyword in it's description
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::FindDeviceInstanceID(std::string &keyWordInDescription, std::string &foundDeviceInstanceID, std::string &foundDeviceDescription)
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	CHAR szDeviceInstanceID[MAX_DEVICE_ID_LEN];

	// List all connected USB devices
	hDevInfo = SetupDiGetClassDevsA(NULL, "USB", NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	//hDevInfo = SetupDiGetClassDevs(NULL, pszEnumerator, NULL, pClassGuid != NULL ? DIGCF_PRESENT : DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return false;

	for (int i = 0;; i++)
	{
		DeviceInfoData.cbSize = sizeof(DeviceInfoData);
		if (!SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData))
			break;

		if (SetupDiGetDeviceInstanceIdA(hDevInfo, &DeviceInfoData, szDeviceInstanceID, MAX_DEVICE_ID_LEN, NULL))
		{
			char deviceDescription[1024] = { 0 };
			if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)deviceDescription, sizeof(deviceDescription), NULL))
			{
				std::string strDeviceDescription = deviceDescription;
				if (strDeviceDescription.find(keyWordInDescription) != std::string::npos)
				{
					std::string strDeviceInstanceID = szDeviceInstanceID;
					foundDeviceInstanceID = strDeviceInstanceID;
					foundDeviceDescription = strDeviceDescription;
					return true;
				}
			}
		}
	}

	return true;
}
		
// Get the device's description from it's instance ID.
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::GetDeviceDescription(std::string &deviceInstanceID, std::string &deviceDescription)
{
	char szDeviceDescription[1024] = { 0 };
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	hDevInfo = SetupDiGetClassDevsA(NULL, deviceInstanceID.c_str(), NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return false;

	DeviceInfoData.cbSize = sizeof(DeviceInfoData);
	if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &DeviceInfoData))
		return false;

	if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)szDeviceDescription, sizeof(szDeviceDescription), NULL))
	{
		std::string temp = szDeviceDescription;
		deviceDescription = temp;
		return true;
	}
	else
		return false;
}

// Get the device's Parent device instance ID from the device's instance id
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::GetParentDeviceInstanceID(std::string &deviceInstanceID, std::string &parentDeviceInstanceID)
{
	CONFIGRET status;
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DEVINST devInstParent;
	CHAR szParentDeviceInstanceID[MAX_DEVICE_ID_LEN];

	hDevInfo = SetupDiGetClassDevsA(NULL, deviceInstanceID.c_str(), NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return false;

	DeviceInfoData.cbSize = sizeof(DeviceInfoData);
	if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &DeviceInfoData))
		return false;

	status = CM_Get_Parent(&devInstParent, DeviceInfoData.DevInst, 0);
	if (status == CR_SUCCESS)
	{
		status = CM_Get_Device_IDA(devInstParent, szParentDeviceInstanceID, MAX_DEVICE_ID_LEN, 0);
		if (status == CR_SUCCESS)
		{
			std::string temp = szParentDeviceInstanceID;
			parentDeviceInstanceID = temp;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

// Reads the camera's Full Name and constructs the needed ID tags for finding it in the system.
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::InitializeFromCamera(std::string serialNumber)
{
	try
	{
		if (serialNumber == "")
		{
			m_errorMessage = "Serial Number Required for Initialization";
			return false;
		}

		m_serialNumber = serialNumber;

		// the device instance ID of the camera device can be found by the camera device's fullname
		std::string fullName;
		std::string vendorID;
		std::string productID; // 0xba02 is ace, 0xba03 is dart

		// we can only run this on usb cameras.
		Pylon::CDeviceInfo filter;
		filter.SetDeviceClass(Pylon::BaslerUsbDeviceClass);
		filter.SetSerialNumber(m_serialNumber.c_str());

		// create a pylon device from the given serial number.
		Pylon::DeviceInfoList_t devices;
		Pylon::DeviceInfoList_t filters;
		filters.push_back(filter);
		Pylon::CTlFactory::GetInstance().EnumerateDevices(devices, filters);

		if (devices.size() == 0)
		{
			//std::cerr << "No matching camera devices found." << std::endl;
			m_errorMessage = "Error: Initialize(): No matching camera devices found.";
			return false;
		}

		Pylon::StringList_t propertyNames;
		devices[0].GetPropertyNames(propertyNames);

		// read through the device's information, and capture the full name, vendor and product Id's to make the device instance id.
		for (size_t i = 0; i < propertyNames.size(); i++)
		{
			Pylon::String_t propertyValue;
			devices[0].GetPropertyValue(propertyNames[i], propertyValue);
			if (propertyNames[i] == "FullName")
				fullName = propertyValue;
			if (propertyNames[i] == "VendorId")
				vendorID = propertyValue;
			if (propertyNames[i] == "ProductId")
				productID = propertyValue;
		}

		m_productID = productID;

		// create the device instance Id from the camera full name
		std::size_t loc1 = fullName.find("usb");
		std::size_t loc2 = fullName.find("{");
		std::string deviceID = fullName.substr(loc1, (loc2 - loc1) - 1);
		std::size_t startLoc = 0;
		while ((startLoc = deviceID.find("#", startLoc)) != std::string::npos)
		{
			deviceID.replace(startLoc, 1, "\\");
			startLoc += 1;
		}

		m_deviceInstance = deviceID;

		// create the device instance Id for the parent usb composite device.
		std::string compositeDeviceID = "USB\\VID_";
		std::string vendorIDsub = vendorID.substr(2, 4);
		std::string productIDsub = productID.substr(2, 4);
		compositeDeviceID.append(vendorIDsub);
		compositeDeviceID.append("&");
		compositeDeviceID.append("PID_");
		compositeDeviceID.append(productIDsub);
		compositeDeviceID.append("\\");
		compositeDeviceID.append(m_serialNumber);

		m_compositeDeviceInstance = compositeDeviceID;

		return true;
	}
	catch (const GenICam::GenericException &e)
	{
		// Error handling.
		m_errorMessage = "Error: Initialize(): GenICam exception occurred. ";
		m_errorMessage.append(e.GetDescription());
		return false;
	}
	catch (std::exception &e)
	{
		// Error handling.
		m_errorMessage = "Error: Initialize(): std exception occurred. ";
		m_errorMessage.append(e.what());
		return false;
	}
	catch (...)
	{
		// Error handling.
		m_errorMessage = "Error: Initialize(): unknown exception occured.";
		return false;
	}
}

// Enables the device like in Windows Device Manager
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::EnableDevice(std::string deviceInstanceID, std::string &errorMessage)
{

	try
	{
		//std::cout << "Enabling USB Camera Device..." << std::endl;

		if (deviceInstanceID == "")
			return false;

		HDEVINFO hDevInfo;
		SP_DEVINFO_DATA spDevInfoData;
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		SP_PROPCHANGE_PARAMS spPropChangeParams;

		// create an HDEVINFO for the specific device
		hDevInfo = SetupDiGetClassDevsA(NULL, deviceInstanceID.c_str(), NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);

		if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &spDevInfoData))
		{
			//std::cerr << "Error in SetupDiEnumDeviceInfo: " << GetLastError() << std::endl;
			errorMessage = "Error: EnableDevice(): SetupDiEnumDeviceInfo(): ";
			errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		// set the disable flag and disable the device
		spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		spPropChangeParams.Scope = DICS_FLAG_GLOBAL;

		// reset the flag to enable and repeat.
		spPropChangeParams.StateChange = DICS_ENABLE;

		if (!SetupDiSetClassInstallParams(hDevInfo, &spDevInfoData, (SP_CLASSINSTALL_HEADER*)&spPropChangeParams, sizeof(spPropChangeParams)))
		{
			errorMessage = "Error: EnableDevice(): SetupDiSetClassInstallParams(): ";
			errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &spDevInfoData))
		{
			errorMessage = "Error: EnableDevice(): SetupDiCallClassInstaller(): ";
			errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		return true;

	}
	catch (const GenICam::GenericException &e)
	{
		// Error handling.
		errorMessage = "Error: EnableDevice(): GenICam exception occurred: ";
		errorMessage.append(e.GetDescription());
		return false;
	}
	catch (std::exception &e)
	{
		// Error handling.
		errorMessage = "Error: EnableDevice(): std exception occurred:";
		errorMessage.append(e.what());
		return false;
	}
	catch (...)
	{
		// Error handling.
		errorMessage = "Error: EnableDevice(): unknown exception occured.";
		return false;
	}
}

// Enables the camera like in Windows Device Manager
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::EnableCamera()
{
	try
	{
		std::string deviceInstanceID = m_deviceInstance;
	
		HDEVINFO hDevInfo;
		SP_DEVINFO_DATA spDevInfoData;
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		SP_PROPCHANGE_PARAMS spPropChangeParams;

		// create an HDEVINFO for the specific device
		hDevInfo = SetupDiGetClassDevsA(NULL, deviceInstanceID.c_str(), NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);

		if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &spDevInfoData))
		{
			//std::cerr << "Error in SetupDiEnumDeviceInfo: " << GetLastError() << std::endl;
			m_errorMessage = "Error: EnableDevice(): SetupDiEnumDeviceInfo(): ";
			m_errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		// set the disable flag and disable the device
		spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		spPropChangeParams.Scope = DICS_FLAG_GLOBAL;

		// reset the flag to enable and repeat.
		spPropChangeParams.StateChange = DICS_ENABLE;

		if (!SetupDiSetClassInstallParams(hDevInfo, &spDevInfoData, (SP_CLASSINSTALL_HEADER*)&spPropChangeParams, sizeof(spPropChangeParams)))
		{
			m_errorMessage = "Error: EnableDevice(): SetupDiSetClassInstallParams(): ";
			m_errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &spDevInfoData))
		{
			m_errorMessage = "Error: EnableDevice(): SetupDiCallClassInstaller(): ";
			m_errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		if (deviceInstanceID == m_deviceInstance)
		{
			// check that the camera is really enabled
			Pylon::CDeviceInfo filter;
			filter.SetDeviceClass(Pylon::BaslerUsbDeviceClass);
			filter.SetSerialNumber(m_serialNumber.c_str());

			// create a pylon device from the given serial number.
			Pylon::DeviceInfoList_t devices;
			Pylon::DeviceInfoList_t filters;
			filters.push_back(filter);

			Pylon::CTlFactory::GetInstance().EnumerateDevices(devices, filters);
			if (devices.size() > 0)
			{
				//std::cout << "USB Camera Device Enabled." << std::endl;
				return true;
			}
			else
			{
				int attempts = 0;
				while (attempts < 10)
				{
					Pylon::CTlFactory::GetInstance().EnumerateDevices(devices, filters);
					if (devices.size() > 0)
					{
						//std::cout << "USB Camera Device Enabled." << std::endl;
						return true;
					}

					attempts++;
					Sleep(1000);
				}

				m_errorMessage = "Error: EnableDevice(): No matching camera devices found.";
				return false;
			}
		}
		else
			return true;
	}
	catch (const GenICam::GenericException &e)
	{
		// Error handling.
		m_errorMessage = "Error: EnableDevice(): GenICam exception occurred: ";
		m_errorMessage.append(e.GetDescription());
		return false;
	}
	catch (std::exception &e)
	{
		// Error handling.
		m_errorMessage = "Error: EnableDevice(): std exception occurred:";
		m_errorMessage.append(e.what());
		return false;
	}
	catch (...)
	{
		// Error handling.
		m_errorMessage = "Error: EnableDevice(): unknown exception occured.";
		return false;
	}
}

// Disables the device like in Windows Device Manager
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::DisableDevice(std::string deviceInstanceID, std::string &errorMessage)
{
	try
	{
		//std::cout << "Disabling USB Camera Device..." << std::endl;
		if (deviceInstanceID == "")
			return false;

		HDEVINFO hDevInfo;
		SP_DEVINFO_DATA spDevInfoData;
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		SP_PROPCHANGE_PARAMS spPropChangeParams;

		// create an HDEVINFO for the specific device
		hDevInfo = SetupDiGetClassDevsA(NULL, deviceInstanceID.c_str(), NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT | DIGCF_ALLCLASSES);

		if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &spDevInfoData))
		{
			errorMessage = "Error: DisableDevice(): SetupDiEnumDeviceInfo(): ";
			errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		// set the disable flag and disable the device
		spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		spPropChangeParams.Scope = DICS_FLAG_GLOBAL;

		// reset the flag to enable and repeat.
		spPropChangeParams.StateChange = DICS_DISABLE;

		if (!SetupDiSetClassInstallParams(hDevInfo, &spDevInfoData, (SP_CLASSINSTALL_HEADER*)&spPropChangeParams, sizeof(spPropChangeParams)))
		{
			errorMessage = "Error: DisableDevice(): SetupDiSetClassInstallParams(): ";
			errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &spDevInfoData))
		{
			errorMessage = "Error: DisableDevice(): SetupDiCallClassInstaller(): ";
			errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		//std::cout << "USB Camera Device Disabled." << std::endl;

		return true;
	}
	catch (const GenICam::GenericException &e)
	{
		// Error handling.
		errorMessage = "Error: DisableDevice(): GenICam exception occurred. ";
		errorMessage.append(e.GetDescription());
		return false;
	}
	catch (std::exception &e)
	{
		// Error handling.
		errorMessage = "Error: DisableDevice(): std exception occurred. ";
		errorMessage.append(e.what());
		return false;
	}
	catch (...)
	{
		// Error handling.
		errorMessage = "Error: DisableDevice(): unknown exception occured.";
		return false;
	}
}

// Disables the camera like in Windows Device Manager
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::DisableCamera()
{
	try
	{
		std::string deviceInstanceID = m_deviceInstance;

		HDEVINFO hDevInfo;
		SP_DEVINFO_DATA spDevInfoData;
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		SP_PROPCHANGE_PARAMS spPropChangeParams;

		// create an HDEVINFO for the specific device
		hDevInfo = SetupDiGetClassDevsA(NULL, deviceInstanceID.c_str(), NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT | DIGCF_ALLCLASSES);

		if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &spDevInfoData))
		{
			m_errorMessage = "Error: DisableDevice(): SetupDiEnumDeviceInfo(): ";
			m_errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		// set the disable flag and disable the device
		spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		spPropChangeParams.Scope = DICS_FLAG_GLOBAL;

		// reset the flag to enable and repeat.
		spPropChangeParams.StateChange = DICS_DISABLE;

		if (!SetupDiSetClassInstallParams(hDevInfo, &spDevInfoData, (SP_CLASSINSTALL_HEADER*)&spPropChangeParams, sizeof(spPropChangeParams)))
		{
			m_errorMessage = "Error: DisableDevice(): SetupDiSetClassInstallParams(): ";
			m_errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &spDevInfoData))
		{
			m_errorMessage = "Error: DisableDevice(): SetupDiCallClassInstaller(): ";
			m_errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		//std::cout << "USB Camera Device Disabled." << std::endl;

		return true;
	}
	catch (const GenICam::GenericException &e)
	{
		// Error handling.
		m_errorMessage = "Error: DisableDevice(): GenICam exception occurred. ";
		m_errorMessage.append(e.GetDescription());
		return false;
	}
	catch (std::exception &e)
	{
		// Error handling.
		m_errorMessage = "Error: DisableDevice(): std exception occurred. ";
		m_errorMessage.append(e.what());
		return false;
	}
	catch (...)
	{
		// Error handling.
		m_errorMessage = "Error: DisableDevice(): unknown exception occured.";
		return false;
	}
}

// Enables the camera device's parent USB Composite Device like in Windows Device Manager
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::EnableCameraCompositeDevice()
{
	try
	{
		if (m_compositeDeviceInstance == "")
		{
			m_errorMessage = "Error: EnableCompositeDevice(): Composite Device Instance Invalid.";
			return false;
		}

		HDEVINFO hDevInfo;
		SP_DEVINFO_DATA spDevInfoData;
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		SP_PROPCHANGE_PARAMS spPropChangeParams;

		// create an HDEVINFO for the specific composite device
		hDevInfo = SetupDiGetClassDevsA(NULL, m_compositeDeviceInstance.c_str(), NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);

		if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &spDevInfoData))
		{
			m_errorMessage = "Error: EnableCompositeDevice(): SetupDiEnumDeviceInfo(): ";
			m_errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		// set the disable flag and disable the device
		spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		spPropChangeParams.Scope = DICS_FLAG_GLOBAL;
		spPropChangeParams.StateChange = DICS_ENABLE;

		if (!SetupDiSetClassInstallParams(hDevInfo, &spDevInfoData, (SP_CLASSINSTALL_HEADER*)&spPropChangeParams, sizeof(spPropChangeParams)))
		{
			m_errorMessage = "Error: EnableCompositeDevice(): SetupDiSetClassInstallParams(): ";
			m_errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &spDevInfoData))
		{
			m_errorMessage = "Error: EnableCompositeDevice(): SetupDiCallClassInstaller(): ";
			DWORD lastError = GetLastError();
			if (lastError == 3758096907)
				m_errorMessage.append("Windows Error code 3758096907. Device may have been removed.");
			else
				m_errorMessage.append(std::to_string(lastError));
			return false;
		}

		//std::cout << "USB Composite Device Enabled." << std::endl;
		return true;
	}
	catch (const GenICam::GenericException &e)
	{
		// Error handling.
		m_errorMessage = "Error: EnableCompositeDevice(): GenICam exception occurred. ";
		m_errorMessage.append(e.GetDescription());
		return false;
	}
	catch (std::exception &e)
	{
		// Error handling.
		m_errorMessage = "Error: EnableCompositeDevice(): std exception occurred. ";
		m_errorMessage.append(e.what());
		return false;
	}
	catch (...)
	{
		// Error handling.
		m_errorMessage = "Error: EnableCompositeDevice(): unknown exception occured.";
		return false;
	}
}

// Disables the camera device's parent USB Composite Device like in Windows Device Manager
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::DisableCameraCompositeDevice()
{
	try
	{
		//std::cout << "Disabling USB Composite Device..." << std::endl;

		if (m_compositeDeviceInstance == "")
		{
			m_errorMessage = "Error: DisableCompositeDevice(): Composite Device Instance Invalid.";
			return false;
		}

		HDEVINFO hDevInfo;
		SP_DEVINFO_DATA spDevInfoData;
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		SP_PROPCHANGE_PARAMS spPropChangeParams;

		// create an HDEVINFO for the specific composite device
		hDevInfo = SetupDiGetClassDevsA(NULL, m_compositeDeviceInstance.c_str(), NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT | DIGCF_ALLCLASSES);

		if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &spDevInfoData))
		{
			m_errorMessage = "Error: DisableCompositeDevice(): SetupDiEnumDeviceInfo(): ";
			DWORD lastError = GetLastError();
			if (lastError == 259)
				m_errorMessage.append("Windows Error code 259. Device may have been removed.");
			else
				m_errorMessage.append(std::to_string(lastError));
			return false;
		}

		// set the disable flag and disable the device
		spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		spPropChangeParams.Scope = DICS_FLAG_GLOBAL;
		spPropChangeParams.StateChange = DICS_DISABLE;

		if (!SetupDiSetClassInstallParams(hDevInfo, &spDevInfoData, (SP_CLASSINSTALL_HEADER*)&spPropChangeParams, sizeof(spPropChangeParams)))
		{
			m_errorMessage = "Error: DisableCompositeDevice(): SetupDiSetClassInstallParams(): ";
			m_errorMessage.append(std::to_string(GetLastError()));
			return false;
		}

		if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &spDevInfoData))
		{
			m_errorMessage = "Error: DisableCompositeDevice(): SetupDiCallClassInstaller(): ";
			DWORD lastError = GetLastError();
			if (lastError == 5)
				m_errorMessage.append("Windows Error code 5. Administrator rights required.");
			else
				m_errorMessage.append(std::to_string(lastError));
			return false;
		}

		//std::cout << "USB Composite Device Disabled." << std::endl;
		return true;
	}
	catch (const GenICam::GenericException &e)
	{
		// Error handling.
		m_errorMessage = "Error: DisableCompositeDevice(): GenICam exception occurred. ";
		m_errorMessage.append(e.GetDescription());
		return false;
	}
	catch (std::exception &e)
	{
		// Error handling.
		m_errorMessage = "Error: DisableCompositeDevice(): std exception occurred. ";
		m_errorMessage.append(e.what());
		return false;
	}
	catch (...)
	{
		// Error handling.
		m_errorMessage = "Error: DisableCompositeDevice(): unknown exception occured.";
		return false;
	}
}

// This will check the status of the "allow computer to turn off this device..." field in device manager
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::ReadDeviceTreePowerStates()
{
#define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof(arr[0]))

	DEVINST devInstParent;
	CONFIGRET status;
	CHAR szDeviceInstanceID[255];

	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA spDevInfoData;
	spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	// create an HDEVINFO for the specific composite device
	hDevInfo = SetupDiGetClassDevsA(NULL, m_deviceInstance.c_str(), NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);

	if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &spDevInfoData))
	{
		m_errorMessage = "Error: EnableCompositeDevice(): SetupDiEnumDeviceInfo(): ";
		m_errorMessage.append(std::to_string(GetLastError()));
		return false;
	}
	
	for (int i = 0; i < INT16_MAX; i++)
	{
		if (i == 0)
		{
			devInstParent = spDevInfoData.DevInst;
			status = CR_SUCCESS;
		}
		else
			status = CM_Get_Parent(&devInstParent, devInstParent, 0);
		if (status == CR_SUCCESS)
		{
			status = CM_Get_Device_IDA(devInstParent, szDeviceInstanceID, ARRAY_SIZE(szDeviceInstanceID), 0);
			if (status == CR_SUCCESS)
			{
				LSTATUS returnCode;
				HKEY hkey;
				std::string key = "SYSTEM\\CurrentControlSet\\Enum\\";
				key.append(szDeviceInstanceID);

				returnCode = RegOpenKeyExA(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &hkey);
				if (returnCode == ERROR_SUCCESS)
				{
					CHAR szBuffer[512];
					DWORD dwBufferSize = sizeof(szBuffer);
					returnCode = RegQueryValueExA(hkey, "DeviceDesc", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
					if (returnCode == ERROR_SUCCESS)
					{
						std::string deviceDesc = szBuffer;
						std::size_t nameStart = deviceDesc.find(';') + 1;
						std::string name = deviceDesc.substr(nameStart, 64);
						key.append("\\Device Parameters\\WDF");
						// Open the registry keys where information about network adapters is stored.Su
						HKEY hSubkey;
						returnCode = RegOpenKeyExA(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &hSubkey);
						if (returnCode == ERROR_SUCCESS)
						{
							DWORD nValue = MAXDWORD;
							DWORD dwBufferSize(sizeof(DWORD));
							DWORD nResult(0);
							returnCode = RegQueryValueExA(hSubkey, "IdleInWorkingState", 0, NULL, reinterpret_cast<LPBYTE>(&nResult), &dwBufferSize);
							if (returnCode == ERROR_SUCCESS)
							{
								nValue = nResult;
								m_deviceNames.push_back(name);
								m_devicePowerStates.push_back(std::to_string(nValue));
							}
							else
							{
								m_deviceNames.push_back(name);
								m_devicePowerStates.push_back("N/A");
							}

							RegCloseKey(hSubkey);
						}
						else
						{
							m_deviceNames.push_back(name);
							m_devicePowerStates.push_back("N/A");
						}
					}
					else
						break;

					RegCloseKey(hkey);
				}
			}
		}
	}

	return true;
}

// TODO
// split and make static.
bool UsbCameraDeviceManager::CUsbCameraDeviceManager::ReadPowerSchemeSettings()
{
	/*
	output of c:\powercfg /q

	Subgroup GUID: 2a737441-1930-4402-8d77-b2bebba308a3  (USB settings)
	Power Setting GUID: 48e6b7a6-50f5-4782-a5d4-53bb8f07e226  (USB selective suspend setting)
	Possible Setting Index: 000
	Possible Setting Friendly Name: Disabled
	Possible Setting Index: 001
	Possible Setting Friendly Name: Enabled
	Current AC Power Setting Index: 0x00000001
	Current DC Power Setting Index: 0x00000001

	Power Setting GUID: d4e98f31-5ffe-4ce1-be31-1b38b384c009  (USB 3 Link Power Mangement)
	Possible Setting Index: 000
	Possible Setting Friendly Name: Off
	Possible Setting Index: 001
	Possible Setting Friendly Name: Minimum power savings
	Possible Setting Index: 002
	Possible Setting Friendly Name: Moderate power savings
	Possible Setting Index: 003
	Possible Setting Friendly Name: Maximum power savings
	Current AC Power Setting Index: 0x00000003
	Current DC Power Setting Index: 0x00000003
	*/
	try
	{
		// check if windows fast boot is enabled (aka hiberboot)
		{
			LSTATUS returnCode;
			HKEY hkey;
			std::string key = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Power";
			// Open the registry keys where information about network adapters is stored.
			returnCode = RegOpenKeyExA(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &hkey);
			if (returnCode == ERROR_SUCCESS)
			{
				DWORD dwResult(0);
				DWORD dwBufferSize(sizeof(DWORD));
				returnCode = RegQueryValueExA(hkey, "HiberbootEnabled", 0, NULL, reinterpret_cast<LPBYTE>(&dwResult), &dwBufferSize);
				if (returnCode == ERROR_SUCCESS)
				{
					if (dwResult == 1)
						m_hiberBootEnabled = 1;
					else if (dwResult == 0)
						m_hiberBootEnabled = 0;
				}
				RegCloseKey(hkey);
			}
		}


		GUID* pGuidActivePwrSchm = NULL;
		DWORD dwR = PowerGetActiveScheme(NULL, &pGuidActivePwrSchm);
		int result = 0;
		result = PowerGetActiveScheme(NULL, &pGuidActivePwrSchm);
		if (result == ERROR_SUCCESS)
		{
			UCHAR   nameBuffer[2048];
			DWORD   nameBufferSize;
			ZeroMemory(&nameBuffer, sizeof(nameBuffer));
			nameBufferSize = sizeof(nameBuffer);
			PowerReadFriendlyName(NULL, pGuidActivePwrSchm, &NO_SUBGROUP_GUID, NULL, nameBuffer, &nameBufferSize);
			char * destString = new char[nameBufferSize];
			WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)nameBuffer, -1, destString, 32, 0, 0);
			if (destString != NULL)
				m_activePowerSchemeName = destString;
		
			DWORD val;
			result = PowerReadACValueIndex(NULL, pGuidActivePwrSchm, &GUID_USB_SUBGROUP, &GUID_USB_SELECTIVE_SUSPEND, &val);
			if (result == ERROR_SUCCESS)
			{
				switch (val)
				{
					case 0:
						m_UsbSelectiveSuspendIsEnabledAC = 0;
						break;
					case 1:
						m_UsbSelectiveSuspendIsEnabledAC = 1;
						break;
				}
			}
			else
			{
				m_errorMessage.append("PowerReadACValueIndex() error: ");
				m_errorMessage.append(std::to_string(result));
				return false;
			}

			result = PowerReadDCValueIndex(NULL, pGuidActivePwrSchm, &GUID_USB_SUBGROUP, &GUID_USB_SELECTIVE_SUSPEND, &val);
			if (result == ERROR_SUCCESS)
			{
				switch (val)
				{
					case 0:
						m_UsbSelectiveSuspendIsEnabledDC = 0;
						break;
					case 1:
						m_UsbSelectiveSuspendIsEnabledDC = 1;
						break;
				}
			}
			else
			{
				m_errorMessage.append("PowerReadDCValueIndex() error: ");
				m_errorMessage.append(std::to_string(result));
				return false;
			}

			result = PowerReadACValueIndex(NULL, pGuidActivePwrSchm, &GUID_USB_SUBGROUP, &GUID_USB3_LINK_POWER_MANAGEMENT, &val);
			if (result == ERROR_SUCCESS)
			{
				switch (val)
				{
					case 0:
						m_Usb3LinkPowerManagmentIsEnabledAC = 0;
						break;
					case 1:
						m_Usb3LinkPowerManagmentIsEnabledAC = 1;
						break;
					case 2:
						m_Usb3LinkPowerManagmentIsEnabledAC = 1;
						break;
					case 3:
						m_Usb3LinkPowerManagmentIsEnabledAC = 1;
						break;
				}
			}
			else
			{
				m_errorMessage.append("Unable to read LPM setting. Probably just not available on this PC. Code: ");
				m_errorMessage.append(std::to_string(result));
				return false;
			}



			result = PowerReadDCValueIndex(NULL, pGuidActivePwrSchm, &GUID_USB_SUBGROUP, &GUID_USB3_LINK_POWER_MANAGEMENT, &val);
			if (result == ERROR_SUCCESS)
			{
				switch (val)
				{
					case 0:
						m_Usb3LinkPowerManagmentIsEnabledDC = 0;
						break;
					case 1:
						m_Usb3LinkPowerManagmentIsEnabledDC = 1;
						break;
					case 2:
						m_Usb3LinkPowerManagmentIsEnabledDC = 1;
						break;
					case 3:
						m_Usb3LinkPowerManagmentIsEnabledDC = 1;
						break;
				}
			}
			else
			{
				m_errorMessage.append("Unable to read LPM setting. Probably just not available on this PC. Code: ");
				m_errorMessage.append(std::to_string(result));
				return false;
			}

			if (pGuidActivePwrSchm)
			{
				LocalFree(pGuidActivePwrSchm);
				pGuidActivePwrSchm = NULL;
			}

			return true;
		}
		else
		{
			m_errorMessage.append("PowerReadDCValueIndex() error: ");
			m_errorMessage.append(std::to_string(result));
			return false;
		}
	}
	catch (const GenICam::GenericException &e)
	{
		// Error handling.
		m_errorMessage = "Error: GetUsbPowerSettings(): GenICam exception occurred. ";
		m_errorMessage.append(e.GetDescription());
		return false;
	}
	catch (std::exception &e)
	{
		// Error handling.
		m_errorMessage = "Error: GetUsbPowerSettings(): std exception occurred. ";
		m_errorMessage.append(e.what());
		return false;
	}
	catch (...)
	{
		// Error handling.
		m_errorMessage = "Error: GetUsbPowerSettings(): unknown exception occured.";
		return false;
	}
}

// TODO
// split and make static
int UsbCameraDeviceManager::CUsbCameraDeviceManager::IsEnabledFastBoot()
{
	return m_hiberBootEnabled;
}

int UsbCameraDeviceManager::CUsbCameraDeviceManager::IsSelectiveSuspendEnabledOnBattery()
{
	return m_UsbSelectiveSuspendIsEnabledDC;
}

int UsbCameraDeviceManager::CUsbCameraDeviceManager::IsSelectiveSuspendEnabledWhenPluggedIn()
{
	return m_UsbSelectiveSuspendIsEnabledAC;
}

int UsbCameraDeviceManager::CUsbCameraDeviceManager::IsUsb3LinkPowerManagementEnabledOnBattery()
{
	return m_Usb3LinkPowerManagmentIsEnabledDC;
}

int UsbCameraDeviceManager::CUsbCameraDeviceManager::IsUsb3LinkPowerManagementEnabledWhenPluggedIn()
{
	return m_Usb3LinkPowerManagmentIsEnabledAC;
}

// get the device names in the camera's tree
std::vector<std::string> UsbCameraDeviceManager::CUsbCameraDeviceManager::GetCameraTreeDeviceNames()
{
	return m_deviceNames;
}

// get the power states of the device names in the tree
std::vector<std::string> UsbCameraDeviceManager::CUsbCameraDeviceManager::GetCameraTreeDevicePowerStates()
{
	return m_devicePowerStates;
}

// get the name of the active power scheme
std::string UsbCameraDeviceManager::CUsbCameraDeviceManager::GetActivePowerSchemeName()
{
	return m_activePowerSchemeName;
}

// For reference, the user can see if the camera is currently connected as USB2 or USB3.
std::string UsbCameraDeviceManager::CUsbCameraDeviceManager::GetUsbConnectionType()
{
	try
	{
		if (m_serialNumber == "")
		{
			m_errorMessage = "Error: GetUsbConnectionType(): Serial Number Invalid.";
			return "InvalidSn";
		}

		Pylon::CDeviceInfo filter;
		filter.SetDeviceClass(Pylon::BaslerUsbDeviceClass);
		filter.SetSerialNumber(m_serialNumber.c_str());

		// create a pylon device from the given serial number.
		Pylon::DeviceInfoList_t devices;
		Pylon::DeviceInfoList_t filters;
		filters.push_back(filter);
		Pylon::CTlFactory::GetInstance().EnumerateDevices(devices, filters);

		if (devices.size() == 0)
		{
			m_errorMessage = "Error: GetUsbConnectionType(): No matching camera devices found.";
			return "NoDeviceFound";
		}

		Pylon::String_t propertyValue;
		devices[0].GetPropertyValue("UsbPortVersionBcd", propertyValue);

		return propertyValue.c_str();
	}
	catch (const GenICam::GenericException &e)
	{
		// Error handling.
		m_errorMessage = "Error: GetUsbConnectionType(): GenICam exception occurred. ";
		m_errorMessage.append(e.GetDescription());
		return "error";
	}
	catch (std::exception &e)
	{
		// Error handling.
		m_errorMessage = "Error: GetUsbConnectionType(): std exception occurred. ";
		m_errorMessage.append(e.what());
		return "error";
	}
	catch (...)
	{
		// Error handling.
		m_errorMessage = "Error: GetUsbConnectionType(): unknown exception occured.";
		return "error";
	}
}

// For reference, the user can see the camera device instance string
std::string UsbCameraDeviceManager::CUsbCameraDeviceManager::GetDeviceInstanceID()
{
	return m_deviceInstance;
}

// For reference, the user can see the camera device's parent USB Composite Device instance string
std::string UsbCameraDeviceManager::CUsbCameraDeviceManager::GetCompositeDeviceInstanceID()
{
	return m_compositeDeviceInstance;
}

// For reference, the user can see the camera device's product ID tag.
std::string UsbCameraDeviceManager::CUsbCameraDeviceManager::GetProductID()
{
	return m_productID;
}

// For reference, the user can see the last error message.
std::string UsbCameraDeviceManager::CUsbCameraDeviceManager::GetLastErrorMessage()
{
	return m_errorMessage;
}

// *********************************************************************************************************

#endif
#endif