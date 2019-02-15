/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpPacketType.h
* Description		: 	RtpPacketType operation center
* Created			: 	2017.09.28.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef RTP_PACKET_TYPE
#define RTP_PACKET_TYPE


#include <stdlib.h>
#include <stdio.h>

typedef struct RtpHeader
{
#ifdef RTP_BIG_ENDIAN
	unsigned short Version:2;//�汾�ţ�V����������־ʹ�õ�RTP�汾��ռ2λ����ǰЭ��汾��Ϊ2
	unsigned short Pad:1;//���λ��P��������־��ռ1λ�����P=1�����RTP����β���Ͱ������ӵ�����ֽ�
	unsigned short Extend:1;//��չλ��X������չ��־��ռ1λ�����X=1������RTP�̶�ͷ������͸���һ����չͷ��
	unsigned short CsrcCount:4;//CSRC��������CC����CSRC��������ռ4λ��ָʾ�̶�ͷ��������ŵ�CSRC ��ʶ���ĸ���

	unsigned short Mark:1;//���λ��M������ǣ�ռ1λ��һ����ԣ�������Ƶ�����һ֡�Ľ�����������Ƶ����ǻỰ�Ŀ�ʼ
	unsigned short PayloadType:7;//�غ����ͣ�PayloadType���� ��Ч�������ͣ�ռ7λ������˵��RTP��������Ч�غɵ�����
#else //little endian
	unsigned short CsrcCount:4;//CSRC��������CC����CSRC��������ռ4λ��ָʾ�̶�ͷ��������ŵ�CSRC ��ʶ���ĸ���
	unsigned short Extend:1;//��չλ��X������չ��־��ռ1λ�����X=1������RTP�̶�ͷ������͸���һ����չͷ��
	unsigned short Pad:1;//���λ��P��������־��ռ1λ�����P=1�����RTP����β���Ͱ������ӵ�����ֽ�
	unsigned short Version:2;//�汾�ţ�V����������־ʹ�õ�RTP�汾��ռ2λ����ǰЭ��汾��Ϊ2

	unsigned short PayloadType:7;//�غ����ͣ�PayloadType���� ��Ч�������ͣ�ռ7λ������˵��RTP��������Ч�غɵ�����
	unsigned short Mark:1;//���λ��M������ǣ�ռ1λ��һ����ԣ�������Ƶ�����һ֡�Ľ�����������Ƶ����ǻỰ�Ŀ�ʼ
#endif
	unsigned short wSeq;//���кţ�SN����ռ16λ�����ڱ�ʶ�����������͵�RTP���ĵ����кţ�ÿ����һ�����ģ����к���1
	unsigned int dwTimestamp;//ʱ���(Timestamp): ռ32λ����¼�˸ð������ݵĵ�һ���ֽڵĲ���ʱ��
	unsigned int dwSSRC;//ͬ��Դ��ʶ��(SSRC)��ռ32λ�����ڱ�ʶͬ����Դ��ͬ��Դ����ָRTP��������Դ����ͬһ��RTP�Ự�в�����������ͬ��SSRCֵ
}T_RtpHeader;//size 12

/*****************************************************************************
-Class			: RtpPacket
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/28	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RtpPacket
{
public:
	unsigned char ParseRtpPacketF(unsigned char *i_aucRtpPayloadData);
	unsigned char ParseRtpPacketNRI(unsigned char *i_aucRtpPayloadData);
	unsigned char ParseRtpPacketType(unsigned char *i_aucRtpPayloadData);
};

/*****************************************************************************
-Class			: NALU
-Description	: NALU�غ����͵�RTP��
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/28	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class NALU: public RtpPacket
{
public:
	bool IsThisPacketType(unsigned char i_ucRtpPacketType);
	bool GetEndFlag();
	unsigned int CopyVideoData(unsigned char *i_aucRtpPacketData,unsigned int i_dwDataLen,unsigned char *o_aucNaluData);
};


/*****************************************************************************
-Class			: FU_A
-Description	: FU_A�غ����͵�RTP��
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/28	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class FU_A: public RtpPacket
{
public:
	bool IsThisPacketType(unsigned char i_ucRtpPacketType);
	bool GetEndFlag();
	unsigned int CopyVideoData(unsigned char *i_aucRtpPacketData,unsigned int i_dwDataLen,unsigned char *o_aucNaluData);
	
private:
	bool IsPacketStart(unsigned char *i_aucRtpPayloadData);
	bool IsPacketEnd(unsigned char *i_aucRtpPayloadData);
	unsigned char GetNaluHeader(unsigned char *i_aucRtpPayloadData);
	const static unsigned char FU_A_TYPE;
	bool m_blStartFlag;
	bool m_blEndFlag;
};




#endif
