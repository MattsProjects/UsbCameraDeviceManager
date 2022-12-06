// UsbCameraDeviceManagerLinux.h
// Various utilities for rebooting USB cameras, etc. in Linux
//
// Copyright (c) 2022 Matthew Breit - matt.breit@baslerweb.com or matt.breit@gmail.com
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
#ifndef USBCAMERADEVICEMANAGERLINUX_H
#define USBCAMERADEVICEMANAGERLINUX_H

#ifdef LINUX_BUILD
#include <pylon/PylonIncludes.h>
#include <iostream>


namespace UsbCameraDeviceManagerLinux
{
	class CUsbCameraDeviceManagerLinux
	{
	private:
		static int systemOutput(std::string& cmd, std::string& output);

	public:
		static bool UsbModeSwitchReset(Pylon::CDeviceInfo& cameraInfo, std::string& errorMessage);
	};
}


// *********************************************************************************************************
// DEFINITIONS

inline int UsbCameraDeviceManagerLinux::CUsbCameraDeviceManagerLinux::systemOutput(std::string& cmd, std::string& output)
{
	int status = 0;

	// ********************************************************P
	// Code block courtesy of "Yuval"
	// https://www.appsloveworld.com/cplus/100/126/how-to-get-linux-command-output-string-and-output-status-in-c

	// Save original stdout and stderr to enable restoring
	int org_stdout = dup(1);
	int org_stderr = dup(2);

	int pd[2];
	pipe(pd);

	// Make the read-end of the pipe non blocking, so if the command being
	// executed has no output the read() call won't get stuck
	//int flags = fcvt(pd[0], L_GET);
	//flags |= SOCK_NONBLOCK;

	//if (fcvt(pd[0], L_SET, flags) == -1) {
	//	throw string("fcntl() failed");
	//}

	// Redirect stdout and stderr to the write-end of the pipe
	dup2(pd[1], 1);
	dup2(pd[1], 2);

	status = system(cmd.c_str());

	int buf_size = 1000;
	char buf[buf_size];

	// Read from read-end of the pipe
	long num_bytes = read(pd[0], buf, buf_size);

	if (num_bytes > 0) {
		output.clear();
		output.append(buf, num_bytes);
	}

	// Restore stdout and stderr and release the org* descriptors
	dup2(org_stdout, 1);
	dup2(org_stderr, 2);
	close(org_stdout);
	close(org_stderr);
	// ***********************************************************************

	return status;
}

inline bool UsbCameraDeviceManagerLinux::CUsbCameraDeviceManagerLinux::UsbModeSwitchReset(Pylon::CDeviceInfo &cameraInfo, std::string &errorMessage)
{
	// this fix requires sudo/root priveledges
	if (CheckForAdmin::CheckForAdmin(errorMessage) == false)
	{
		errorMessage.append("must be run as sudo / root.");
		return false;
	}

	if (cameraInfo.GetDeviceClass() != BaslerUsbDeviceClass)
	{
		errorMessage.append("Only usb cameras support this.");
		return false;
	}

	std::string vendor = "2676";
	std::string product = "";

	// AceClassic/AceU/AceL have a different productID than Ace2
	if (cameraInfo.GetModelName().find("acA") != std::string::npos)
		product = "ba02";
	if (cameraInfo.GetModelName().find("a2A") != std::string::npos)
		product = "ba05";

	// First, find the bus and device id of the camera from the serial number.
	// eg: shell command to find the id line from lsusb output is:
	//     lsusb -v | grep -B 20 '24281256' | grep '2676:ba02'
	std::string strCommand = "";
	std::string strOutput = "";
	strCommand.append("lsusb -v | grep -B 20 '");
	strCommand.append(cameraInfo.GetSerialNumber());
	strCommand.append("' | grep ");
	strCommand.append("'");
	strCommand.append(vendor);
	strCommand.append(":");
	strCommand.append(product);
	strCommand.append("'");

	// run the command and capture the output
	systemOutput(strCommand, strOutput);

	// next, parse the output to find bus and device numbers
	std::size_t pos = strOutput.find("Bus ");
	std::string bus = strOutput.substr(pos + 4, 3);
	pos = strOutput.find("Device ");
	std::string device = strOutput.substr(pos + 7, 3);

	// Now we can reset the individual camera using usb_modeswitch
	// eg: sudo usb_modeswitch -v 0x2676 -p 0xba02 -b 003 -g 039 --reset-usb
	strCommand = "";
	strCommand.append("sudo usb_modeswitch");
	strCommand.append(" -v 0x");
	strCommand.append(vendor);
	strCommand.append(" -p 0x");
	strCommand.append(product);
	strCommand.append(" -b ");
	strCommand.append(bus);
	strCommand.append(" -g ");
	strCommand.append(device);
	strCommand.append(" -R");

	// issue the command to reset the usb port
	system(strCommand.c_str());

	return true;
}
// *********************************************************************************************************

#endif
#endif