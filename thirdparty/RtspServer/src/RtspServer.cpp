/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtspServer.cpp
* Description		: 	RtspServer operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <pthread.h>

#include "Definition.h"
#include "RtpPacket.h"
#include "Base64.hh"
#include "Tools.h"
#include "RtspServer.h"

using std::cout;//��Ҫ<iostream>
using std::hex;
using std::endl;
using std::stringstream;

string RtspServer::m_astrCmd[]={"OPTIONS", "DESCRIBE", "SETUP", "TEARDOWN", "PLAY", "PAUSE"};

/*****************************************************************************
-Fuction		: RtspServer
-Description	: �����ĳɴ����д�ص���������ʽ
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtspServer::RtspServer(FILE * i_pVideoFile,FILE * i_pAudioFile)
{
    m_pVideoFile = i_pVideoFile;
    m_pAudioFile = i_pAudioFile;
    m_SessionList.clear();
    m_strURL.assign("");
    m_strIP.assign("");
    m_wPort = 0;
    m_dwBandwidth =500;//Ĭ��500kbps
    m_iSessionCount =0;
    m_HandleCmdMap.insert(make_pair("OPTIONS", &RtspServer::HandleCmdOPTIONS)); //WirelessNetManage::ָ��������,����ͷ�ļ��ﶨ������ʱҲҪָ��
    m_HandleCmdMap.insert(make_pair("DESCRIBE", &RtspServer::HandleCmdDESCRIBE)); //��ĳ�Ա��������ָ�룬����Ҫ�� &
    m_HandleCmdMap.insert(make_pair("SETUP", &RtspServer::HandleCmdSETUP));
    m_HandleCmdMap.insert(make_pair("PLAY", &RtspServer::HandleCmdPLAY));
    m_HandleCmdMap.insert(make_pair("PAUSE", &RtspServer::HandleCmdPAUSE));
    m_HandleCmdMap.insert(make_pair("TEARDOWN", &RtspServer::HandleCmdTEARDOWN));

    
    //�����߳�
    pthread_t tSessionHandleID;
	if( pthread_create(&tSessionHandleID,NULL,RtspServer::SessionHandleThread, (void *)this) != 0 )
	{
		perror("SessionHandleThread pthread_create err\r\n");
	}

	memset(m_abSPS,0,sizeof(m_abSPS));
	memset(m_abPPS,0,sizeof(m_abPPS));
    m_iSPS_Len = 0;
    m_iPPS_Len = 0;
    m_pRtpPacket = new RtpPacket();
}

/*****************************************************************************
-Fuction		: ~RtspServer
-Description	: ~RtspServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtspServer::~RtspServer()
{
    m_pVideoFile = NULL;
    m_pAudioFile = NULL;
    delete m_pRtpPacket;
    m_pRtpPacket = NULL;
}
/*****************************************************************************
-Fuction		: GetIpAndPort
-Description	: GetIpAndPort
-Input			: " rtsp://192.168.1.88:554/1"
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::GetIpAndPort(string * i_pstrURL,string *o_Ip,unsigned short *o_wPort)
{
	int iRet=FALSE;
	string IP("");
	unsigned int iIpStartPos;
	unsigned int iIpEndPos;
	unsigned int iPortEndPos;
	iIpStartPos=i_pstrURL->find("//",0);
	if(iIpStartPos!=5)
	{	
		cout<<"GetIp err:"<<iIpStartPos<<endl;
	}
	else
	{
		iIpEndPos=i_pstrURL->find(":",iIpStartPos+2);
		if(iIpEndPos==string::npos)
		{
			cout<<"GetIp1 err:"<<iIpEndPos<<endl;
		}
		else
		{
			iPortEndPos=i_pstrURL->find("/",iIpEndPos+1);
			if(iPortEndPos==string::npos)
			{
				cout<<"GetPort err:"<<iPortEndPos<<endl;
			}
			else
			{
				//IP.assign(i_URL,iIpStartPos+2,iIpEndPos-iIpStartPos-2);//����֮��Ŀ�����ǳ��������������ͷ�ͬһ���ڴ棬
				//memcpy(o_Ip,&IP,sizeof(string));//�������double free or corruption (top)�Ĵ���
				o_Ip->assign(*i_pstrURL,iIpStartPos+2,iIpEndPos-iIpStartPos-2);				
				*o_wPort=atoi(i_pstrURL->substr(iIpEndPos+1,iPortEndPos-iIpEndPos-1).c_str());
				cout<<"IP:"<<o_Ip->c_str()<<" Port:"<<i_pstrURL->substr(iIpEndPos+1,iPortEndPos-iIpEndPos-1).c_str()<<endl;
				iRet=TRUE;
			}
		}
	}
	return iRet;
}

/*****************************************************************************
-Fuction		: ConnectHandle
-Description	: һ���׽������Ӱ���������ip���������˿ں�
�ͻ���ip���ͻ��˶˿ںš�����ĸ�Ԫ��ֻҪ��һ����ͬ��
�Ǿ�����ȫ��ͬ���������ӡ�
�ͻ�����Ҫ֪�������ĸ���������������Ҫ�����������ip�Ͷ˿ں� �Ϳ���
�������ӡ�
���������ǿͻ��˷���ģ����������Դ����ӹ�������Ϣ�л�ȡ��
�ͻ��˵�ip�Ͷ˿ں�(�������ڣ� tcp/ipЭ�����һ�����ݰ�
����Դ��ַ����Ϣ(ip and port)��Ŀ�ĵ�ַ����Ϣ(ip and port),�����tcp/ip�涨��)��
����(�����������ȴ�����)ֻ�贫���������
ip(���ʹ�ñ���������IP��ipҲ���Բ�Ҫ)�Ͷ˿ںż��ɣ�
����ʹ��sockettool��������֤������ۡ�
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::ConnectHandle(char *i_strURL)
{
    int iRet=FALSE;
    int iClientSocketFd=-1;
    T_Session tSession={0};
    m_strURL.assign(i_strURL);
    GetIpAndPort(&m_strURL,&m_strIP,&m_wPort);
	if(FALSE==TcpServer::Init(m_strIP,m_wPort))//server socket handle bind listen
	{
	}
	else
	{
	    iClientSocketFd = TcpServer::Accept();
        if(iClientSocketFd<0)
        {
        }
        else
        {//���سɹ����ʾ��һ�����Ӳ�������������Ӽ��뵽����
            tSession.iTrackNumber=m_iSessionCount;//��Ƶ��Ӧһ��rtp�Ự����Ƶ��Ӧһ��rtp�Ự,�����Ự�໥������һ��rtp�Ự��Ӧһ��TrackId
            m_iSessionCount++;//һ��rtsp����Ự���԰�������rtp�Ự��
            
            gettimeofday(&tSession.tCreateTime, NULL);
            tSession.iClientSocketFd = iClientSocketFd;//�Ự�������ݵ�ʱ��ʹ�����socket
            tSession.eRtspState=INIT;
            tSession.pVideoRtpSession=NULL;
            tSession.pAudioRtpSession=NULL;
            tSession.dwSessionId=0;
            m_SessionList.push_back(tSession);//�����̴߳����������
        }
        iRet=TRUE;
	}
    return iRet;
	
}

/*****************************************************************************
-Fuction		: SessionHandle
-Description	: SessionHandle
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::SessionHandle()
{
	int iRet=FALSE;	
    //T_Session tSession={0};
    list<T_Session> ::iterator Iter;
    timeval tTimeVal={0};
    string strMsg("");
    string strSendMsg("");
    char acRecvBuf[3*1024]={0};
    int iRecvLen=0;
    
    while(1)
    {
        if(true == m_SessionList.empty())
        {
        }
        else
        {
            //memset(tSession,0,sizeof(tSession));
            for(Iter=m_SessionList.begin();Iter!=m_SessionList.end();Iter++)
            {
                RtspStreamHandle(&*Iter);//���Ե����߳�ע�����,����rtsp��Ҫ��Ƶ����������Է�ǰ��
                
                strSendMsg.clear();
                strMsg.clear();
                memset(&tTimeVal,0,sizeof(tTimeVal));
                tTimeVal.tv_sec      = 0;//��ʱʱ�䣬��ʱ���ش���
                tTimeVal.tv_usec     = 100*1000;
                memset(acRecvBuf,0,sizeof(acRecvBuf));
                iRecvLen=0;
                if(FALSE == TcpServer::Recv(acRecvBuf,&iRecvLen,sizeof(acRecvBuf),Iter->iClientSocketFd,&tTimeVal))
                {
                }
                else
                {
                    strMsg.assign(acRecvBuf);
                    //RtspCmdHandle(&Iter->first);
                    cout<<"RtspCmdHandle State:"<<Iter->eRtspState<<" SocketFd:"<<Iter->iClientSocketFd<<endl;
                    if(FALSE==RtspCmdHandle(&*Iter,&strMsg,&strSendMsg))//��iter���н�����,��ȡ��ַ������ֱ��ʹ��Iter
                    {
                        cout<<"RtspCmdHandle err:"<<Iter->eRtspState<<",session:"<<Iter->dwSessionId<<" will be erased"<<endl;
                        delete Iter->pVideoRtpSession;
                        delete Iter->pAudioRtpSession;
                        m_SessionList.erase(Iter);
                    }
                    else
                    {
                        TcpServer::Send((char *)strSendMsg.c_str(),strSendMsg.length(),Iter->iClientSocketFd);
                    }

                }
            }
        }
        usleep(100*1000);
    }

	return iRet;
}

/*****************************************************************************
-Fuction		: RtspCmdHandle
-Description	: RtspCmdHandle
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::RtspCmdHandle(T_Session *i_ptSession,string *i_pstrMsg,string *o_pstrMsg)
{
    int iRet=FALSE;
    int iCSeq;
    unsigned int dwCmdPos;
    unsigned int dwCSeqPos;
    unsigned int dwCSeqEndPos;
    string strCmd("");    
	map<string, HandleCmd> ::iterator HandleCmdMapIter;

    dwCmdPos=i_pstrMsg->find("rtsp:");
    if(dwCmdPos==string::npos)
    {
        cout<<"RtspCmdHandle can not find rtsp:"<<dwCmdPos<<endl;
    }
    else
    {
        strCmd.assign(i_pstrMsg->substr(0,dwCmdPos-1));
        
        dwCSeqPos=i_pstrMsg->find("CSeq:");
        if(dwCSeqPos==string::npos)
        {
            cout<<"RtspCmdHandle can not find CSeq:"<<dwCSeqPos<<endl;
        }
        else
        {
            dwCSeqEndPos=i_pstrMsg->find("\r\n",dwCSeqPos);
            if(dwCSeqEndPos==string::npos)
            {
                cout<<"RtspCmdHandle can not find CSeq:"<<dwCSeqEndPos<<endl;
            }
            else
            {
            
                iCSeq = atoi(i_pstrMsg->substr(dwCSeqPos+strlen("CSeq:"),dwCSeqEndPos-(dwCSeqPos+strlen("CSeq:"))).c_str());
            
                HandleCmdMapIter=m_HandleCmdMap.find(strCmd.c_str());
                if(HandleCmdMapIter == m_HandleCmdMap.end())
                {
                    cout<<"Can not find:"<<strCmd<<endl;
                }
                else
                {
                    iRet=(this->*(HandleCmdMapIter->second))(i_ptSession,i_pstrMsg,iCSeq,o_pstrMsg);
                    if(FALSE==iRet)
                    {
                    }
                    else
                    {
                    }
                }
            }    
        }    
    }    
    
    return iRet;
}

/*****************************************************************************
-Fuction		: HandleCmdOPTIONS
-Description	: HandleCmdOPTIONS
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::HandleCmdOPTIONS(T_Session *i_ptSession,string *i_pstrMsg,int i_iCSeq,string *o_pstrMsg)
{
    int iRet=FALSE;
	stringstream Msg("");
	
	Msg<<RTSP_VERSION<<RTSP_RESPONSE_OK<<"\r\n";
	Msg << "CSeq: " << i_iCSeq<< "\r\n";
    Msg <<GetDateHeader()<< "\r\n";
	Msg << "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE"<< "\r\n";//�����Ż�Ϊ��ʹ��m_astrCmd�ַ�������
	Msg << "\r\n";

    iRet = TRUE;
	o_pstrMsg->assign(Msg.str());

    return iRet;
}

/*****************************************************************************
-Fuction		: HandleCmdDESCRIBE
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::HandleCmdDESCRIBE(T_Session *i_ptSession,string *i_pstrMsg,int i_iCSeq,string *o_pstrMsg)
{
    int iRet=FALSE;
	stringstream Msg("");
    string strSDP("");
    
    if(FALSE == GenerateSDP(i_ptSession,&strSDP))
    {
        Msg<<RTSP_VERSION<<RTSP_RESPONSE_ERROR_404<<"\r\n";
        Msg << "CSeq: " << i_iCSeq<< "\r\n";
        Msg <<GetDateHeader()<< "\r\n";
        Msg << "\r\n";
    }
    else
    {
        Msg<<RTSP_VERSION<<RTSP_RESPONSE_OK<<"\r\n";
        Msg << "CSeq: " << i_iCSeq<< "\r\n";
        Msg <<GetDateHeader()<< "\r\n";
        Msg << "Content-Base: "<<m_strURL<< "/\r\n";
        Msg <<"Content-Type: application/sdp\r\n";
        Msg << "Content-Length: "<<strSDP.length()<< "\r\n";
        Msg << "\r\n";

        Msg <<strSDP;
    }

    iRet = TRUE;
	o_pstrMsg->assign(Msg.str());

    return iRet;
}

/*****************************************************************************
-Fuction		: HandleCmdSETUP
-Description	: ����ȷ��ת����ƣ�����RTSP�Ự��
�ͻ����ܹ�����һ��SETUP����Ϊ���ڲ��ŵ�ý�����ı䴫�����������������ͬ����Щ�����ĸı䡣
���ǲ�ͬ�⣬��������Ӧ����"455 Method Not Valid In This State"��
Request�е�Transportͷ�ֶ�ָ���˿ͻ��˿ɽ��ܵ����ݴ��������Response�е�Transport ͷ�ֶΰ������ɷ�����ѡ���Ĵ������

������ý��Ự����֪RTSP�����׼����Դ���Դ�������һ������
RTSP/<RTSP VERSION><BLANK><STATE ID><BLANK><STATE DESCRIBE>\r\nCSeq:<BLANK><COMMAND SEQUENCE>\r\n<OTHER>\r\n<SESSION ID>\r\n\r\n
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::HandleCmdSETUP(T_Session *i_ptSession,string *i_pstrMsg,int i_iCSeq,string *o_pstrMsg)
{
    int iRet=FALSE;
	stringstream Msg("");
	const char *strPatten=NULL;
	regmatch_t atMatch[MAX_MATCH_NUM];
	string strStreamTransportProtocol("");
    string strCastMethod("");
    string strRtpPort("");
	string strRtcpPort("");
	string strTrackID("");
    if(i_ptSession->eRtspState!=INIT && i_ptSession->eRtspState!=SETUP_PLAY_READY)
    {
        Msg<<RTSP_VERSION<<RTSP_RESPONSE_METHOD_NOT_VALID_455<<"\r\n";
        Msg << "CSeq: " << i_iCSeq<< "\r\n";
        Msg <<GetDateHeader()<< "\r\n";
        Msg << "\r\n";
    }
    else
    {
        strPatten=".*Transport: ([A-Z/]+);([a-z]+);client_port=([0-9]+)-([0-9]+).*";
        //strPatten=".*Transport: "RTSP_TRANSPORT_BASE"[A-Z]+;[a-z]+;client_port=[0-9]+-[0-9]+.*";//Transport: RTP/AVP/UDP;unicast;client_port=10330-10331
        memset(atMatch,0,sizeof(atMatch));//ʹ��RTP���䣨RTP/AVP/UDP,RTPoverUDP,һ��ΪRTP+UDP�������䷽ʽΪ������unicast����RTP��RTCP�Ķ˿ںŷֱ�Ϊ10330��10331��client_port=10330-10331��
        if(REG_NOERROR!=Tools::Instance()->Regex(strPatten,(char *)i_pstrMsg->c_str(),atMatch))
        {
            Msg<<RTSP_VERSION<<RTSP_RESPONSE_UNSUPPORTED_TRANSPORT_461<<"\r\n";
            Msg << "CSeq: " << i_iCSeq<< "\r\n";
            Msg <<GetDateHeader()<< "\r\n";
            Msg << "\r\n";
        }
        else
        {
            strStreamTransportProtocol.assign(*i_pstrMsg,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);//*i_pstrMsg�ᱨ��
            strCastMethod.assign(*i_pstrMsg,atMatch[2].rm_so,atMatch[2].rm_eo-atMatch[2].rm_so);
            strRtpPort.assign(*i_pstrMsg,atMatch[3].rm_so,atMatch[3].rm_eo-atMatch[3].rm_so);
            strRtcpPort.assign(*i_pstrMsg,atMatch[4].rm_so,atMatch[4].rm_eo-atMatch[4].rm_so);
            cout<<"strStreamTransportProtocol "<<strStreamTransportProtocol<<endl;
            cout<<"strCastMethod "<<strCastMethod<<endl;
            cout<<"strRtpPort "<<strRtpPort<<endl;
            cout<<"strRtcpPort "<<strRtcpPort<<endl;
            
            strPatten=".*/([A-Za-z0-9]+) "RTSP_VERSION".*";//SETUP rtsp://127.0.0.1/ansersion/track1 RTSP/1.0��һ��setup����ֻ��һ��trackid
            memset(atMatch,0,sizeof(atMatch));//ʹ��RTP���䣨RTP/AVP/UDP,RTPoverUDP,һ��ΪRTP+UDP�������䷽ʽΪ������unicast����RTP��RTCP�Ķ˿ںŷֱ�Ϊ10330��10331��client_port=10330-10331��
            if(REG_NOERROR!=Tools::Instance()->Regex(strPatten,(char *)i_pstrMsg->c_str(),atMatch))
            {
                Msg<<RTSP_VERSION<<RTSP_RESPONSE_UNSUPPORTED_TRANSPORT_461<<"\r\n";
                Msg << "CSeq: " << i_iCSeq<< "\r\n";
                Msg <<GetDateHeader()<< "\r\n";
                Msg << "\r\n";
            }
            else
            {
                strTrackID.assign(*i_pstrMsg,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
                cout<<"strTrackID "<<strTrackID<<endl;
                char strCurrentTrackID[50]={0};                
                //int iTrackIdType=-1;//-1 err,0 video,1 audio
                bool blTrackIdIsVideo=false;
                bool blTrackIdIsAudio=false;
                snprintf(strCurrentTrackID,sizeof(strCurrentTrackID),"track%d",2*i_ptSession->iTrackNumber+1);//video trackid
                if(string::npos==strTrackID.find(strCurrentTrackID)|| m_pVideoFile==NULL)
                {
                    cout<<"Video TrackID err:"<<strTrackID<<" TrackId:"<<2*i_ptSession->iTrackNumber+1<<" m_pVideoFile:"<<m_pVideoFile<<endl;
                    memset(strCurrentTrackID,0,sizeof(strCurrentTrackID));
                    snprintf(strCurrentTrackID,sizeof(strCurrentTrackID),"track%d",2*i_ptSession->iTrackNumber+2);//audio trackid
                    if(string::npos==strTrackID.find(strCurrentTrackID) ||m_pAudioFile==NULL)
                    {
                        cout<<"Audio TrackID err:"<<strTrackID<<" TrackId:"<<2*i_ptSession->iTrackNumber+2<<" m_pAudioFile:"<<m_pAudioFile<<endl;
                        Msg<<RTSP_VERSION<<RTSP_RESPONSE_UNSUPPORTED_TRANSPORT_461<<"\r\n";
                        Msg << "CSeq: " << i_iCSeq<< "\r\n";
                        Msg <<GetDateHeader()<< "\r\n";
                        Msg << "\r\n";
                    }
                    else
                    {
                        blTrackIdIsAudio=true;
                    }
                }
                else
                {
                    blTrackIdIsVideo=true;
                }
                string strClientIP(Tools::Instance()->UseSocketGetIP(i_ptSession->iClientSocketFd));
                int iRtpSocket=0,iRtcpSocket=0;
                if(false==blTrackIdIsAudio && false == blTrackIdIsVideo)
                {
                    iRet=FALSE;
                }
                else if(true == blTrackIdIsVideo)
                {
                    if(NULL != i_ptSession->pVideoRtpSession)
                        delete i_ptSession->pVideoRtpSession;
                    i_ptSession->pVideoRtpSession=new RtpSession(0);
                    if(strStreamTransportProtocol.find(RTSP_TRANSPORT_RTP_OVER_TCP)!=string::npos)    
                        iRet=i_ptSession->pVideoRtpSession->Init(true,m_strIP,strClientIP,(unsigned short)atoi(strRtpPort.c_str()),(unsigned short)atoi(strRtcpPort.c_str()));
                    else
                        iRet=i_ptSession->pVideoRtpSession->Init(false,m_strIP,strClientIP,(unsigned short)atoi(strRtpPort.c_str()),(unsigned short)atoi(strRtcpPort.c_str()));
                    iRtpSocket = i_ptSession->pVideoRtpSession->GetRtpSocket();
                    iRtcpSocket =i_ptSession->pVideoRtpSession->GetRtcpSocket();
                }
                else
                {
                    if(NULL != i_ptSession->pAudioRtpSession)
                        delete i_ptSession->pAudioRtpSession;
                    i_ptSession->pAudioRtpSession=new RtpSession(1);
                    if(strStreamTransportProtocol.find(RTSP_TRANSPORT_RTP_OVER_TCP)!=string::npos)    
                        iRet=i_ptSession->pAudioRtpSession->Init(true,m_strIP,strClientIP,(unsigned short)atoi(strRtpPort.c_str()),(unsigned short)atoi(strRtcpPort.c_str()));
                    else
                        iRet=i_ptSession->pAudioRtpSession->Init(false,m_strIP,strClientIP,(unsigned short)atoi(strRtpPort.c_str()),(unsigned short)atoi(strRtcpPort.c_str()));
                    iRtpSocket = i_ptSession->pAudioRtpSession->GetRtpSocket();
                    iRtcpSocket =i_ptSession->pAudioRtpSession->GetRtcpSocket();
                }
                if(FALSE==iRet)
                {
                    cout<<"i_ptSession->pRtpSession->Init err"<<endl;
                    Msg<<RTSP_VERSION<<RTSP_RESPONSE_INTERNAL_SERVER_ERROR_500<<"\r\n";
                    Msg << "CSeq: " << i_iCSeq<< "\r\n";
                    Msg <<GetDateHeader()<< "\r\n";
                    Msg << "\r\n";
                }
                else
                {
                    char strSession[50]={0};
                    const char * strSessionFmt="Session: %08x;";//client��Ҫ�ֺ�
                    i_ptSession->eRtspState=SETUP_PLAY_READY;
                    //RTSP/<RTSP VERSION><BLANK><STATE ID><BLANK><STATE DESCRIBE>\r\nCSeq:<BLANK><COMMAND SEQUENCE>\r\n<OTHER>\r\n<SESSION ID>\r\n\r\n
                    Msg<<RTSP_VERSION<<RTSP_RESPONSE_OK<<"\r\n";
                    Msg << "CSeq: " << i_iCSeq<< "\r\n";
                    Msg <<GetDateHeader()<< "\r\n";
                    //Transport: RTP/AVP;unicast;destination=127.0.0.1;source=127.0.0.1;client_port=10330-10331;server_port=6970-6971
                    if(strStreamTransportProtocol.find(RTSP_TRANSPORT_RTP_OVER_TCP)!=string::npos)//multicast streams can't be sent via TCP ����������������жϲ����ش�������ͻ���    
                    {
                        Msg <<"Transport: RTP/AVP/TCP;unicast;destination="<<strClientIP<<";source="<<m_strIP<<";client_port="<<strRtpPort<<"-"<<
                        strRtcpPort<<";server_port="<<Tools::Instance()->UseSocketGetPort(iRtpSocket)<<"-"<<Tools::Instance()->UseSocketGetPort(iRtcpSocket)<< "\r\n";
                    }
                    else
                    {
                        Msg <<"Transport: RTP/AVP;unicast;destination="<<strClientIP<<";source="<<m_strIP<<";client_port="<<strRtpPort<<"-"<<
                        strRtcpPort<<";server_port="<<Tools::Instance()->UseSocketGetPort(iRtpSocket)<<"-"<<Tools::Instance()->UseSocketGetPort(iRtcpSocket)<< "\r\n";
                    }
                    i_ptSession->dwSessionId=Tools::Instance()->GetRandom();//<SESSION ID>������˽�������Դ��ͨ���ñ�ʶ������ý������Դ��
                    snprintf(strSession,sizeof(strSession),strSessionFmt,i_ptSession->dwSessionId);//"Session: SESSION ID"��PLAY�����Դ�Ϊ����
                    Msg <<strSession<< "\r\n";
                    Msg << "\r\n";
                }
            }
        }
    }

    iRet = TRUE;
	o_pstrMsg->assign(Msg.str());

    return iRet;
}

