/**

** @file : udpserver.h

** @brief : communicate with dfu target using udp protocol

*/

#ifndef __UDPSERVER__
#define __UDPSERVER__

#include <string>
#include <winsock2.h>
#include "SdUpdatePacket.h"



using namespace std;

enum DfuState
{
	DFU_START,
	DFU_INIT,
	DFU_PAYLOAD,
	DFU_STOP,
	DFU_COMPLETE
};


class UdpServer
{

public:
	UdpServer(int port, const std::string& host_ip);
	bool initialize();
	void sendPacketAndWaitFeedback(const uint8_t* buffer, uint32_t bufferSize);
	bool sendPacket(const uint8_t* buffer, uint32_t bufferSize);
	void transferFirmware(string& sd_filename,string& bl_filename, SdkVersion sdkversion);
	bool updateSoftDevice(string& sd_filename, string& bl_filename, SdkVersion sdkversion);
	bool check_bootloader_ip();
	void closeSocket();
	void requestFirmwareReset();
	bool updateIp(string const& ip);
private:
	int m_port;    // port used for the udp communication
	std::string m_host_ip; // ip address of the tdfu target
	struct sockaddr_in si;
	int s;
	DfuState m_dfuState;
	Packet m_rcvPacket;

};

#endif