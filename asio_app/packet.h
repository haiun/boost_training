#pragma once

#pragma pack(push, 1)

struct PacketProtocol
{
	unsigned __int16 layer;
	unsigned __int16 command;
};

struct PacketHeader
{
	unsigned __int32 length;
	PacketProtocol protocol;
};

#pragma pack(pop)

inline unsigned __int32 GetLength(void* pPacket)
{
	return *((unsigned __int32*)pPacket);
}