/*****************************************************************************
-Fuction		: HandleCmdPLAY
-Description	: 

PLAY������֪������ͨ��SETUP��ָ���Ļ��ƿ�ʼ�������� ��
����δ�յ�SETUP����ĳɹ�Ӧ��֮ǰ���ͻ��˲����Է���PLAY����
PLAY������������ʱ�䣨normal play time����λ��ָ����Χ����ʼ����
���Ҵ���������ֱ�����ŷ�Χ������
PLAY������ܱ��ܵ�����pipelined��������������У�queued����
���������뽫PLAY����ŵ�����������ִ�С�Ҳ����˵��
��һ��PLAY������Ҫ�ȴ�ǰһ��PLAY������ɲ��ܵõ�ִ��

-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::HandleCmdPLAY(T_Session *i_ptSession,string *i_pstrMsg,int i_iCSeq,string *o_pstrMsg)
{
    int iRet=FALSE;
	stringstream Msg("");
	
	if (i_ptSession->eRtspState!= SETUP_PLAY_READY && i_ptSession->eRtspState != PLAYING) 
	{//��ü��ϱȶ�session id
        Msg<<RTSP_VERSION<<RTSP_RESPONSE_METHOD_NOT_VALID_455<<"\r\n";
        Msg << "CSeq: " << i_iCSeq<< "\r\n";
        Msg <<GetDateHeader()<< "\r\n";
        Msg << "\r\n";
        cout<<"HandleCmdPLAY state err:"<<i_ptSession->eRtspState<<endl;
	}
    else
    {
        char strSession[50]={0};
        const char * strSessionFmt="Session: %08x";
        snprintf(strSession,sizeof(strSession),strSessionFmt,i_ptSession->dwSessionId);
        
        Msg<<RTSP_VERSION<<RTSP_RESPONSE_OK<<"\r\n";//Range: npt= ����Ϣ��������
        Msg << "CSeq: " << i_iCSeq<< "\r\n";//Rangeͷ���ܰ���һ��ʱ��������ò�����UTC��ʽָ���˲��ſ�ʼ��ʱ��,
        Msg <<GetDateHeader()<< "\r\n";//��������ָ��ʱ����յ���Ϣ����ô����������ʼ,
        Msg <<strSession<< "\r\n";//ʱ�����������������ͬ���Ӳ�ͬ����Դ��ȡ����������
        Msg << "\r\n";//����Rangeͷ��PLAY����Ҳ�ǺϷ��ġ�����ý������ͷ��ʼ���ţ�ֱ��ý��������ͣ��
        //���ý����ͨ��PAUSE��ͣ��ý�������佫����ͣ�㣨the pause point�����¿�ʼ
        //���ý�������ڲ��ţ���ô����һ��PLAY���󽫲����������ã�ֻ�ǿͻ��˿����ô������Է������Ƿ���
        if (i_ptSession->eRtspState != PLAYING)
            i_ptSession->eRtspState = PLAYING;
    }


    iRet = TRUE;
	o_pstrMsg->assign(Msg.str());
    return iRet;
}

/*****************************************************************************
-Fuction		: HandleCmdPAUSE
-Description	: 
PAUSE��������ý�����������ʱ�жϡ�
�������URL��ָ���˾����ý������
��ôֻ�и�ý�����Ĳ��źͼ�¼����ͣ��halt��
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::HandleCmdPAUSE(T_Session *i_ptSession,string *i_pstrMsg,int i_iCSeq,string *o_pstrMsg)
{
    int iRet=FALSE;
	stringstream Msg("");
	
	if (i_ptSession->eRtspState != SETUP_PLAY_READY && i_ptSession->eRtspState != PLAYING) 
	{
        Msg<<RTSP_VERSION<<RTSP_RESPONSE_METHOD_NOT_VALID_455<<"\r\n";
        Msg << "CSeq: " << i_iCSeq<< "\r\n";
        Msg <<GetDateHeader()<< "\r\n";
        Msg << "\r\n";
        cout<<"HandleCmdPAUSE state err:"<<i_ptSession->eRtspState<<endl;
	}
    else
    {
        Msg<<RTSP_VERSION<<RTSP_RESPONSE_OK<<"\r\n";//Rangeͷ�ݲ�����
        Msg << "CSeq: " << i_iCSeq<< "\r\n";//PAUSE�����п��ܰ���һ��Rangeͷ����ָ����ʱý������ͣ��
        Msg <<GetDateHeader()<< "\r\n";//���ǳ����ʱ��Ϊ��ͣ�㣨pause point������ͷ�������һ����ȷ��ֵ��������һ��ʱ�䷶Χ��ý��������������ʱ�����ó���ͣ��
        Msg << "\r\n";//���Rangeͷָ����һ��ʱ�䳬�����κ�һ����ǰ�����PLAY���󣬽����ش���"457 Invalid Range" ��
        //���Rangeͷȱʧ����ô���յ���ͣ��Ϣ��ý�������������жϣ�������ͣ�����óɵ�ǰ��������ʱ�䡣

        if (i_ptSession->eRtspState != SETUP_PLAY_READY)
            i_ptSession->eRtspState = SETUP_PLAY_READY;
    }

    iRet = TRUE;
	o_pstrMsg->assign(Msg.str());

    return iRet;
}

/*****************************************************************************
-Fuction		: HandleCmdTEARDOWN
-Description	: HandleCmdTEARDOWN
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::HandleCmdTEARDOWN(T_Session *i_ptSession,string *i_pstrMsg,int i_iCSeq,string *o_pstrMsg)
{
    int iRet=FALSE;
	stringstream Msg("");

    if(NULL !=i_ptSession->pVideoRtpSession)
        i_ptSession->pVideoRtpSession->Close();
    if(NULL !=i_ptSession->pAudioRtpSession)
        i_ptSession->pAudioRtpSession->Close();
    
    if(NULL !=i_ptSession->pVideoRtpSession)
        delete i_ptSession->pVideoRtpSession;
    if(NULL != i_ptSession->pAudioRtpSession)
        delete i_ptSession->pAudioRtpSession;
        
    i_ptSession->eRtspState=INIT;

	
	Msg<<RTSP_VERSION<<RTSP_RESPONSE_OK<<"\r\n";
	Msg << "CSeq: " << i_iCSeq<< "\r\n";
    Msg <<GetDateHeader()<< "\r\n";
	Msg << "\r\n";

    cout<<"Recv CmdTEARDOWN"<<endl;
    iRet = FALSE;//TEARDOWN ��ɾ����rtsp�Ự
	o_pstrMsg->assign(Msg.str());

    return iRet;
}


/*****************************************************************************
-Fuction		: RtspStreamHandle
-Description	: rtsp��ý�����ݴ�������
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::RtspStreamHandle(T_Session *i_ptSession)
{
    int iRet=FALSE;
    list<T_Session> ::iterator Iter;

    if(NULL==m_pVideoFile)
    {
    }
    else
    {
        unsigned char * pbVideoBuf=(unsigned char * )malloc(VIDEO_BUFFER_MAX_SIZE);
        int iVideoBufLen=0;
        
        if(NULL == pbVideoBuf)
        {
            cout<<"pbVideoBuf malloc NULL"<<endl;
        }
        else
        {
            memset(pbVideoBuf,0,VIDEO_BUFFER_MAX_SIZE);
            iRet=GetNextVideoFrame(pbVideoBuf,&iVideoBufLen,VIDEO_BUFFER_MAX_SIZE);
            if(FALSE == iRet)
            {
            }
            else
            {
                for(Iter=m_SessionList.begin();Iter!=m_SessionList.end();Iter++)
                {//ͬ������Ƶ����Ҫ�������������Ŀͻ��ˣ���Ȼ�ͻ�����Ƶ���ݽ�������
                    PushVideoStream(&*Iter,pbVideoBuf,iVideoBufLen);
                }
            }
            free(pbVideoBuf);
        }
    }
    if(NULL==m_pAudioFile)
    {
    }
    else
    {
        unsigned char * pbAudioBuf=(unsigned char * )malloc(AUDIO_BUFFER_MAX_SIZE);
        int iAudioBufLen=0;
        if(NULL == pbAudioBuf)
        {
            cout<<"pbAudioBuf malloc NULL"<<endl;
        }
        else
        {
            memset(pbAudioBuf,0,AUDIO_BUFFER_MAX_SIZE);
            iRet=GetNextAudioFrame(pbAudioBuf,&iAudioBufLen,AUDIO_BUFFER_MAX_SIZE);
            if(FALSE == iRet)
            {
            }
            else
            {
                for(Iter=m_SessionList.begin();Iter!=m_SessionList.end();Iter++)
                {//ͬ������Ƶ����Ҫ�������������Ŀͻ��ˣ���Ȼ�ͻ�����Ƶ���ݽ�������
                    PushAudioStream(&*Iter,pbAudioBuf,iAudioBufLen);
                }
            }
            free(pbAudioBuf);
        }
    }
    return iRet;
}





















/*****************************************************************************
-Fuction		: GenerateSDP
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::GenerateSDP(T_Session *i_ptSession,string *o_pstrSDP)
{
    int iRet=FALSE;
    char *pcSdpBuf=new char[2000];
    string strMediaSDP("");
    string strDescriptionSDP(RTSP_SERVER_NAME);
    
    memset(pcSdpBuf,0,2000);
    strDescriptionSDP.append(RTSP_SERVER_VERSION);

    if(NULL == o_pstrSDP)
    {
        cout<<"GenerateSDP err o_pstrSDP NULL"<<endl;
    }
    else
    {
        //Generate the media SDP
        iRet=GenerateMediaSDP(i_ptSession,&strMediaSDP);
        if(FALSE== iRet)
        {
            cout<<"GenerateMediaSDP err"<<endl;
        }
        else
        {
            //Generate the Session SDP
            const char * strSdpPrefixFmt =
                 "v=0\r\n"                   //V=0     ;Version ������SDPЭ��İ汾
                 "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,�����˻Ự�ķ�������Ϣ
                 "s=%s\r\n"                 //s=<sessionname> ;������Session Name
                 "i=%s\r\n"                 //i=<sessiondescription> ; Information ����Session��һЩ��Ϣ
                 "t=0 0\r\n"                //t=<start time><stop time> ;Time
                 "a=tool:%s%s\r\n"          //a=<attribute>     ; Attribute
                 "a=type:broadcast\r\n"     //a=<attribute>:<value>
                 "a=control:*\r\n"          //"a=control:%s\r\n", uri ? uri : "*"
                 "a=range:npt=0-\r\n"       // a=range: line duration
                 "a=x-qt-text-nam:%s\r\n"
                 "a=x-qt-text-inf:%s\r\n"
                 "%s";                      // Then, add the (media-Sdp)
            snprintf(pcSdpBuf,2000,strSdpPrefixFmt,
                  i_ptSession->tCreateTime.tv_sec, i_ptSession->tCreateTime.tv_usec, // o= <session id>
                  1,                                                        // o= <version> // (needs to change if params are modified)
                  m_strIP.c_str(),//Tools::Instance()->GetLocalIP(),                                             // o= <address>
                  strDescriptionSDP.c_str(),                                // s= <description>
                  strDescriptionSDP.c_str(),                                // i= <info>
                  RTSP_SERVER_NAME, RTSP_SERVER_VERSION,                    // a=tool:
                  strDescriptionSDP.c_str(),                                // a=x-qt-text-nam: line
                  strDescriptionSDP.c_str(),                                // a=x-qt-text-inf: line
                  strMediaSDP.c_str());
                  
            if(strlen(pcSdpBuf)!=0)
            {
                o_pstrSDP->assign(pcSdpBuf);
                iRet=TRUE;
            }
        }
        
    }
    delete [] pcSdpBuf;
    return iRet;
}


/*****************************************************************************
-Fuction		: GenerateMediaSDP
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::GenerateMediaSDP(T_Session *i_ptSession,string *o_pstrMediaSDP)
{
    int iRet=FALSE;
    unsigned short wPortNumForSDP=0;
    unsigned char ucRtpPayloadType=0;
    char *pstrRtpPayloadFormatName=NULL;
    unsigned int dwRtpTimestampFrequency=0;
    
    char *pcAuxSdpBuf=new char[300];
    char *pcMediaSdpBuf=new char[1000];
    char *pcMediaSdq=NULL;
    
    memset(pcAuxSdpBuf,0,300);
    memset(pcMediaSdpBuf,0,1000);
    if(NULL == o_pstrMediaSDP)
    {
        cout<<"GenerateMediaSDP err o_pstrMediaSDP NULL"<<endl;
    }
    else
    {
        //Generate the media SDP
        const char * strMediaSdpFmt =
             "m=%s %u RTP/AVP %d\r\n"  // m= <media><port>
             "c=IN IP4 0.0.0.0\r\n"
             "b=AS:%u\r\n"             // b=AS:<bandwidth>
             "a=rtpmap:%d %s/%d%s\r\n"  // a=rtpmap:... (if present) rtpmapLine
             "a=range:npt=0-\r\n"      // a=range:... (if present)
             "%s"                      //���������������� "a=fmtp" ��
             "a=control:track%d\r\n";  //"a=control:track1"ָ���˷��ʸ���ý��ķ�ʽ���Ǻ���SETUP�������Ҫ����
        if(m_pVideoFile!=NULL)
        {//�����ϱ�����ȷָ��PTֵ�ĸ������ͣ�����Щ�����������ڵ����Ľ���û�о����PTֵ��
            ucRtpPayloadType =RTP_PAYLOAD_H264;//ֻ��ʹ�ö�̬��dynamic��PTֵ����96��127�������Ϊʲô����ձ�ָ��H264��PTֵΪ96��
            pstrRtpPayloadFormatName =(char *)VIDEO_ENCODE_FORMAT_NAME;
            dwRtpTimestampFrequency =H264_TIMESTAMP_FREQUENCY;

            // begin Generate a new "a=fmtp:" line each time, using our SPS and PPS (if we have them),
            unsigned char abSPS_WEB[SPS_PPS_BUF_MAX_LEN]={0};// "WEB" means "Without Emulation Bytes"
            int iSPS_WEB_Len= RemoveH264EmulationBytes(abSPS_WEB, m_iSPS_Len, m_abSPS, m_iSPS_Len);
            if (iSPS_WEB_Len < 4) 
            { // Bad SPS size => assume our source isn't ready
                cout<<"Bad SPS size:"<<iSPS_WEB_Len<<endl;
            }
            else
            {
                unsigned int dwProfileLevelId = (abSPS_WEB[1]<<16) | (abSPS_WEB[2]<<8) | abSPS_WEB[3];
                char * strSPS_Base64 = base64Encode((char*)m_abSPS, m_iSPS_Len);
                char * strPPS_Base64 = base64Encode((char*)m_abPPS, m_iPPS_Len);
                const char * strAuxSdpFmt =
                     "a=fmtp:%d packetization-mode=1"//��ʾ֧�ֵķ��ģʽ.�� packetization-mode ��ֵΪ 1 ʱ����ʹ�÷ǽ���(non-interleaved)���ģʽ.
                     ";profile-level-id=%06X"   //�����������ָʾ H.264 ���� profile ���ͺͼ���. �� Base16(ʮ������) ��ʾ�� 3 ���ֽ�. ��һ���ֽڱ�ʾ H.264 �� Profile ����, �������ֽڱ�ʾ H.264 �� Profile ����:
                     ";sprop-parameter-sets=%s,%s\r\n";//����H264��SPS��PPS��Base64����         
                snprintf(pcAuxSdpBuf,300,strAuxSdpFmt,
                      ucRtpPayloadType,        //sps_pps��Ҫ��h264�ļ�(Դ)�л�ȡ,����������������һ������(gop)
                      dwProfileLevelId,        //��Ӧ����һ����������ж�Ӧ���ж��,Ҳ����˵������Ƶ���Ĺ������������ǲ��ϸ��µģ����б任�˾Ͳ�ͬ��
                      strSPS_Base64,           //������Ҫ�и�һֱ��ִ�еĺ���ȥ����������������,���Ǵ���ֻ��Ҫ�õ���һ��sps��pps
                      strPPS_Base64);          //����H264��SPS��PPS��Base64����         
                delete[] strSPS_Base64;
                delete[] strPPS_Base64;
                // end Generate a new "a=fmtp:" 
                
                snprintf(pcMediaSdpBuf,1000,strMediaSdpFmt,
                      "video",              // m= <media>
                      wPortNumForSDP,       // m= <port>
                      ucRtpPayloadType,     // m= <fmt list>
                      m_dwBandwidth,        // b=AS:<bandwidth>// If bandwidth is specified, use it and add 5% for RTCP overhead. Otherwise make a guess at 500 kbps.
                      ucRtpPayloadType,     // a=rtpmap:... (if present):
                      pstrRtpPayloadFormatName,// rtpPayloadType�������� rtpPayloadFormatName�������� TimestampFrequencyʱ��Ƶ��encodingParamsPart
                      dwRtpTimestampFrequency,
                      "",                   //encodingParamsPart
                      pcAuxSdpBuf,          // optional extra SDP line
                      2*i_ptSession->iTrackNumber+1); // a=control:<track-id> //track-idΨһ���ɣ�����Ҫ����������ʹ��2n+1�ļ��㷽ʽ
                      //��Ƶ��Ӧһ��rtp�Ự����Ƶ��Ӧһ��rtp�Ự,�����Ự�໥������һ��rtp�Ự��Ӧһ��TrackId
                      //һ��rtsp����Ự���԰�������rtp�Ự��
            }
        }
        
        pcMediaSdq = pcMediaSdpBuf+strlen(pcMediaSdpBuf);
        if(m_pAudioFile!=NULL)
        {
            ucRtpPayloadType =RTP_PAYLOAD_G711;//https://tools.ietf.org/html/rfc3551#page-32
            pstrRtpPayloadFormatName =(char *)AUDIO_ENCODE_FORMAT_NAME;
            dwRtpTimestampFrequency =AUDIO_TIMESTAMP_FREQUENCY;
            snprintf(pcMediaSdq,1000-strlen(pcMediaSdpBuf),strMediaSdpFmt,
                  "audio",              // m= <media>
                  wPortNumForSDP,       // m= <port>
                  ucRtpPayloadType,     // m= <fmt list>
                  m_dwBandwidth,        // b=AS:<bandwidth>// If bandwidth is specified, use it and add 5% for RTCP overhead. Otherwise make a guess at 500 kbps.
                  ucRtpPayloadType,     // a=rtpmap:... (if present):
                  pstrRtpPayloadFormatName,// rtpPayloadType�������� rtpPayloadFormatName�������� TimestampFrequencyʱ��Ƶ��encodingParamsPart
                  dwRtpTimestampFrequency,
                  NUM_CHANNELS,         //encodingParamsPart
                  "",                   // optional extra SDP line
                  2*i_ptSession->iTrackNumber+2); // a=control:<track-id> //track-idΨһ���ɣ�����Ҫ����������ʹ��2n+2�ļ��㷽ʽ
                  //��Ƶ��Ӧһ��rtp�Ự����Ƶ��Ӧһ��rtp�Ự,�����Ự�໥������һ��rtp�Ự��Ӧһ��TrackId
                  //һ��rtsp����Ự���԰�������rtp�Ự��
        }
        
        if(strlen(pcMediaSdpBuf)!=0)
        {
            o_pstrMediaSDP->assign(pcMediaSdpBuf);
            iRet=TRUE;
        }
        else
        {
            cout<<"GenerateMediaSDP err len=0"<<endl;
        }
    }
    delete [] pcAuxSdpBuf;
    delete [] pcMediaSdpBuf;
    return iRet;
}

/*****************************************************************************
-Fuction		: GetDateHeader
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
const char * RtspServer::GetDateHeader()
{
    static char DateBuf[200];
    time_t tTime = time(NULL);

    memset(DateBuf,0,sizeof(DateBuf));
    strftime(DateBuf, sizeof(DateBuf), "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", gmtime(&tTime));
    

    return DateBuf;
}

/*****************************************************************************
-Fuction		: PushVideoStream
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::PushVideoStream(T_Session *i_ptSession,unsigned char *i_pbVideoBuf,int i_iVideoBufLen)
{
    int iRet=FALSE;
    unsigned char * pbVideoBuf= i_pbVideoBuf;
    int iVideoBufLen = i_iVideoBufLen;
    
    unsigned char *pbNaluStartPos=NULL;
    int iNaluLen=0;
    unsigned char bNaluType=0;
    
    if(NULL == i_pbVideoBuf)
    {
        cout<<"PushVideoStream err null"<<endl;
    }
    else
    {
        //cout<<"PushVideoStream VideoBufLen:"<<iVideoBufLen<<endl;
        while (iVideoBufLen > 0) 
        {
            iRet=FindH264Nalu(pbVideoBuf, iVideoBufLen,&pbNaluStartPos, &iNaluLen,&bNaluType);
            if(FALSE== iRet || NULL == pbNaluStartPos ||0==iNaluLen)
            {
                cout<<"FindH264Nalu err:"<<iRet<<" iNaluLen:"<<iNaluLen<<" iVideoBufLen:"<<iVideoBufLen<<endl;
                break;
            }
            else
            {
                TrySetSPS_PPS(pbNaluStartPos,iNaluLen);
                
                if (i_ptSession->eRtspState!= PLAYING || NULL==i_ptSession->pVideoRtpSession)
                { //��Ƶ������ʵʱ�ģ�״̬�������������ݴ���˾ʹ����
                }
                else
                {
                    int iPacketNum=0;
                    unsigned char *ppbPacketBuf[RTP_MAX_PACKET_NUM]={0};
                    int aiEveryPacketLen[RTP_MAX_PACKET_NUM]={0};
                    int i=0;
                    for(i=0;i<RTP_MAX_PACKET_NUM;i++)
                    {
                        ppbPacketBuf[i]=(unsigned char *)malloc(RTP_MAX_PACKET_SIZE);
                        memset(ppbPacketBuf[i],0,RTP_MAX_PACKET_SIZE);
                    }
                
                    T_RtpPacketParam tRtpPacketParam={0};
                    i_ptSession->pVideoRtpSession->GetRtpPacketParam(&tRtpPacketParam);
                    iPacketNum=m_pRtpPacket->Packet(&tRtpPacketParam,pbNaluStartPos,iNaluLen,ppbPacketBuf,aiEveryPacketLen);
                    if(iPacketNum<=0)
                    {
                        cout<<"m_pRtpPacket->Packet err"<<iPacketNum<<endl;
                    }
                    else
                    {
                        i_ptSession->pVideoRtpSession->SetRtpPacketParam(&tRtpPacketParam);
                        cout<<"PacketBuf:"<<ppbPacketBuf[0][0]<<" PacketNum:"<<iPacketNum<<endl;
                        for (i = 0; i < iPacketNum; i++) 
                        {
                            iRet=i_ptSession->pVideoRtpSession->SendRtpData((char *)ppbPacketBuf[i], aiEveryPacketLen[i]);
                            if(FALSE==iRet)
                                break;
                        }
                    }
                    for(i=0;i<RTP_MAX_PACKET_NUM;i++)
                        free(ppbPacketBuf[i]);
                }

            }
            iVideoBufLen -= (pbNaluStartPos - pbVideoBuf) + iNaluLen;
            pbVideoBuf = pbNaluStartPos + iNaluLen;
        }
    }
    return iRet;
}

/*****************************************************************************
-Fuction		: VideoHandle::GetNextVideoFrame
-Description	: ��������������Ը�Ϊ���ⲿ�������Ļص�����
�������ⲿ�������Ķ����е�һ����Ա����
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::GetNextVideoFrame(unsigned char *o_pbVideoBuf,int *o_iVideoBufSize,int i_iBufMaxSize)
{
    int iRet=FALSE;
    int iVideoBufSize=0;
    unsigned char abReadDataBuf[1024]={0};
    int iReadDataLen=0;
	//int iRemainDataLen = 0;
	int iRetSize=0;
	int iNaluStartPos=0;
	if(o_pbVideoBuf==NULL ||i_iBufMaxSize<VIDEO_BUFFER_MAX_SIZE ||NULL==m_pVideoFile||NULL == o_iVideoBufSize)
	{
        cout<<"GetNextVideoFrame err:"<<i_iBufMaxSize<<"m_pVideoFile:"<<m_pVideoFile<<endl;
        iRet=FALSE;
	}
	else
	{
        while ((iRetSize = fread(abReadDataBuf + iReadDataLen, 1, sizeof(abReadDataBuf) - iReadDataLen, m_pVideoFile)) > 0) 
        {
            iNaluStartPos = 3;//�Թ���һ����ʼ��,��֤һ��ʼ��ȡ�ľ���һ֡(nalu)����
            iReadDataLen += iRetSize;
            while (iNaluStartPos < iReadDataLen - 3 && !(abReadDataBuf[iNaluStartPos] == 0 &&  abReadDataBuf[iNaluStartPos+1] == 0 && (abReadDataBuf[iNaluStartPos+2] == 1
            || (abReadDataBuf[iNaluStartPos+2] == 0 && abReadDataBuf[iNaluStartPos+3] == 1)))) 
            {
                iNaluStartPos++;
            }
            memcpy(o_pbVideoBuf + iVideoBufSize, abReadDataBuf, iNaluStartPos);
            iVideoBufSize +=iNaluStartPos;
            
            memmove(abReadDataBuf, abReadDataBuf +iNaluStartPos, iReadDataLen - iNaluStartPos);//������ȥnalu֮������ݣ����������3��ʾ��ȡ��һ��nalu���˳�
            iReadDataLen -= iNaluStartPos;//���ǰ��memcpy�����Ĳ���һ��nalu����memmove��Ч���ۺ�����memmove��������ȥ��
            if (iReadDataLen >3)//ע��!!!,����ǡ�ɶ�����һ��nalu+3����ʼ����������ʹ��>=������> ,����ʹ��>=�ᵼ��ÿ��iVideoBufSize=1021���˳�
            {//???,����ǡ��ÿ�ζ�����һ��nalu+С��3���Ŀ�ʼ������,��ʱ��һֱ���������˳�ѭ����
            //��������Ӧ��ʹ��iReadDataLen>=0,ͬʱ�����޸�����߼��������Ͽ�ʼ��ͽ�����(��һ���Ŀ�ʼ��)һ���ж�(�����迪ʼ�ͽ����������)
                fseek(m_pVideoFile, -iReadDataLen, SEEK_CUR);//�ļ�ָ��ӵ�ǰλ����ǰ�ƶ����ȡ�Ĳ���,�����nalu�Ĳ���
                break;
            }
        }
        if(iVideoBufSize>i_iBufMaxSize)
        {//�����ݴ���
            cout<<"GetNextVideoFrame too long err:"<<iVideoBufSize<<endl;
        }
        else
        {
            if(iRetSize<=0)
            {
                cout<<"fread err:"<<iRetSize<<endl;
                fseek(m_pVideoFile, 0, SEEK_SET);
            }
            else
            {
                *o_iVideoBufSize = iVideoBufSize;
                iRet=TRUE;
            }
        }
	}
	return iRet;
}

/*****************************************************************************
-Fuction		: VideoHandle::FindH264Nalu
-Description	: ��������������Ը�Ϊ���ⲿ�������Ļص�����
�������ⲿ�������Ķ����е�һ����Ա����
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::FindH264Nalu(unsigned char *i_pbVideoBuf,int i_iVideoBufLen,unsigned char **o_ppbNaluStartPos,int *o_iNaluLen,unsigned char *o_bNaluType)
{
    int iRet=FALSE;

    if(NULL == i_pbVideoBuf || NULL == o_ppbNaluStartPos ||NULL ==o_iNaluLen || NULL ==  o_bNaluType)
    {
        cout<<"FindH264Nalu NULL"<<endl;
    }
    else
    {
    	unsigned char *pbNaluStartPos=NULL;
    	unsigned char *pbVideoBuf=i_pbVideoBuf;
    	int iVideoBufLen=i_iVideoBufLen;
    	while (iVideoBufLen >= 3) 
    	{
    		if (pbVideoBuf[0] == 0 && pbVideoBuf[1] == 0 && pbVideoBuf[2] == 1) 
    		{
    			if (!pbNaluStartPos) 
    			{//һ��nalu�Ŀ�ʼ��
    				if (iVideoBufLen < 4)
    				{
                        iRet=FALSE;//������������ڳ�ʼ������ʵ����ȥ��
                        pbNaluStartPos=NULL;
                        break;//���ؽ��
    				}
    				else
    				{
                        pbNaluStartPos = pbVideoBuf;
                        *o_bNaluType = pbVideoBuf[3] & 0x1f;
    				}
    			} 
    			else 
    			{//��һ��nalu�Ŀ�ʼ�룬��ǰһ��nalu�Ľ����룬Ҳ�����ҵ�һ����nalu��
    				*o_iNaluLen = (pbVideoBuf - pbNaluStartPos);
    				iRet=TRUE;
                    break;//���ؽ��
    			}
    			pbVideoBuf += 3;
    			iVideoBufLen  -= 3;
    			continue;
    		}
    		if (iVideoBufLen >= 4 && pbVideoBuf[0] == 0 && pbVideoBuf[1] == 0 && pbVideoBuf[2] == 0 && pbVideoBuf[3] == 1) 
    		{
    			if (!pbNaluStartPos) 
    			{//һ��nalu�Ŀ�ʼ��
    				if (iVideoBufLen < 5)
                    {
                        iRet=FALSE;//������������ڳ�ʼ������ʵ����ȥ��
                        pbNaluStartPos=NULL;
                        break;//���ؽ��
                    }
                    else
                    {
                        pbNaluStartPos= pbVideoBuf;
                        *o_bNaluType = pbVideoBuf[4] & 0x1f;
                    }
    			} 
    			else 
    			{//��һ��nalu�Ŀ�ʼ�룬��ǰһ��nalu�Ľ����룬Ҳ�����ҵ�һ����nalu��
    				*o_iNaluLen = (pbVideoBuf - pbNaluStartPos);
    				iRet=TRUE;
                    break;//���ؽ��
    			}
    			pbVideoBuf += 4;
    			iVideoBufLen  -= 4;
    			continue;
    		}
    		pbVideoBuf ++;
    		iVideoBufLen --;
    	}
    	if(NULL != pbNaluStartPos && FALSE==iRet)
    	{
            *o_iNaluLen = (pbVideoBuf - pbNaluStartPos + iVideoBufLen);//����buf���һ��nalu�����
            *o_ppbNaluStartPos=pbNaluStartPos;
            iRet=TRUE;
    	}
    }
	return iRet;
}

/*****************************************************************************
-Fuction		: VideoHandle::TrySetSpsPps
-Description	: ��������������Ը�Ϊ���ⲿ�������Ļص�����
�������ⲿ�������Ķ����е�һ����Ա����
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::TrySetSPS_PPS(unsigned char *i_pbNaluBuf,int i_iNaluLen)
{
    int iRet=FALSE;
	unsigned char bNaluType = 0;
	
	if (m_iSPS_Len > 0 && m_iPPS_Len > 0) 
	{//ֻ��Ҫ��һ��SPS��PPS
        iRet=TRUE;
	}
	else
	{
        if (i_pbNaluBuf[0] == 0 && i_pbNaluBuf[1] == 0 && i_pbNaluBuf[2] == 1) {
            bNaluType = i_pbNaluBuf[3] & 0x1f;
            i_pbNaluBuf += 3;
            i_iNaluLen   -= 3;
        }
        if (i_pbNaluBuf[0] == 0 && i_pbNaluBuf[1] == 0 && i_pbNaluBuf[2] == 0 && i_pbNaluBuf[3] == 1) {
            bNaluType = i_pbNaluBuf[4] & 0x1f;
            i_pbNaluBuf += 4;
            i_iNaluLen   -= 4;
        }
        if (i_iNaluLen < 1)
        {
            iRet=FALSE;
        }
        else
        {
            if (bNaluType == 7 && 0 == m_iSPS_Len) 
            {
                cout<<"SPS Len:"<<i_iNaluLen<<endl;
                if ((unsigned int)i_iNaluLen > sizeof(m_abSPS))
                    i_iNaluLen = sizeof(m_abSPS);
                    
                memcpy(m_abSPS, i_pbNaluBuf, i_iNaluLen);
                m_iSPS_Len = i_iNaluLen;
            }
            if (bNaluType == 8 && 0 == m_iPPS_Len) 
            {
                cout<<"PPS Len:"<<i_iNaluLen<<endl;
                if ((unsigned int)i_iNaluLen > sizeof(m_abPPS))
                    i_iNaluLen = sizeof(m_abPPS);
                    
                memcpy(m_abPPS, i_pbNaluBuf, i_iNaluLen);
                m_iPPS_Len = i_iNaluLen;
            }
            iRet=TRUE;
        }
	}
	return iRet;
}

/*****************************************************************************
-Fuction		: VideoHandle::RemoveH264EmulationBytes
-Description	: ��������������Ը�Ϊ���ⲿ�������Ļص�����
�������ⲿ�������Ķ����е�һ����Ա����
ȥ��h264�з�ֹ�������ֽڣ��ѿǲ�����
-Input			: i_pbNaluBuf i_iNaluLen i_iMaxNaluBufLen
-Output 		: o_pbNaluBuf
-Return 		: iNaluLen //�����ѿǲ�����ĳ���
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::RemoveH264EmulationBytes(unsigned char *o_pbNaluBuf,int i_iMaxNaluBufLen,unsigned char *i_pbNaluBuf,int i_iNaluLen)
{
    int iNaluLen=0;
    
    int i = 0;
    while (i < i_iNaluLen && iNaluLen+1 < i_iMaxNaluBufLen) 
    {
      if (i+2 < i_iNaluLen && i_pbNaluBuf[i] == 0 && i_pbNaluBuf[i+1] == 0 && i_pbNaluBuf[i+2] == 3) 
      {
        o_pbNaluBuf[iNaluLen] = o_pbNaluBuf[iNaluLen+1] = 0;
        iNaluLen += 2;
        i += 3;
      } 
      else 
      {
        o_pbNaluBuf[iNaluLen] = i_pbNaluBuf[i];
        iNaluLen += 1;
        i += 1;
      }
    }
    
    return iNaluLen;
}

/*****************************************************************************
-Fuction		: PushVideoStream
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::PushAudioStream(T_Session *i_ptSession,unsigned char *i_pbAudioBuf,int i_AudioBufLen)
{
    int iRet=FALSE;
    unsigned char * pbAudioBuf= i_pbAudioBuf;
    int iAudioBufLen = i_AudioBufLen;
    if(NULL == i_pbAudioBuf || NULL==i_ptSession->pAudioRtpSession)
    {
        cout<<"PushAudioStream err null"<<endl;
    }
    else
    {
        int iPacketNum=0;
        unsigned char *ppbPacketBuf[RTP_MAX_PACKET_NUM]={0};
        int aiEveryPacketLen[RTP_MAX_PACKET_NUM]={0};
        int i=0;
        for(i=0;i<RTP_MAX_PACKET_NUM;i++)
        {
            ppbPacketBuf[i]=(unsigned char *)malloc(RTP_MAX_PACKET_SIZE);
            memset(ppbPacketBuf[i],0,RTP_MAX_PACKET_SIZE);
        }
        
        T_RtpPacketParam tRtpPacketParam={0};
        i_ptSession->pAudioRtpSession->GetRtpPacketParam(&tRtpPacketParam);
        iPacketNum=m_pRtpPacket->Packet(&tRtpPacketParam,pbAudioBuf,iAudioBufLen,ppbPacketBuf,aiEveryPacketLen,1);
        if(iPacketNum<=0)
        {
            cout<<"m_pRtpPacket->Packet err"<<iPacketNum<<endl;
        }
        else
        {
            i_ptSession->pAudioRtpSession->SetRtpPacketParam(&tRtpPacketParam);
            if (i_ptSession->eRtspState!= PLAYING || NULL==i_ptSession->pAudioRtpSession)
            { //��Ƶ������ʵʱ�ģ�״̬�������������ݴ���˾ʹ����
            }
            else
            {
                for (i = 0; i < iPacketNum; i++) 
                {
                    iRet=i_ptSession->pAudioRtpSession->SendRtpData((char *)ppbPacketBuf[i], aiEveryPacketLen[i]);
                    if(FALSE==iRet)
                        break;
                }
            }
        }
        for(i=0;i<RTP_MAX_PACKET_NUM;i++)
            free(ppbPacketBuf[i]);
    }
    return iRet;
}

/*****************************************************************************
-Fuction		: AudioHandle::GetNextAudioFrame
-Description	: ��������������Ը�Ϊ���ⲿ�������Ļص�����
�������ⲿ�������Ķ����е�һ����Ա����
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtspServer::GetNextAudioFrame(unsigned char *o_pbAudioBuf,int *o_iAudioBufSize,int i_iBufMaxSize)
{
    int iRet=FALSE;
	int iRetSize=0;
	if(o_pbAudioBuf==NULL ||i_iBufMaxSize<AUDIO_BUFFER_MAX_SIZE ||NULL==m_pAudioFile||NULL == o_iAudioBufSize)
	{
        cout<<"GetNextAudioFrame err: "<<i_iBufMaxSize<<"m_pVideoFile:"<<m_pAudioFile<<endl;
        iRet=FALSE;
	}
	else
	{
        iRetSize = fread(o_pbAudioBuf, 1, AUDIO_BUFFER_MAX_SIZE, m_pAudioFile);
        if (iRetSize > 0) 
        {
            *o_iAudioBufSize = iRetSize;
            iRet=TRUE;
        }
	
	}
	return iRet;
}

/*****************************************************************************
-Fuction		: SessionHandleThread
-Description	: SessionHandleThread
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
void * RtspServer::SessionHandleThread(void *pArg)
{
	if( NULL != pArg )
	{
		RtspServer *pRtspServer = ( RtspServer * )pArg;
		pRtspServer->SessionHandle();
	}

	return NULL;
}



















