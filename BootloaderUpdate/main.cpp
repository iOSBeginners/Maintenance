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
const string BRIDGE_IP = "192.168.4.2";
const string ANTENNA_IP = "192.168.4.3";
const string BEACON_IP = "192.168.4.4";
#endif
const uint8_t BRIDGE_MAC[] = { 0x00, 0x08, 0xdc, 0x00, 0xab, 0xab };
const uint8_t ANTENNA_MAC[] = { 0x00, 0x08, 0xdc, 0x00, 0xab, 0xbc };
const uint8_t BEACON_MAC[] = { 0x00, 0x08, 0xdc, 0x00, 0xab, 0xcd };
const int PORT = 70;


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
				memcpy(mac, BRIDGE_MAC, 6);
				break;
			case ANTENNA:
				ip = ANTENNA_IP;
				memcpy(mac, ANTENNA_MAC, 6);
				break;
			case BEACON:
				ip = BEACON_IP;
				memcpy(mac, BEACON_MAC, 6);
				break;
			default:
				ip = NO_DEVICE_IP;
				break;
		}
	}
	DeviceType deviceType;
	string ip;
	uint8_t mac[6];
};

DeviceType UpdatedDeviceType = BEACON;

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
		MessageBox(nullptr, TEXT("Echec de la mise � jour"), TEXT("Message"), MB_OK);
		return DFU_ERROR;
	}

	// change the ip address of the bootloader if the current device type of the current device is not the same as the device type of the device to update
	if (currentDevice.deviceType == updatedDevice.deviceType)
	{
		MessageBox(nullptr, TEXT("Aucune mise � jour n'est requise. Op�ration r�ussite"), TEXT("Message"), MB_OK);
		return 0;
	}
	
	UdpServer udpChangeIp(PORT, currentDevice.ip);
	ret = udpChangeIp.initialize();
	if (!ret)
	{
		Logger::log("failed to initialize udp server");
		MessageBox(nullptr, TEXT("Echec de la mise � jour"), TEXT("Message"), MB_OK);
		return RetErrors::DFU_ERROR;
	}
	ret  = udpChangeIp.updateIp(updatedDevice.ip,updatedDevice.mac);
	if (!ret)
	{
		Logger::log("failed to change the ip address ");
		MessageBox(nullptr, TEXT("Echec de la mise � jour"), TEXT("Message"), MB_OK);
		return RetErrors::DFU_ERROR;
	}
	Sleep(100);
	udpChangeIp.requestFirmwareReset();
	udpChangeIp.closeSocket();

	switch (updatedDevice.deviceType)
	{
		case BRIDGE:
			MessageBox(nullptr, TEXT("Mise � jour du bridge effectu�e avec succ�s"), TEXT("Message"), MB_OK);
			break;
		case ANTENNA:
			MessageBox(nullptr, TEXT("Mise � jour de l'antenne effectu�e avec succ�s"), TEXT("Message"), MB_OK);
			break;
		case BEACON:
			MessageBox(nullptr, TEXT("Mise � jour du beacon effectu�e avec succ�s"), TEXT("Message"), MB_OK);
			break;
	}
	return 0;
}
