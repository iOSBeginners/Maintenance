#include "udpServer.h"
#include <iostream>
#include <Windows.h>
#include <cmath>
#include <sstream>
#include "dfulogger.h"
#include "vector"

using namespace std;		

#define INIT_PACKET                     0x01                                                            /**< Packet identifies for initialization packet. */
#define STOP_INIT_PACKET                0x02                                                            /**< Packet identifies for stop initialization packet. Used when complete init packet has been received so that the init packet can be used for pre validaiton. */
#define START_PACKET                    0x03                                                            /**< Packet identifies for the Data Start Packet. */
#define DATA_PACKET                     0x04                                                            /**< Packet identifies for a Data Packet. */
#define STOP_DATA_PACKET                0x05                                                            /**< Packet identifies for the Data Stop Packet. */
#define REQ_APP_RUNNING_PACKET			0x06
#define START_DFU_COMMAND				0x07
#define CONFIG_FIRMWARE					0x08
#define RESET_FIRMWARE					0x09
#define CHECK_IP						0x10
#define UPDATE_IP						0x11

const uint8_t FEEDBACK_VALID_START_PACKET[] = { START_PACKET, 0x01, 0x00, 0x00 };
const uint8_t FEEDBACK_INVALID_START_PACKET[] = { START_PACKET, 0x02, 0x00, 0x00 };
const uint8_t FEEDBACK_VALID_INIT_PACKET[] = { INIT_PACKET, 0x01, 0x00, 0x00 };
const uint8_t FEEDBACK_INVALID_INIT_PACKET[] = { INIT_PACKET, 0x02, 0x00, 0x00 };
const uint8_t FEEDBACK_DFU_COMPLETE[] = { STOP_DATA_PACKET, 0x01, 0x00, 0x00 };
const uint8_t FEEDBACK_DFU_FAILED[] = { STOP_DATA_PACKET, 0x02, 0x00, 0x00 };

const uint8_t REQ_APP_RUNNING[] = { REQ_APP_RUNNING_PACKET, 0x00, 0x00, 0x00 };
const uint8_t FEEDBACK_BOOTLOADER_IS_RUNNING[] = { REQ_APP_RUNNING_PACKET, 0x01, 0x00, 0x00 };

const uint8_t CHECK_IP_PACKET[] = { CHECK_IP, 0x00, 0x00, 0x00 };
const uint8_t FEEDBACK_CHECK_IP_PACKET[] = { CHECK_IP, 0x01, 0x00, 0x00 };

const uint8_t REQ_RESET_FIRMWARE[] = { RESET_FIRMWARE, 0x00, 0x00, 0x00 };

const uint8_t UPDATE_IP_START_PACKET[] = {UPDATE_IP, 0x00, 0x00, 0x00};
const uint8_t UPDATE_IP_FEEDBACK[] = { UPDATE_IP, 0x00, 0x00, 0x00 };



UdpServer::UdpServer(int port, const string& host_ip)
{
	m_port = port;
	m_host_ip = host_ip;
	m_dfuState = DFU_START;
}

bool UdpServer::initialize()
{

	WSADATA wsa;
	int slen = sizeof(si);
	char buffer[1024];
	//Initialise winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		ostringstream ss;
		ss << "Failed to initialize winsock. Error code: " << WSAGetLastError();
		Logger::log(ss.str());
		return false;
	}

	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		ostringstream ss;
		ss << "failed to create socket. Error code: " << WSAGetLastError();
		Logger::log(ss.str());
		return false;
	}

	//setup address structure
	memset((char *)&si, 0, sizeof(si));
	si.sin_family = AF_INET;
	si.sin_port = htons(m_port);
	si.sin_addr.S_un.S_addr = inet_addr(m_host_ip.c_str());
	return true;
}

void UdpServer::sendPacketAndWaitFeedback(const uint8_t* buffer, uint32_t bufferSize)
{
	m_rcvPacket.length = 0;

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(s, &readfds);
	int max_sd = s;

	if (sendto(s, (const char*)buffer, bufferSize, 0, (struct sockaddr *) &si, sizeof(si)) == SOCKET_ERROR)
	{
		ostringstream ss;
		ss << "Failed to send a packet : sendto() failed with error code : " << WSAGetLastError();
		Logger::log(ss.str());
		return;
	}

	// wait for the feedback
	timeval selectTimeout = { 2, 0 };
	int activity = select(max_sd + 1, &readfds, NULL, NULL, &selectTimeout);
	if ((activity < 0) && (errno != EINTR))
	{
		return;
	}
	if (FD_ISSET(s, &readfds))
	{
		int valread = recv(s, (char*)m_rcvPacket.data, MAX_LENGTH_PACKET, 0);
		if (valread>0)
			m_rcvPacket.length = valread;
	}
}

