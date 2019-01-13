#include "Packet.h"
#include "def.h"

Packet::TypeMap Packet::typeMap = {
	{typeid(Data), MessageType::DATA},
	{typeid(RoomList), MessageType::ROOMLIST},
	{typeid(RoomInfo), MessageType::ROOM},
	{typeid(Client), MessageType::CLIENT},
	{typeid(PlayState), MessageType::PLAY_STATE},
	{typeid(TransformProto), MessageType::TRANSFORM},
	{typeid(Vector3Proto), MessageType::VECTOR_3}
};

Packet::InvTypeMap Packet::invTypeMap = {
	{MessageType::DATA, typeid(Data)},
	{MessageType::ROOMLIST, typeid(RoomList)},
	{MessageType::ROOM, typeid(RoomInfo)},
	{MessageType::CLIENT, typeid(Client)},
	{MessageType::PLAY_STATE, typeid(PlayState)},
	{MessageType::TRANSFORM, typeid(TransformProto)},
	{MessageType::VECTOR_3, typeid(Vector3Proto)},
};

Packet::Packet() 
{
	memset(buffer, 0, FOR_IO_SIZE);
	memset(backup, 0, FOR_BAKCUP_SIZE);
	memset(pack, 0, FOR_PACK_SIZE);

	backupBufLength = packBufLength = packBufOffset = 0;
	hMutexObj = CreateMutex(NULL, false, NULL);
}

Packet::~Packet() 
{
	if (hMutexObj != NULL)
		CloseHandle(hMutexObj);
}

void Packet::ClearBuffer()
{
	memset(buffer, 0, FOR_IO_SIZE);
}

bool Packet::GetTypeAndLength(int& type, int& length)
{
	return false;
}

bool Packet::CheckValidType(int& type)
{
	return invTypeMap.find(type) != invTypeMap.end();
}

int Packet::PackMessage(int type, MessageLite* message)
{
	//WaitForSingleObject(hMutexObj, INFINITE);
	ClearBuffer();
	memset(pack, 0, FOR_PACK_SIZE);

	aos = new ArrayOutputStream(pack, FOR_PACK_SIZE);
	cos = new CodedOutputStream(aos);

	if (message != nullptr)
	{
		Serialize(cos, message);
	}
	else 
	{	
		assert(type != -1);
		cos->WriteLittleEndian32(type);
		cos->WriteLittleEndian32(0);
	}

	int rtn = cos->ByteCount();
	memcpy(buffer, pack, rtn);

	delete cos;
	delete aos;
	//ReleaseMutex(hMutexObj);

	return rtn;
}

bool Packet::UnpackMessage(DWORD& bytesTransferred, int& type, int& length, MessageLite*& message)
{
	WaitForSingleObject(hMutexObj, INFINITE);

	packBufOffset = packBufLength = 0;
	memset(pack, 0, FOR_PACK_SIZE);
	// backup이 존재한다면
	if (backupBufLength > 0) {
		memcpy(pack, backup, backupBufLength);
		memcpy(pack + backupBufLength, buffer, (size_t)bytesTransferred);
		packBufLength = backupBufLength + bytesTransferred;
		fprintf(stderr, "[In bakcup branch] offset: %d, length: %d\n", packBufOffset, packBufLength);
	}
	else {
		memcpy(pack, buffer, (size_t)bytesTransferred);
		packBufLength = bytesTransferred;
		fprintf(stderr, "[In buffer branch] offset: %d, length: %d\n", packBufOffset, packBufLength);
	}

	ais = new ArrayInputStream(pack, FOR_PACK_SIZE);
	cis = new CodedInputStream(ais);

	// rtn의 값에 따라 추가 수신여부를 판단
	bool rtn = true;
	unsigned int _type, _length;
	while (packBufLength > 0) 
	{
		if (packBufLength < 8) {
			fprintf(stderr, "packBufLength < 8 => Can't get header info...\n");
			fprintf(stderr, "[After Length < 8] offset: %d, length: %d\n", packBufOffset, packBufLength);

			memset(backup, 0, FOR_BAKCUP_SIZE);
			memcpy(backup, pack + packBufOffset, packBufLength);
			backupBufLength = packBufLength;
			rtn = false;
			break;
		}
		cis->ReadLittleEndian32(&_type);
		type = (int)_type;
		packBufOffset += 4; packBufLength -= 4;

		fprintf(stderr, "[After Type] offset: %d, length: %d\n", packBufOffset, packBufLength);

		cis->ReadLittleEndian32(&_length);
		length = (int)_length;
		packBufOffset += 4; packBufLength -= 4;

		fprintf(stderr, "[After Length] offset: %d, length: %d\n", packBufOffset, packBufLength);

		fprintf(stderr, "[After Type and Length] Type: %d, Body Length: %d\n", type, length);
		if (type == 0 && length == 0) break;
		if (length > 0)
		{	
			// Message Body가 정확히 Length와 일치할 때,
			if (length <= packBufLength) {
				Deserialize(type, cis, message);
				packBufOffset += length;
				packBufLength -= length;

				fprintf(stderr, "[After Body] offset: %d, length: %d\n", packBufOffset, packBufLength);
			}
			else {
				memset(backup, 0, FOR_BAKCUP_SIZE);
				memcpy(backup, pack + packBufOffset - 8, packBufLength + 8);
				backupBufLength = packBufLength + 8;
				rtn = false;
				break;
			}
		}
	}

	delete cis;
	delete ais;
	ClearBuffer();
	ReleaseMutex(hMutexObj);
	return rtn;
}

void Packet::Serialize(CodedOutputStream*& cos, MessageLite*& message)
{
	int contentType = typeMap[typeid(*message)];
	int contentLength = message->ByteSize();

	//fprintf(stderr, "[ContentType]: %d, [ContentLength]: %d\n", contentType, contentLength);

	cos->WriteLittleEndian32(contentType);
	cos->WriteLittleEndian32(contentLength);
	message->SerializeToCodedStream(cos);
}

void Packet::Deserialize(int& type, CodedInputStream*& cis, MessageLite*& message)
{
	if (!CheckValidType(type)) {
		fprintf(stderr, "Invalid type... %d\n", type);
		return;
	}

	if (type == MessageType::DATA)
	{
		message = new Data();
	}
	else if (type == MessageType::PLAY_STATE)
	{
		message = new PlayState();
	}
	else if (type == MessageType::TRANSFORM)
	{
		message = new TransformProto();
	}
	else if (type == MessageType::VECTOR_3)
	{
		message = new Vector3Proto();
	}
	message->ParseFromCodedStream(cis);
}

Packet* Packet::AllocatePacket()
{
	Packet* lpPacket = new Packet();
	return lpPacket;
}

void Packet::DeallocatePacket(Packet* lpPacket)
{
	assert(lpPacket != NULL);
	delete lpPacket;
}