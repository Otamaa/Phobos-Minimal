#pragma once

#include <Base/Always.h>
#include <Helpers/CompileTime.h>

class ConnManClassVtbl;
class CommBufferClass;

enum class ConnectionEnum : unsigned char
{
	PACKET_DATA_ACK = 0,
	PACKET_DATA_NOACK = 1,
};

#pragma pack(push, 1)
struct CommHeaderType
{
	__int16 MagicNumber;
	ConnectionEnum Code;
	char forwardto;
	int PacketID;
	char field_8;
	char field_9;
	char field_A;
	char field_B;
	char field_C;
	char field_D;
};
#pragma pack(pop)
static_assert(sizeof(CommHeaderType) == 14);

struct GlobalHeaderType
{
	CommHeaderType Header;
	__int16 ProductID;
};
static_assert(sizeof(GlobalHeaderType) == 16);

class ConnectionClass
{
public:
	ConnManClassVtbl* vtable;
	CommBufferClass* Queue;
	int __resends;
	int __numlost;
	int __percentlost;
	int __missedoverall;
	int __missedmagic;
	DWORD MaxPacketLen;
	GlobalHeaderType* PacketBuf;
	WORD MagicNum;
	DWORD RetryDelta;
	DWORD MaxRetries;
	DWORD Timeout;
	int NumRecNoAck;
	int NumRecAck;
	int NumSendNoAck;
	int NumSendAck;
	int LastSeqID;
	int LastReadID;
};
static_assert(sizeof(ConnectionClass) == 0x4C);

class SendQueueType
{
public:
	int Flags;
	int FirstTime;
	int LastTime;
	int SendCount;
	int BufLen;
	CommHeaderType* Buffer;
	int ExtraLen;
	int ExtraBuffer;
	__int16 owntalk_20;
};
static_assert(sizeof(SendQueueType) == 0x24);