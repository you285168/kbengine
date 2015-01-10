/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KBE_NETWORK_BUNDLE_H
#define KBE_NETWORK_BUNDLE_H

#include "common/common.h"
#include "common/timer.h"
#include "common/objectpool.h"
#include "helper/debug_helper.h"
#include "network/address.h"
#include "network/event_dispatcher.h"
#include "network/endpoint.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/interface_defs.h"

namespace KBEngine { 
namespace Network
{
class NetworkInterface;
class Channel;

#define PACKET_MAX_CHUNK_SIZE() isTCPPacket_ ? (PACKET_MAX_SIZE_TCP - ENCRYPTTION_WASTAGE_SIZE):			\
	(PACKET_MAX_SIZE_UDP - ENCRYPTTION_WASTAGE_SIZE);

#define PACKET_OUT_VALUE(v)																					\
	if(packets_.size() <= 0)																				\
		return *this;																						\
																											\
	Packet* pPacket = packets_[0];																			\
    (*pPacket) >> v;																						\
	if(pPacket->length() == 0)																				\
	{																										\
		RECLAIM_PACKET(isTCPPacket_, pPacket);																\
		packets_.erase(packets_.begin());																	\
	}																										\
																											\
	return *this;																							\


// �Ӷ�����д��������
#define MALLOC_BUNDLE() Network::Bundle::ObjPool().createObject()
#define DELETE_BUNDLE(obj) { Network::Bundle::ObjPool().reclaimObject(obj); obj = NULL; }
#define RECLAIM_BUNDLE(obj) { Network::Bundle::ObjPool().reclaimObject(obj);}

class Bundle : public PoolObject
{
public:
	typedef KBEShared_ptr< SmartPoolObject< Bundle > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj();
	static ObjectPool<Bundle>& ObjPool();
	static void destroyObjPool();
	void onReclaimObject();
	virtual size_t getPoolObjectBytes();

	typedef std::vector<Packet*> Packets;
	
	Bundle(Channel * pChannel = NULL, ProtocolType pt = PROTOCOL_TCP);
	Bundle(const Bundle& bundle);
	virtual ~Bundle();
	
	void newMessage(const MessageHandler& msgHandler);
	void finiMessage(bool isSend = true);

	void clearPackets();

	INLINE MessageLength currMsgLength()const;
	
	INLINE void pCurrMsgHandler(const Network::MessageHandler* pMsgHandler);

	/**
		�������а�������ǰ��δд��İ����ܳ���
	*/
	int32 packetsLength(bool calccurr = true);

	INLINE bool isTCPPacket()const{ return isTCPPacket_; }

	void clear(bool isRecl);
	bool empty() const;
	int packetsSize() const;
	
	INLINE Packets& packets();
	INLINE Packet* pCurrPacket()const;
	INLINE void pCurrPacket(Packet* p);

	INLINE void finiCurrPacket();

	Packet* newPacket();
	
	INLINE MessageID messageID() const { return currMsgID_; }

protected:
	int32 onPacketAppend(int32 addsize, bool inseparable = true);
	void _debugMessages();
public:
    Bundle &operator<<(uint8 value)
    {
		onPacketAppend(sizeof(uint8));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(uint16 value)
    {
		onPacketAppend(sizeof(uint16));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(uint32 value)
    {
		onPacketAppend(sizeof(uint32));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(uint64 value)
    {
		onPacketAppend(sizeof(uint64));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(int8 value)
    {
		onPacketAppend(sizeof(int8));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(int16 value)
    {
		onPacketAppend(sizeof(int16));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(int32 value)
    {
		onPacketAppend(sizeof(int32));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(int64 value)
    {
		onPacketAppend(sizeof(int64));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(float value)
    {
		onPacketAppend(sizeof(float));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(double value)
    {
		onPacketAppend(sizeof(double));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(COMPONENT_TYPE value)
    {
		onPacketAppend(sizeof(int32));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(ENTITY_MAILBOX_TYPE value)
    {
		onPacketAppend(sizeof(int32));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(bool value)
    {
		onPacketAppend(sizeof(int8));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(const std::string &value)
    {
		int32 len = (int32)value.size() + 1; // +1Ϊ�ַ���β����0λ��
		int32 addtotalsize = 0;

		while(len > 0)
		{
			int32 ilen = onPacketAppend(len, false);
			pCurrPacket_->append(value.c_str() + addtotalsize, ilen);
			addtotalsize += ilen;
			len -= ilen;
		}

        return *this;
    }
	
    Bundle &operator<<(const char *str)
    {
		int32 len = (int32)strlen(str) + 1;  // +1Ϊ�ַ���β����0λ��
		int32 addtotalsize = 0;

		while(len > 0)
		{
			int32 ilen = onPacketAppend(len, false);
			pCurrPacket_->append(str + addtotalsize, ilen);
			addtotalsize += ilen;
			len -= ilen;
		}

        return *this;
    }
    
	Bundle &append(Bundle* pBundle)
	{
		KBE_ASSERT(pBundle != NULL);
		return append(*pBundle);
	}

	Bundle &append(Bundle& bundle)
	{
		Packets::iterator iter = bundle.packets_.begin();
		for(; iter!=bundle.packets_.end(); ++iter)
		{
			append((*iter)->data(), (*iter)->length());
		}
		
		if(bundle.pCurrPacket_ == NULL)
			return *this;

		return append(bundle.pCurrPacket_->data(), bundle.pCurrPacket_->length());
	}

	Bundle &append(MemoryStream* s)
	{
		KBE_ASSERT(s != NULL);
		return append(*s);
	}

	Bundle &append(MemoryStream& s)
	{
		if(s.length() > 0)
			return append(s.data() + s.rpos(), s.length());

		return *this;
	}

	Bundle &appendBlob(const std::string& str)
	{
		return appendBlob((const uint8 *)str.data(), str.size());
	}

	Bundle &appendBlob(const char* str, ArraySize n)
	{
		return appendBlob((const uint8 *)str, n);
	}

	Bundle &appendBlob(const uint8 *str, ArraySize n)
	{
		(*this) << n;
		return assign((char*)str, n);
	}

	Bundle &append(const uint8 *str, int n)
	{
		return assign((char*)str, n);
	}

	Bundle &append(const char *str, int n)
	{
		return assign(str, n);
	}

	Bundle &assign(const char *str, int n)
	{
		int32 len = (int32)n;
		int32 addtotalsize = 0;

		while(len > 0)
		{
			int32 ilen = onPacketAppend(len, false);
			pCurrPacket_->append((uint8*)(str + addtotalsize), ilen);
			addtotalsize += ilen;
			len -= ilen;
		}

		return *this;
	}

    Bundle &operator>>(bool &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(uint8 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(uint16 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(uint32 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(uint64 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(int8 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(int16 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(int32 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(int64 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(float &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(double &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(COMPONENT_TYPE &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(ENTITY_MAILBOX_TYPE &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(std::string& value)
    {
        PACKET_OUT_VALUE(value);
    }
private:
	Channel * pChannel_;
	int		numMessages_;
	
	Packet* pCurrPacket_;
	MessageID currMsgID_;
	uint32 currMsgPacketCount_;
	MessageLength1 currMsgLength_;	
	int32 currMsgHandlerLength_;
	size_t currMsgLengthPos_;

	Packets packets_;
	
	bool isTCPPacket_;

	const Network::MessageHandler* pCurrMsgHandler_;

};

}
}

#ifdef CODE_INLINE
#include "bundle.inl"
#endif
#endif // KBE_NETWORK_BUNDLE_H