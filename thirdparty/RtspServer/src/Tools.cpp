/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	Tools.cpp
* Description		: 	Tools operation center:�ڲ�ȫ��һЩ���ߺ���
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "Tools.h"

#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <err.h>

#include "Definition.h"
#include <iostream>

using std::cout;//��Ҫ<iostream>
using std::endl;

Tools * Tools::m_pInstance = new Tools();//һ��ʹ�ö���ģʽ,����ģʽ�̲߳���ȫ
/*****************************************************************************
-Fuction		: Tools
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Tools::Tools()
{

}

/*****************************************************************************
-Fuction		: ~Tools
-Description	: ~Tools
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Tools::~Tools()
{

}

/*****************************************************************************
-Fuction		: Instance
-Description	: Instance
-Input			:
-Output 		:
-Return 		:
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/26	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Tools * Tools::Instance()
{
	return m_pInstance;
}

/*****************************************************************************
-Fuction		: Regex
-Description	: ������ʽ
.��				ƥ�����\r\n��֮����κε����ַ�
*				ƥ��ǰ����ӱ��ʽ����Ρ����磬zo*��ƥ�䡰z����Ҳ��ƥ�䡰zo���Լ���zoo����*�ȼ���o{0,}
				����.*��ƥ��������洢�����������
(pattern)		ƥ��ģʽ��pattern����ȡ��һƥ�䡣����ȡ��ƥ����ԴӲ�����Matches���ϵõ�
[xyz]			�ַ����ϡ�ƥ��������������һ���ַ������磬��[abc]������ƥ�䡰plain���еġ�a����
+				ƥ��ǰ����ӱ��ʽһ�λ���(���ڵ���1�Σ������磬��zo+����ƥ�䡰zo���Լ���zoo����������ƥ�䡰z����+�ȼ���{1,}��
				//���������в���+��Ĭ����һ�Σ���ֻ��ƥ�䵽һ������6
				
[A-Za-z0-9] 	26����д��ĸ��26��Сд��ĸ��0��9����
[A-Za-z0-9+/=]	26����д��ĸ��26��Сд��ĸ0��9�����Լ�+/= �����ַ�


-Input			: i_strPattern ģʽ��,i_strBuf��ƥ���ַ���,
-Output 		: o_ptMatch �洢ƥ�䴮λ�õ�����,���ڴ洢ƥ�����ڴ�ƥ�䴮�е��±귶Χ
//����0��Ԫ�����������ʽƥ������λ��,�������������������ƥ��������ߵĵ�Ԫ���δ����������ʽƥ������λ��
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/11/01	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Tools::Regex(const char *i_strPattern,char *i_strBuf,regmatch_t *o_ptMatch)
{
    char acErrBuf[256];
    int iRet=-1;
    regex_t tReg;    //����һ������ʵ��
    //const size_t dwMatch = 6;    //����ƥ�������������       //��ʾ������ƥ��


    //REG_ICASE ƥ����ĸʱ���Դ�Сд��
    iRet =regcomp(&tReg, i_strPattern, REG_EXTENDED);    //��������ģʽ��
    if(iRet != 0) 
    {
        regerror(iRet, &tReg, acErrBuf, sizeof(acErrBuf));
        cout<<"Regex Error:"<<acErrBuf<<endl;
    }
    else
    {
        iRet = regexec(&tReg, i_strBuf, MAX_MATCH_NUM, o_ptMatch, 0); //ƥ����
        if (iRet == REG_NOMATCH)
        { //���ûƥ����
            cout<<"No Match!"<<endl;
        }
        else if (iRet == REG_NOERROR)
        { //���ƥ������
            /*cout<<"Match"<<endl;
            int i=0,j=0;
			for(j=0;j<MAX_MATCH_NUM;j++)
			{
				for (i= o_ptMatch[j].rm_so; i < o_ptMatch[j].rm_eo; i++)
				{ //�������ƥ�䷶Χ���ַ���
					printf("%c", i_strBuf[i]);
				}
				printf("\n");
			}*/
        }
        else
        {
            cout<<"Unknow err:"<<iRet<<endl;
        }
        regfree(&tReg);  //�ͷ�������ʽ
    }
    
    return iRet;
}

/*****************************************************************************
-Fuction		: GetLocalIP
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
const char * Tools::GetLocalIP()
{
    static char strIpBuf[128];
    struct hostent *he;  
    char strHostName[40]={0} ;  
  
  
    gethostname(strHostName,sizeof(strHostName));  
    he = gethostbyname(strHostName);
    cout<<he<<"-handle-gethostbyname hostname:"<<strHostName<<endl;
    
    char **phe = NULL;  
    for( phe=he->h_addr_list ; NULL != *phe ; ++phe)  
    {  
        inet_ntop(he->h_addrtype,*phe,strIpBuf,sizeof(strIpBuf));
        cout<<"GetLocalIP addr:"<<strIpBuf<<endl;
    }  

    return strIpBuf;
}

/*****************************************************************************
-Fuction		: GetRemoteIP
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
const char * Tools::UseSocketGetIP(int i_iClientSocketFd)
{
	struct sockaddr_in inaddr;
	socklen_t addrlen = sizeof(inaddr);
	int ret = getpeername(i_iClientSocketFd, (struct sockaddr*)&inaddr, &addrlen);
	if (ret < 0) {
		err(1,"getpeername failed: %s\n", strerror(errno));
		return NULL;
	}
	return inet_ntoa(inaddr.sin_addr);
}

/*****************************************************************************
-Fuction		: GetLocalPort
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
unsigned short Tools::UseSocketGetPort(int i_iSocketFd)
{
	struct sockaddr_in inaddr;
	socklen_t addrlen = sizeof(inaddr);
	int ret = getsockname(i_iSocketFd, (struct sockaddr*)&inaddr, &addrlen);
	if (ret < 0) {
		err(1,"getsockname failed: %s\n", strerror(errno));
		cout<<"UseSocketGetPort err"<<endl;
		return 0;
	}
	return ntohs(inaddr.sin_port);
}

/*****************************************************************************
-Fuction		: GetRandom
-Description	: 
   Return a 32-bit random number.
   Because "our_random()" returns a 31-bit random number, we call it a second
   time, to generate the high bit.
   (Actually, to increase the likelhood of randomness, we take the middle 16 bits of two successive calls to "our_random()")

-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
unsigned int Tools::GetRandom()
{
    long random_1 = random();
    unsigned int random16_1 = (unsigned int)(random_1&0x00FFFF00);
    
    long random_2 = random();
    unsigned int random16_2 = (unsigned int)(random_2&0x00FFFF00);
    
    return (random16_1<<8) | (random16_2>>8);
}

/*****************************************************************************
-Fuction		: GetLocalPort
-Description	: 
-Input			: 
-Output 		: 
-Return 		: ����΢��us
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
unsigned long long Tools:: GetSysTime (void)
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);//clk_idΪCLOCK_MONOTONIC���򷵻�ϵͳ��������������������
	return (tp.tv_sec * 1000000llu + tp.tv_nsec / 1000llu);//ת��Ϊus
}









