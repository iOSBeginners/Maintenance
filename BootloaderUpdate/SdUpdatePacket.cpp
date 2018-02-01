#include "SdUpdatePacket.h"
#include "dfulogger.h"

#include <fstream>
#include <algorithm>
#include <iostream>

#define MAX_BOOTLOADER_SIZE 0x4c00 

using namespace std;

#if 1
uint16_t crc16_compute(uint8_t const * p_data, uint32_t size, uint16_t const * p_crc)
{
	uint16_t crc = (p_crc == NULL) ? 0xFFFF : *p_crc;

	for (uint32_t i = 0; i < size; i++)
	{
		crc = (uint8_t)(crc >> 8) | (crc << 8);
		crc ^= p_data[i];
		crc ^= (uint8_t)(crc & 0xFF) >> 4;
		crc ^= (crc << 8) << 4;
		crc ^= ((crc & 0xFF) << 4) << 1;
	}

	return crc;
}

SdUpdatePacket::SdUpdatePacket(string& sd_filename, string& bl_filename, SdkVersion sdkVersion)
{
	// initialize attributes
	m_sdkVersion = sdkVersion;
	m_sdSize = 0;
	m_blSize = 0;
	m_totalSize = 0;
	m_data = NULL;
	m_payloadPos = 0;
	m_payloadSendingIscomplete = false;
	m_packet.length = 0;
	m_data = new char[512000];
	// read sd binary file
	
	ifstream sdfile(sd_filename, ios::in | ios::binary | ios::ate);
	if (sdfile.is_open())
	{
		streampos sdsize;
		sdsize = sdfile.tellg();
		sdfile.seekg(0, ios::beg);
		sdfile.read(m_data, sdsize);
		m_sdSize = sdsize;
		sdfile.close();
	}
	else
	{
		Logger::log("failed to read softdevice binary file");
	}

	// read bl binary file
	
	/*ifstream blfile(bl_filename, ios::in | ios::binary | ios::ate);
	if (blfile.is_open())
	{
		streampos blsize;
		blsize = blfile.tellg();
		blsize = min((int)blsize, MAX_BOOTLOADER_SIZE);
		blfile.seekg(0, ios::beg);
		blfile.read(m_data, blsize);
		m_blSize = blsize;
		blfile.close();
	}
	else
	{
		Logger::log("failed to read bootloader binary file");
	}*/
	m_blSize = 0;
	m_totalSize = m_sdSize + m_blSize;
}

SdUpdatePacket::~SdUpdatePacket()
{
	if (m_data)
		delete[] m_data;
}

Packet& SdUpdatePacket::buildStartPacket()
{
	uint32_t type_packet = 3;
	uint32_t type_image = 1; // soft device
	uint32_t length_sd_image = m_sdSize;
	uint32_t length_bootloader_image = m_blSize;
	uint32_t length_app_image = 0;

	uint32_t size = 0;
	memcpy(m_packet.data + size, &type_packet, sizeof(type_packet));
	size += sizeof(type_packet);
	memcpy(m_packet.data + size, &type_image, sizeof(type_image));
	size += sizeof(type_image);
	memcpy(m_packet.data + size, &length_sd_image, sizeof(length_sd_image));
	size += sizeof(length_sd_image);
	memcpy(m_packet.data + size, &length_bootloader_image, sizeof(length_bootloader_image));
	size += sizeof(length_bootloader_image);
	memcpy(m_packet.data + size, &length_app_image, sizeof(length_app_image));
	size += sizeof(length_app_image);

	m_packet.length = size;
	return m_packet;
}

Packet& SdUpdatePacket::buildInitPacket()
{
	uint32_t type_packet = 1;
	uint8_t device_type[] = { 0xff, 0xff };
	uint8_t device_revision[] = { 0xff, 0xff };
	uint32_t application_version = 1;
	uint16_t length_list_sd = 1;
	uint16_t list_sd = 0xFFFE;// (m_sdkVersion == SDK11) ? 0x0081 : 0x0091;
	uint16_t crc = crc16_compute((uint8_t*)m_data, m_totalSize, NULL);
	uint16_t filler = 0;

	uint32_t size = 0;
	memcpy(m_packet.data + size, &type_packet, sizeof(type_packet));
	size += sizeof(type_packet);
	memcpy(m_packet.data + size, device_type, sizeof(device_type));
	size += sizeof(device_type);
	memcpy(m_packet.data + size, device_revision, sizeof(device_revision));
	size += sizeof(device_revision);
	memcpy(m_packet.data + size, &application_version, sizeof(application_version));
	size += sizeof(application_version);
	memcpy(m_packet.data + size, &length_list_sd, sizeof(length_list_sd));
	size += sizeof(length_list_sd);
	memcpy(m_packet.data + size, &list_sd, sizeof(list_sd));
	size += sizeof(list_sd);
	memcpy(m_packet.data + size, &crc, sizeof(crc));
	size += sizeof(crc);
	memcpy(m_packet.data + size, &filler, sizeof(filler));
	size += sizeof(filler);

	m_packet.length = size;
	return m_packet;
}

Packet& SdUpdatePacket::buildPayloadPacket()
{

	if (m_payloadPos >= m_totalSize){
		m_payloadSendingIscomplete = true;
		m_packet.length = 0;
		return m_packet;
	}
	uint16_t payloadPacketSize = std::min(LENGTH_PAYLOAD_PACKET, (int)(m_totalSize - m_payloadPos));

	uint32_t type_packet = 4;
	uint32_t size = 0;
	memcpy(m_packet.data + size, &type_packet, sizeof(type_packet));
	size += sizeof(type_packet);
	memcpy(m_packet.data + size, m_data + m_payloadPos, payloadPacketSize);
	size += payloadPacketSize;

	m_payloadPos += payloadPacketSize;
	m_packet.length = size;
	return m_packet;
}

Packet& SdUpdatePacket::buildStopPacket()
{
	uint32_t type_packet = 5;

	uint32_t size = 0;
	memcpy(m_packet.data + size, &type_packet, sizeof(type_packet));
	size += sizeof(type_packet);
	m_packet.length = size;
	return m_packet;
}

bool SdUpdatePacket::isAllPayloadSent()
{
	return m_payloadSendingIscomplete;
}

#endif