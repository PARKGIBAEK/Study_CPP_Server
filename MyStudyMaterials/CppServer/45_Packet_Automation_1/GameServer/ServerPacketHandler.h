#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;

/* ��ŶID�� �ش��ϴ� �Լ��� ������ �迭
  GPacketHandler[PacketId](PacketSessionRef&, BYTE*, int32) �̷������� ȣ���Ͽ� ����� �뵵.
  �������� ��Ŷ�� ��Ŷ ó�� �Լ��� ������ �� true�� ��ȯ�ϰ�,
  ���� �ش� PacketId�� �´� ��Ŷ�� ���ų�, ����ũ ��Ŷ���� �����ϰ� false�� ��ȯ�Ѵ�.
  */
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

// TODO : �ڵ�ȭ
enum : uint16
{
	PKT_S_TEST = 1,
	PKT_S_LOGIN = 2,
};

// TODO : �ڵ�ȭ
// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_TEST(PacketSessionRef& session, Protocol::S_TEST& pkt);

class ServerPacketHandler
{
public:
	// TODO : �ڵ�ȭ
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;

		GPacketHandler[PKT_S_TEST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) 
		{ return HandlePacket<Protocol::S_TEST>(Handle_S_TEST, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}

	// TODO : �ڵ�ȭ
	static SendBufferRef MakeSendBuffer(Protocol::S_TEST& pkt) { return MakeSendBuffer(pkt, PKT_S_TEST); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};