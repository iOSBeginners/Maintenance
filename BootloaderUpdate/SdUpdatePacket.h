#ifndef __SD_UPDATE_PACKET__
#define __SD_UPDATE_PACKET__

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
#include <string>

enum SdkVersion
{
	SDK11,
	SDK12
};
enum DeviceType
{
	BRIDGE,
	ANTENNA,
	BEACON,
	NO_DEVICE
};

#define MAX_LENGTH_PACKET 300
#define LENGTH_PAYLOAD_PACKET 200

struct Packet
{
	uint8_t data[MAX_LENGTH_PACKET];
	uint16_t length;
};
#if 0
class SdUpdatePacket
{
public:
	SdUpdatePacket(std::string& sd_filename, std::string& bl_filename, SdkVersion sdkVersion);
	~SdUpdatePacket();
	Packet& buildStartPacket();
	Packet& buildInitPacket();
	Packet& buildPayloadPacket();
	Packet& buildStopPacket();
	bool isAllPayloadSent();

private:
	uint32_t m_sdSize;
	uint32_t m_blSize;
	uint32_t m_totalSize;
	char* m_data;
	SdkVersion m_sdkVersion;
	uint32_t m_payloadPos;
	bool m_payloadSendingIscomplete;
	Packet m_packet;
};
#endif 

#endif 