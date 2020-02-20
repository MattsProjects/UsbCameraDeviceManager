/*
Note: Before getting started, Basler recommends reading the Programmer's Guide topic
in the pylon C++ API documentation that gets installed with pylon.
If you are upgrading to a higher major version of pylon, Basler also
strongly recommends reading the Migration topic in the pylon C++ API documentation.

This custom sample is not found in the standard Pylon SDK. It is released under the included Pylon License and is without warranty.

*/

// Include files to use the PYLON API
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// for using the UsbDeviceManager Class
#include "UsbCameraDeviceManager.h"

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using cout.
using namespace std;


int main(int argc, char* argv[])
{
	// The exit code of the sample application.
	int exitCode = 0;

	// Before using any pylon methods, the pylon runtime must be initialized. 
	PylonInitialize();

	try
	{
		// the serial number of the camera
		std::string serialNumber = "22663088";

		cout << "Device Serial Number         : " << serialNumber << endl;

		// create the UsbDeviceManager object (does not require camera to be connected)
		UsbCameraDeviceManager::CUsbCameraDeviceManager dm;

		// initial the device manager (pulls device instance ID, etc)
		dm.InitializeFromCamera(serialNumber);

		cout << "Device Instance ID           : " << dm.GetDeviceInstanceID() << endl;
		cout << "Composite Device Instance ID : " << dm.GetCompositeDeviceInstanceID() << endl;

		cout << "Powering down camera device..." << endl;
		dm.DisableCamera();

		cout << "Waiting 10 seconds..." << endl;
		Sleep(10000);

		cout << "Powering up camera device..." << endl;
		dm.EnableCamera();

		cout << "Waiting 10 seconds..." << endl;
		Sleep(10000);

		CDeviceInfo info;
		info.SetSerialNumber(serialNumber.c_str());
		CInstantCamera camera(CTlFactory::GetInstance().CreateDevice(info));
		camera.Open();
		cout << "Camera Connected :-)" << endl;

	}
	catch (const GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred:" << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	catch (std::exception &e)
	{
		// Error handling.
		cerr << "An exception occurred:" << endl
			<< e.what() << endl;
		exitCode = 1;
	}
	catch (...)
	{
		// Error handling.
		cerr << "An unknown exception occured:" << endl;
		exitCode = 1;
	}

	// Comment the following two lines to disable waiting on exit.
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	// Releases all pylon resources. 
	PylonTerminate();

	return exitCode;
}

