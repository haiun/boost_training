#pragma once
#include <boost/atomic/atomic.hpp>

enum PacketEnum
{
	NONE,
	LOGIN,
	CHAT,
	END
};

#pragma pack(push, 1)
struct PacketBase
{
	unsigned short id;
	unsigned short size;

	PacketBase(unsigned short _id, unsigned short _size) : id(_id), size(_size) {}
	~PacketBase() {}

	template< typename T >
	T* Cast() { return reinterpret_cast<T*>(this); }
};

struct LoginPacket : public PacketBase
{
	LoginPacket() : PacketBase(LOGIN, sizeof(LoginPacket)) {}
};

struct ChatPacket : public PacketBase
{
	char message[128];

	ChatPacket() : PacketBase(CHAT, sizeof(ChatPacket)) {}
};

struct TimeStampPacket : public PacketBase
{
	DWORD time;

	TimeStampPacket(unsigned short _id, unsigned short _size) : PacketBase(_id, _size)
	{
		time = GetTickCount();
	}
};

#pragma pack(pop, 1)

class WriteCommand
{
public:
	WriteCommand(PacketBase* packet) : pData(packet), ref(1) {}

protected:
	~WriteCommand() { delete pData; }
	void AddRef()
	{
		ref.fetch_add(1);
	}

public:
	unsigned short size() { return pData->size; }

	WriteCommand* Clone()
	{
		AddRef();
		return this;
	}

	void Release()
	{
		uint32_t subValue = ref.fetch_sub(1);
		if (subValue == 0)
		{
			delete this;
		}
	}

	PacketBase* pData;
	boost::atomics::atomic_uint32_t ref;
};