bool UdpServer::sendPacket(const uint8_t* buffer, uint32_t bufferSize)
{
	if (sendto(s, (const char*)buffer, bufferSize, 0, (struct sockaddr *) &si, sizeof(si)) == SOCKET_ERROR)
	{
		ostringstream ss;
		ss << "Failed to send a packet : sendto() failed with error code : " << WSAGetLastError();
		Logger::log(ss.str());
		return false;
	}
}

bool UdpServer::check_bootloader_ip()
{
	sendPacketAndWaitFeedback(CHECK_IP_PACKET, 4);
	if (m_rcvPacket.length > 0 && !memcmp(m_rcvPacket.data, FEEDBACK_CHECK_IP_PACKET, 4)){
		return true;
	}
	return false;

}
#if 0
void UdpServer::transferFirmware(string& sd_filename, string& bl_filename, SdkVersion sdkversion)
{
	bool ret;
	// read firmware binary file
	SdUpdatePacket dfuPacket(sd_filename, bl_filename, sdkversion);

	while (true)
	{
		switch (m_dfuState)
		{
		case DFU_START:
		{
						  Packet& packet = dfuPacket.buildStartPacket();
						  sendPacketAndWaitFeedback(packet.data, packet.length);
						  if (m_rcvPacket.length > 0 && !memcmp(m_rcvPacket.data, FEEDBACK_VALID_START_PACKET, 4)){
							  m_dfuState = DFU_INIT;
						  }
						  else{
							  Logger::console("fail to send the correct start packet ");
							  return;

						  }
						  break;
		}
		case DFU_INIT:
		{
						 Sleep(100);
						 Packet& packet = dfuPacket.buildInitPacket();
						 sendPacketAndWaitFeedback(packet.data, packet.length);
						 if (m_rcvPacket.length > 0 && !memcmp(m_rcvPacket.data, FEEDBACK_VALID_INIT_PACKET, 4)){
							 m_dfuState = DFU_PAYLOAD;
						 }
						 else{
							 Logger::console("fail to send the correct init packet ");
							 return;
						 }
						 break;
		}
		case DFU_PAYLOAD:
		{
							Sleep(50);
							Packet& packet = dfuPacket.buildPayloadPacket();
							if (dfuPacket.isAllPayloadSent())
								m_dfuState = DFU_STOP;
							else
								sendPacket(packet.data, packet.length);
							break;
		}
		case DFU_STOP:
		{
						 Sleep(100);

						 Packet& packet = dfuPacket.buildStopPacket();
						 sendPacketAndWaitFeedback(packet.data, packet.length);
						 if (m_rcvPacket.length > 0 && !memcmp(m_rcvPacket.data, FEEDBACK_DFU_COMPLETE, 4)){
							 m_dfuState = DFU_COMPLETE;
						 }
						 else{
							 return;
						 }
						 break;
		}
		case DFU_COMPLETE:
			return;
			break;
		}
	}

}

bool UdpServer::updateSoftDevice(string& sd_filename, string& bl_filename, SdkVersion sdkversion)
{
	int i = 0;
	while (i<2)
	{
		Sleep(1000);
		sendPacketAndWaitFeedback(REQ_APP_RUNNING, 4);
		if (m_rcvPacket.length > 0 && !memcmp(m_rcvPacket.data, FEEDBACK_BOOTLOADER_IS_RUNNING, 4)){
			Logger::console("bootloader mode is running");
			Sleep(100);
			transferFirmware(sd_filename, bl_filename, sdkversion);
			break;
		}
		i++;
	}

	if (m_dfuState == DFU_COMPLETE)
		return true;
	return false;
}
#endif
void UdpServer::closeSocket()
{
	closesocket(s);
	WSACleanup();
}

void UdpServer::requestFirmwareReset()
{
	sendPacket(REQ_RESET_FIRMWARE, sizeof(REQ_RESET_FIRMWARE));
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

bool UdpServer::updateIp(string const& ip)
{
	// convert the ip string to an array of 4 bytes
	vector<string> ipArrayStr = split(ip,'.');
	if (ipArrayStr.size() != 4)
		return false;
	vector<uint8_t> ipArray;
	for (int i = 0; i < ipArrayStr.size(); i++)
	{
		try
		{
			string str  = ipArrayStr[i] ;
			int val = stoi(str);
			ipArray.push_back((uint8_t)val);
		}
		catch (exception const& e)
		{
			return false;
		}
		
	}
	// send ip packet 
	
	uint8_t updateIpPacket[8];
	memcpy(updateIpPacket, UPDATE_IP_START_PACKET, 4);
	memcpy(updateIpPacket + 4, &ipArray[0], 4);
	sendPacketAndWaitFeedback(updateIpPacket, 8);
	for (int i = 0; i<8; i++)
		cout << (int)updateIpPacket[i] << endl;
	if (m_rcvPacket.length > 0 && !memcmp(m_rcvPacket.data, UPDATE_IP_FEEDBACK, 4)){
		Logger::console("the ip is updated");
		Sleep(100);
		return true;
	}
	return false;
	// send the reaquest to the device
}