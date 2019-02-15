/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       : 	Main.cpp
* Description       : 	RtspServer Demo

���˼·:
����ƵԴ��socket����������Ϊrtspserver�Ĳ���
�����ĻỰ�ȴ��̵߳õ��µ�socket����newһ��rtspserver
Ȼ���new���������rtspserver������socket���� Ҳ���ǻỰ���浽������
ͬʱ��һ���̲߳���ȥ��������Ự���С�
����,rtsp������Գ�������ŵ�������е����С�

�����ƣ�
1.ʹ��vlc��Ϊ�ͻ����޷����ţ�ֻ��ʹ�ö�Ӧ�Ŀͻ���
2.����URLĿǰ����RTSP_SERVER_URL�궨��д���ģ�����Ҫ��Ϊ������
�����Ż�Ϊ���ô�ip��Ĭ�϶˿ڵķ�ʽ��

* Created           : 	2017.11.21.
* Author            : 	Yu Weifeng
* Function List     : 	
* Last Modified     : 	
* History           : 	
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/11/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "Definition.h"
#include "RtspServer.h"

using std::cout;
using std::endl;


#define MAX_CHANNEL     16
#define RTSP_SERVER_URL "rtsp://192.168.4.199:8554/1"

static void PrintUsage(char *i_strProcName);

/*****************************************************************************
-Fuction        : main
-Description    : main
-Input          : 
-Output         : 
-Return         : 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/11/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int main(int argc,char **argv)
{
    int iRet = FALSE;
    int i = 0;
    FILE * apVideoFile[MAX_CHANNEL] = {NULL};//VideoHandle *apVideoHandle[MAX_CHANNEL]
    FILE * apAudioFile[MAX_CHANNEL] = {NULL};//AudioHandle *apAudioHandle[MAX_CHANNEL]
    RtspServer * aRtspServer[MAX_CHANNEL] = {NULL}; 
    
    if (argc>MAX_CHANNEL+1 ||argc <2 /*||argc%2!=1*/) //��������������ݴ���
    {
        PrintUsage(argv[0]);
    }
    else
    {
        iRet = TRUE;
        cout<<"Rtsp server url:"<<RTSP_SERVER_URL<<endl;
        for(i=0;i<argc/2;i++)
        {
            apVideoFile[i] = fopen(argv[i*2+1],"rb");//apVideoHandle[i]=new VideoHandle();
            if(NULL == apVideoFile[i])//if(FALSE==apVideoHandle[i]->Init(argv[i*2+1]))
            {
                cout<<"Open "<<argv[i*2+1]<<"failed !"<<endl;
                iRet = FALSE;
                break;
            }
            if((i*2+1+1)<argc)
            {//����û����Ƶ�ļ������
                apAudioFile[i] = fopen(argv[i*2+1+1],"rb");//apAudioHandle[i]=new AudioHandle();
                if(NULL == apVideoFile[i])//if(FALSE==apAudioHandle[i]->Init(argv[i*2+1+1]))
                {
                    cout<<"Open "<<argv[i*2+1+1]<<"failed !"<<endl;
                    iRet = FALSE;
                    break;
                }                
            }
        }
        if(FALSE == iRet)
        {
            PrintUsage(argv[0]);
        }
        else
        {
            for(i=0;i<MAX_CHANNEL;i++)
            {
                if(apVideoFile[i]!=NULL || apAudioFile[i]!=NULL)//if(apVideoHandle[i]!=NULL || apAudioHandle[i]!=NULL)
                {
                    aRtspServer[i] = new RtspServer(apVideoFile[i],apAudioFile[i]);//�����Ż�Ϊ�����ȡ����Ƶ�ĺ������߶���
                }//aRtspServer[i] = new RtspServer(apVideoHandle[i],apAudioHandle[i]);
            }

            while(1)
            {
                for(i=0;i<MAX_CHANNEL;i++)
                {
                    if(aRtspServer[i]!=NULL)
                    {
                        aRtspServer[i]->ConnectHandle((char *)RTSP_SERVER_URL);//��������������ڲ��ᱣ��Ự������,�����̹߳���ö���
                    }
                }
            }
        }    
    }
    //process exit handle
    for(i=0;i<MAX_CHANNEL;i++)
    {
        if(apVideoFile[i]!=NULL)//if(apVideoHandle[i]!=NULL)
        {
            fclose(apVideoFile[i]);//delete apVideoHandle[i];
        }
        if(apAudioFile[i]!=NULL)//if(apAudioHandle[i]!=NULL)
        {
            fclose(apVideoFile[i]);//delete apAudioHandle[i];
        }
        if(aRtspServer[i]!=NULL)
        {
            delete aRtspServer[i];
        }
    }
    return 0;
}

/*****************************************************************************
-Fuction        : PrintUsage
-Description    : PrintUsage
-Input          : 
-Output         : 
-Return         : 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/11/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
static void PrintUsage(char *i_strProcName)
{
    cout<<"Usage: "<<i_strProcName<<" <H264FILE> "<<endl;
    cout<<"Usage: "<<i_strProcName<<" <H264FILE> <G711AFILE>"<<endl;
    cout<<"Usage: "<<i_strProcName<<" <H264FILE> <G711AFILE> <H264FILE>"<<endl;
    cout<<"Usage: "<<i_strProcName<<" <H264FILE> <G711AFILE> <H264FILE> <G711AFILE> ... ..."<<endl;
}

