#pragma comment(lib,"ws2_32.lib") //Winsock Library

#include <string>
#include <vector>
#include "udpServer.h"
#include "dfulogger.h"
#include <iostream>
#include <Windows.h>
#if _MSC_VER < 1600 //MSVC version <8
	#include "nullptr_emulation.h"
#endif
using namespace std;

enum RetErrors
{
	ARGUMENT_ERROR = 1,
	DFU_ERROR,
	CONF_ERROR
};
//#define _LOCAL
#ifdef _LOCAL
const string NO_DEVICE_IP = "0.0.0.0";
const string BRIDGE_IP = "192.168.1.45";
const string ANTENNA_IP = "192.168.1.46";
const string BEACON_IP = "192.168.1.47";
#else
const string NO_DEVICE_IP = "0.0.0.0";
const string BRIDGE_IP = "192.168.94.2";
const string ANTENNA_IP = "192.168.94.3";
const string BEACON_IP = "192.168.94.4";
#endif
const int PORT = 1234;


struct Device
{
	Device(){ deviceType = NO_DEVICE; ip = NO_DEVICE_IP; }
	Device(DeviceType _deviceType)
	{ 
		deviceType = _deviceType; 
		switch (deviceType)
		{
			case BRIDGE:
				ip = BRIDGE_IP;
				break;
			case ANTENNA:
				ip = ANTENNA_IP;
				break;
			case BEACON:
				ip = BEACON_IP;
				break;
			default:
				ip = NO_DEVICE_IP;
				break;
		}
	}
	DeviceType deviceType;
	string ip;
};

DeviceType UpdatedDeviceType = BEACON;
#if 1
int main(int argc,char* argv[])
{
	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, SW_HIDE);
	// define the device instance with the ip we want to update
	Device updatedDevice(UpdatedDeviceType);
	Device currentDevice(NO_DEVICE);

	// define all devices
	vector<Device> devices;
	devices.push_back(Device(BRIDGE));
	devices.push_back(Device(ANTENNA));
	devices.push_back(Device(BEACON));
	
	// identify the ip address of the bootloader
	bool ret = false;
	for (int i = 0,j=0; i < devices.size(); j++)
	{
		
		Sleep(100);
 		Device device = devices[i];
		UdpServer udpCheckIp(PORT, device.ip);
		udpCheckIp.initialize();
		ret = udpCheckIp.check_bootloader_ip();
		udpCheckIp.closeSocket();
		if (ret){
			currentDevice = device;
			break;
		}
		if (j % 2)
			i++;
	}
	if (!ret)
	{
		Logger::log(" no device is detected through ethernet ");
		MessageBox(nullptr, TEXT("Echec de la mise à jour"), TEXT("Message"), MB_OK);
		return DFU_ERROR;
	}

	// change the ip address of the bootloader if the current device type of the current device is not the same as the device type of the device to update
	if (currentDevice.deviceType == updatedDevice.deviceType)
	{
		MessageBox(nullptr, TEXT("Aucune mise à jour n'est requise. Opération réussite"), TEXT("Message"), MB_OK);
		return 0;
	}
	
	UdpServer udpChangeIp(PORT, currentDevice.ip);
	ret = udpChangeIp.initialize();
	if (!ret)
	{
		Logger::log("failed to initialize udp server");
		MessageBox(nullptr, TEXT("Echec de la mise à jour"), TEXT("Message"), MB_OK);
		return RetErrors::DFU_ERROR;
	}
	ret  = udpChangeIp.updateIp(updatedDevice.ip);
	if (!ret)
	{
		Logger::log("failed to change the ip address ");
		MessageBox(nullptr, TEXT("Echec de la mise à jour"), TEXT("Message"), MB_OK);
		return RetErrors::DFU_ERROR;
	}
	Sleep(100);
	udpChangeIp.requestFirmwareReset();
	udpChangeIp.closeSocket();

	switch (updatedDevice.deviceType)
	{
		case BRIDGE:
			MessageBox(nullptr, TEXT("Mise à jour du bridge effectuée avec succès"), TEXT("Message"), MB_OK);
			break;
		case ANTENNA:
			MessageBox(nullptr, TEXT("Mise à jour de l'antenne effectuée avec succès"), TEXT("Message"), MB_OK);
			break;
		case BEACON:
			MessageBox(nullptr, TEXT("Mise à jour du beacon effectuée avec succès"), TEXT("Message"), MB_OK);
			break;
	}
	return 0;
}
#endif
#if 0
int main(int argc, char argv[])
{
	UdpServer udpChangeIp(PORT, BRIDGE_IP);
	bool ret = udpChangeIp.initialize();
	if (!ret)
	{
		Logger::log("failed to initialize udp server");
		MessageBox(nullptr, TEXT("Echec de la mise à jour"), TEXT("Message"), MB_OK);
		return RetErrors::DFU_ERROR;
	}
	udpChangeIp.UdpServer::updateSoftDevice(string("sdk12.bin"), string("bl"), SdkVersion::SDK11);
}
#endif