#ifdef __SLC_CUSTOM_FUN__
#include "slc_net.h"
#include "slc_rec.h"
extern U16 gRecListActive[SLC_MAX_REC_FILE_NAME_LEN];
#define REGET_HOST_IP_DELAY_TIME  (10)


slc_NetStruct gScllink = {0};
slc_socket_queue gSocQueue = {0};
char tmp_soc[]={0};
extern U16 gRecFileNameSendActive[SLC_MAX_REC_FILE_NAME_LEN];
/*
	初始化socket
*/
void slc_init_socket(void)
{
	slc_NetStruct *p = &gScllink;

	p->Soc_hand = -1;
	p->SendBuffer = NULL;
	p->RecveBuffer = NULL;
	p->RecvBuffSize = 0;
	p->SendBuffSize = 0;
	p->account_id = 0;
}


kal_uint32 slc_GetGPRSAccount(void)
{
	cbm_app_info_struct app_info;
	kal_uint8 vs_app_id;
	kal_int8 ret;

	kal_uint32 account_id;
	memset(&app_info, 0, sizeof(app_info));
	app_info.app_icon_id = 0;
	app_info.app_str_id = 0;
	app_info.app_type = DTCNT_APPTYPE_DEF | DTCNT_APPTYPE_SKIP_WIFI | DTCNT_APPTYPE_BRW_HTTP;//DTCNT_APPTYPE_MRE_WAP;
	ret = cbm_register_app_id_with_app_info(&app_info, &vs_app_id);
    account_id = cbm_encode_data_account_id(CBM_DEFAULT_ACCT_ID, CBM_SIM_ID_SIM1, vs_app_id, KAL_FALSE);
	slc_print("account_id %d ret %d",account_id,ret);
	return account_id;
}
void slc_GetHost_CallBack(void *InMsg)
{
	static count = 0;
	slc_NetStruct *p = &gScllink;
	char str_host[20]={0};
	app_soc_get_host_by_name_ind_struct* dns_ind = (app_soc_get_host_by_name_ind_struct*)InMsg;

	if(dns_ind->result != FALSE)
	{
		sprintf(str_host,"%d.%d.%d.%d",dns_ind->addr[0],dns_ind->addr[1],dns_ind->addr[2],dns_ind->addr[3]);

		mmi_frm_clear_protocol_event_handler(MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND, (PsIntFuncPtr)slc_GetHost_CallBack);
	}
	else
	{
		slc_print("get host ip faild");
		if(++count > 3)
		{
		 //获取３次还没有获取到IP,暂停。沿用上次的IP.
			slc_print("get host ip faild,MAX 3 timers");
		}
		else
		StartTimer(SLC_GET_HOST_RETRY_TIMER_ID, 1000*REGET_HOST_IP_DELAY_TIME, slc_GetHostByName);
	}
}
void slc_GetHostByName(char *domain_url)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    kal_int32 ret;
	kal_int8 handle;
    kal_uint8 option = 0, option_set = 0, option_get = 0;
	kal_uint8 addr_len = 0;
	kal_uint8 addr_buf[4];
	kal_int8 reasult = 0;
	
	slc_print("LOC get host by name %s",domain_url);
	if(gScllink.account_id == 0)
	gScllink.account_id = slc_GetGPRSAccount();

	mmi_frm_set_protocol_event_handler(MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND, (PsIntFuncPtr)slc_GetHost_CallBack, MMI_TRUE); 
    ret = soc_gethostbyname(
            KAL_FALSE,
            MOD_MMI,
            100,
            domain_url,
            addr_buf,
            &addr_len,
            0,
            gScllink.account_id);
}
void slc_socket_close(void)
{
	slc_NetStruct *p = &gScllink;

	slc_print("slc_socket_close...");
	if(p->Soc_hand >= 0)
	{
		soc_close(p->Soc_hand);
		p->Soc_hand=-1;
	}
}
static void slc_socket_read_internal(U8 *p_data,U32 lenth)
{
	slc_task_struct* task_p=NULL;
#define CMD_STRING     "SLC"
	char Head_cx[10]={0};
	char Head_id[21]={0};
	char Head_ProductName[10]={0};
	char Head_SWVer[10]={0};
	char Ser_CMD[10]={0};
	char Ser_Result[5]={0};
	char *head_cmd = NULL;
	U32 lenth_data = 0;
	//head:   SLC1{<C1#1>}	[1-0:0:54]
	if((head_cmd=strstr((char*)p_data,CMD_STRING)) == NULL)
	{
		slc_print("%s",(char *)p_data);
		slc_print("丢弃。不是命令字节。");
		//slc_release_mem_ptr(p_data);
		//p_data = NULL;
		return;
	}
	slc_print("head:   %s",head_cmd);
	//SLC1{<D49#0,AMR格式音频数据>}
	slc_scanf((char*)p_data,15,"SLC%s{<%s#",Head_id,Ser_CMD);
	if(!strcmp(Ser_CMD,"D49"))
	{
		head_cmd = strstr(p_data,"D49|0,");
		head_cmd += strlen("D49|0,");
		lenth_data = lenth - (head_cmd-p_data);//得到除SLC1{<D49|0,长度
		if((*(p_data+lenth-1)=='}')&&(*(p_data+lenth-1-1) == '>'))
		{
			*(p_data+lenth-1) = 0;
			lenth_data--;
			*(p_data+lenth-1-1) = 0;
			lenth_data--;
			//lenth_data　= lenth_data -2;
			//是录音文件下发。

			task_p = (slc_task_struct*) construct_local_para(sizeof(slc_task_struct), TD_CTRL);
			task_p->_msg= head_cmd;
			task_p->_msg_lenth = lenth_data;
			slc_msg_send(SLC_MSG_RCV_REC, task_p);
		}
		else
		{
			slc_print("录音文件下发不完整。没有结束符。");
		}

		
	}
	else
	{
		//在转发消息之后释放，还是在这里释放?
		//slc_release_mem_ptr((void *)p_data);
		//p_data = NULL;
	}
}
//char recv_buff[1024*20]={0};
static void slc_Notify_Internal(void* Content)
{
	char soc_tmp[25][30]={"","SOC_READ","SOC_WRITE","","SOC_ACCEPT","","","","SOC_CONNECT","","","","","","","SOC_CLOSE"};
	int soc_index[5]={0x01,0x02,0x04,0x08,0x10};

	slc_NetStruct *p = &gScllink;
	app_soc_notify_ind_struct *soc_notify = (app_soc_notify_ind_struct *) Content;
	S32 ret = 0;
	SLC_SEND_DATA_RESULT soc_result=SLC_FAIL;

	slc_print("slc_notify:%s :%d",soc_tmp[soc_notify->event_type],soc_notify->event_type);
	switch (soc_notify->event_type)
	{
		case SOC_WRITE:
		{
			//写
		}
		break;
		case SOC_READ:
		{
			char *data_section = NULL;
			int buffer_len=0;
			//if(p->RecveBuffer == NULL)
			//{
			    //申请30K空间。
				//p->RecveBuffer = (U8*)slc_get_mem_ptr(1024*30);
				p->RecveBuffer = gSocReciveBuffer;

			//}
				memset(p->RecveBuffer,0,sizeof(gSocReciveBuffer));
			//读
			do{
				ret = soc_recv(p->Soc_hand, (void *)(p->RecveBuffer+buffer_len), SLC_MAX_RCV_BUFFER_SIZE, 0);
				//ret = soc_recv(p->Soc_hand, (void *)(recv_buff+buffer_len), SLC_MAX_RCV_BUFFER_SIZE, 0);
				if(ret >= 0)
				buffer_len +=  ret;
				
			}while(ret > 0);
			soc_result = SLC_READ_DATA;

			p->_queue->CallBack=NULL;
			p->_queue->_data_lenth = buffer_len;
			p->_queue->_send_data = p->RecveBuffer;

			slc_socket_read_internal(p->RecveBuffer,buffer_len);
			if(p->CallBack)
			p->CallBack(soc_result,(void *)p->_queue);
		}
		break;
		case SOC_CONNECT:
		{
			//连接上
			soc_result = SLC_CONECT_SUCCEED;

			p->isConnect = TRUE;
			if(p->CallBack)
			p->CallBack(soc_result,(void *)p->_queue);
			
			ret = soc_send(p->Soc_hand, (U8*) (p->SendBuffer), p->SendBuffSize, 0);
			//ret = soc_send(p->Soc_hand, (U8*) (tmp_soc), p->SendBuffSize, 0);
			slc_print("send size:%d",ret);
			if(ret > 0)
			{
				//成功
				soc_result = SLC_SEND_SUCCEED;
				if(p->CallBack)
				p->CallBack(soc_result,(void *)p->_queue);
			}
			else if(ret == SOC_WOULDBLOCK)
			{	//发送失败
				soc_result = SLC_SEND_FAILD;
				SetProtocolEventHandler(slc_Notify_Internal,MSG_ID_APP_SOC_NOTIFY_IND);

				if(p->CallBack)
				p->CallBack(soc_result,(void *)p->_queue);
			}
			else
			{
				
			}
		}
		break;
		case SOC_ACCEPT:
		{
			//接受
		}
		break;
		case SOC_CLOSE:
		{
			//关闭
			slc_print("soc closed %d",p->Soc_hand);
			if(p->Soc_hand >= 0)
			{
				soc_close(p->Soc_hand);
				p->Soc_hand=-1;
			}
			soc_result = SLC_CLOSED;

			if(p->CallBack)
			p->CallBack(soc_result,(void *)p->_queue);
		}
		default:
			break;
		break;
	}

}
void slc_socket_send_timer_out(void)
{
	slc_NetStruct *p = &gScllink;

	if((p->CallBack)&&(p->isConnect == FALSE))
	p->CallBack(SLC_CONNECT_FIALD,(void *)p->_queue);
}
/*
	_addr :ip地址。
	_data :数据。
	_callback:上层应用返回回调函数。
*/
BOOL slc_send_data(sockaddr_struct _soc,slc_soc_queue *_ptr)
{
	#define VALID_SOCKET(s) (s>=0)
	kal_uint32 acct_id = 0;
	slc_NetStruct *p = &gScllink;
	kal_int8 ret = 0;
	// 检查参数
    if ((_soc.addr[0]==0) || _soc.port <= 0 || !_ptr)
    {
    	slc_print("soc params error");
    	_ptr->CallBack(SLC_VRG_VAIL,NULL);
        return;
    }
    p->CallBack = _ptr->CallBack;
    p->SendBuffer = _ptr->_send_data;
    p->SendBuffSize = _ptr->_data_lenth;
    p->_queue = _ptr;
    if(!VALID_SOCKET(p->Soc_hand))
	{
			U8 val = 1;
			if(p->account_id == 0)
			p->account_id = slc_GetGPRSAccount();
			
			p->Soc_hand = soc_create(SOC_PF_INET, _soc.sock_type, 0, MOD_MMI, p->account_id);
			if(p->Soc_hand >= 0)
			{
				if (soc_setsockopt(p->Soc_hand, SOC_NBIO, &val, sizeof(val)) < 0)
				{
					//错误
    				_ptr->CallBack(SLC_VRG_VAIL,(void *)p->_queue);
        			return;
				}
				val = SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT;
				if (soc_setsockopt(p->Soc_hand, SOC_ASYNC, &val, sizeof(val)) < 0)
				{
					//错误
    				_ptr->CallBack(SLC_VRG_VAIL,(void *)p->_queue);
        			return;
				}
				if(_soc.sock_type == SOC_SOCK_STREAM)
				{
					//tcp
					slc_print("soc_connect:hand %d,IP[%d.%d.%d.%d:%d]",p->Soc_hand,_soc.addr[0],_soc.addr[1],_soc.addr[2],_soc.addr[3],_soc.port);
					ret = soc_connect(p->Soc_hand, &_soc);
					p->isConnect = FALSE;
					
				}
				else
				{
					//udp
					//直接发送
					/*
					slc_print("direct send data 1");
					ret = soc_sendto(p->Soc_hand, (U8*) p->SendBuffer, sizeof(p->SendBuffer), 0);
					if(ret<0)
					{
					//错误
					}
					*/
				}
				if(ret < 0)
				{
					//slc_print("soc_connect faild! %d",ret);
					//阻塞模式，不用回调。
					//_ptr->CallBack(SLC_RECONECT,(void *)p->_queue);
					StartTimer(SLC_SOCKET_SEND_TIMER_OUT_ID,1000*30,slc_socket_send_timer_out);
					SetProtocolEventHandler(slc_Notify_Internal,MSG_ID_APP_SOC_NOTIFY_IND);
				}
			}
	}
	else
	{
		//直接发送
		ret = soc_send(p->Soc_hand, (U8*) p->SendBuffer, p->SendBuffSize, 0);
		//ret = soc_send(p->Soc_hand, (U8*) tmp_soc, sizeof(p->SendBuffer), 0);
		slc_print("direct send data size: %d buffsize:%d",ret,p->SendBuffSize);
		if(ret<0)
		{
			//错误
			if(_ptr->CallBack)
			_ptr->CallBack(SLC_SEND_FAILD,(void *)p->_queue);
		}
		else
		{
			if(_ptr->CallBack)
			_ptr->CallBack(SLC_SEND_SUCCEED,(void *)p->_queue);
		}
	}
	return TRUE;
}
/*
	从队列里取数据发送。
	
*/
void slc_queue_send_data(void)
{
    slc_soc_queue *heatPtr = NULL;

	if((heatPtr = slc_get_font_note()) != NULL)
    {
    	/*192.168.2.26*/
    	sockaddr_struct _addr;
    	_addr.addr[0] = 112;
    	_addr.addr[1] = 74;
    	_addr.addr[2] = 97;
    	_addr.addr[3] = 0;

		_addr.addr_len = 4;
		_addr.port = 9500;
		_addr.sock_type = SOC_SOCK_STREAM;

		slc_send_data(_addr,heatPtr);
    }
}
/*
	一对一，还是一对多?
*/
char *slc_get_active_point_to(void)
{
	return "0";
}
void slc_rec_soc_call_back(SLC_SEND_DATA_RESULT result,void *data)
{
	static int send_count = 0;
	slc_socket_queue *p = &gSocQueue;
	slc_soc_queue *_data = NULL;
	sockaddr_struct _addr;
	char StatuString[10][35]={"SLC_CONECT_SUCCEED","SLC_RECONECT","SLC_CONNECT_FIALD","SLC_VRG_VAIL","SLC_READ_DATA","SLC_SEND_SUCCEED","SLC_SEND_FAILD","SLC_CLOSED","SLC_FAIL"};
	slc_soc_queue *ptr = (slc_soc_queue *)data;

	
	if(result == SLC_SEND_SUCCEED)
	{
		slc_print("call back!!! result %s　",StatuString[result]);
		send_count =0;
		if(ptr->_send_data != NULL)
		{
			//slc_release_mem_ptr(ptr->_send_data);
			//ptr->_send_data = NULL;
		}
		ptr->_data_lenth = 0;
		ptr->_send_count = 0;
		#ifndef WIN32
		//先清掉录音的list.
		slc_del_filename_from_list(gRecFileNameSendActive);
		//再删掉这个文件。
		slc_delete_file_from_filepath(gRecFileNameSendActive);
		#endif
	}
	else if((result == SLC_CONNECT_FIALD)||(result ==SLC_SEND_FAILD))
	{
		slc_print("call back!!! result %s　失败次数:%d",StatuString[result],send_count);
		send_count++;
		slc_socket_close();
		if(send_count >= 3)
		{
			slc_print("连续３次都发送失败。删掉了。");
			ptr->_send_count = 0;
			ptr->_data_lenth = 0;
			send_count=0;
			slc_record_delete_all_file_and_list();
		}
		else
		{
			slc_print("失败重发第 %d 次",send_count);
			//先关闭。
			slc_send_amr_to_ser();
		}
	}
	else
	{
		slc_print("call back!!! result %s",StatuString[result]);
	}
}

/*
	组合录音数据包。
*/
void slc_set_record_packet(void)
{
#define SLC_MAX_REC_FILE_NAME_LEN_   (80)
	U16 FullNamePath[SLC_MAX_REC_FILE_NAME_LEN_] = {0};
	int v;
	slc_soc_queue _data;
    FS_HANDLE fd = 0;
    S32 fs_ret;
    S32 i = 0, result = 0;
    U32 file_size;
    U32 nRW;
    signed char _index=-1;
    U32 lenth_p = 0;
	sockaddr_struct *_paddr=NULL;
	char testPath[215]={0};

	mmi_wcscpy(FullNamePath, slc_get_will_send_file_full_name());

	mmi_wcs_to_asc(testPath, FullNamePath);
	slc_print("slc_set_record_packet:%s",testPath);
	if(mmi_wcslen(FullNamePath) != 0)
	{
		#ifdef WIN32
		fd = FS_Open(L"c:\\01000016.amr", FS_READ_ONLY);
		#else
		fd = FS_Open(FullNamePath, FS_READ_ONLY);
		#endif
		if(fd>= FS_NO_ERROR)
		{
			fs_ret = FS_GetFileSize(fd, &file_size);

			_data.CallBack = slc_rec_soc_call_back;
			_data._data_lenth = file_size;
			//_data._send_data = (U8 *)slc_get_mem_ptr(file_size+200);
			memset(gSocSendBuffer,0,sizeof(gSocSendBuffer));
			_data._send_data = gSocSendBuffer;


			strcpy(_data._send_data,(const char*)slc_get_string_cs());
			lenth_p += strlen((const char*)slc_get_string_cs());
			
			strcat(_data._send_data,(const char*)slc_get_string_imei());
			lenth_p += strlen((const char*)slc_get_string_imei());

			strcat(_data._send_data,"|");
			lenth_p += 1;

			strcat(_data._send_data,(const char*)slc_get_product_name());
			lenth_p += strlen((const char*)slc_get_product_name());

			strcat(_data._send_data,"|");
			lenth_p += 1;
			
			strcat(_data._send_data,(const char*)slc_get_release_verno());
			lenth_p += strlen((const char*)slc_get_release_verno());

			strcat(_data._send_data,"{<");
			lenth_p += 1;
			
			strcat(_data._send_data,"U49");
			lenth_p += 3;

			strcat(_data._send_data,"#");
			lenth_p += 1;

			strcat(_data._send_data,(const char*)slc_get_active_point_to());
			lenth_p += strlen((const char*)slc_get_active_point_to());
			
			strcat(_data._send_data,",");
			lenth_p += 1;


			fs_ret = FS_Seek(fd, 0, FS_FILE_BEGIN);
			fs_ret = FS_Read(fd, _data._send_data+lenth_p+1, file_size, &nRW);
			lenth_p += file_size;
			_data._data_lenth = lenth_p;

			_data._data_lenth += 1;
			*(_data._send_data+_data._data_lenth) = '>';
			_data._data_lenth += 1;
			*(_data._send_data+_data._data_lenth) = '}';

			FS_Close(fd);
			 /*
				不使用队列。
				slc_add_node_tail(_data);

				slc_queue_send_data();
			*/
			slc_print("%d:----%s",_data._data_lenth,_data);
			_paddr = slc_get_soc_addr();
			slc_send_data(*_paddr,&_data);
		}
		else
		{
			slc_print("打开文件失败。");
		}
	}
	else
	{
		slc_print("Full name path is nil!!!!!");
	}
}
#define __CUSTOM_READ_FILE__

//初始化队列
void slc_init_link_queue(void) 
{
	int v;
	slc_socket_queue *p = &gSocQueue;
	memset(p->_queue,0,sizeof(p->_queue));
	for(v=0;v<MAX_SOCKET_QUEUE_LIST;v++)
	{
		if(p->_queue[v]._send_data != NULL)
		{
			//TempUartBuff = (char *)med_alloc_ext_mem(MAX_UART_RECIVE);
			//slc_release_mem_ptr(p->_queue[v]._send_data);
			p->_queue[v]._send_data = NULL;
			p->_queue[v].CallBack = NULL;
			p->_queue[v]._data_lenth = 0;
		}
	}
	p->_index = -1;
}

//向队尾添加数据
int slc_add_node_tail(slc_soc_queue _data) 
{
	slc_socket_queue *p = &gSocQueue;
	int i,v=0;
	
	for(i=0;i<MAX_SOCKET_QUEUE_LIST;i++)
	{
		if(p->_queue[i]._send_data == NULL)
		{
			v++;
			break;
		}
	}
	if(v>0)
	{
		p->_queue[i]._send_data= _data._send_data;
		p->_queue[i]._data_lenth = _data._data_lenth;
		p->_queue[i].CallBack = _data.CallBack;
	}
	else
	return 0;

    return 1;
}
//得到队列的队头
slc_soc_queue *slc_get_font_note(void) 
{
	slc_socket_queue *p = &gSocQueue;

	
	int i;
	int v= 0;
	
	for(i=0;i<MAX_SOCKET_QUEUE_LIST;i++)
	{
		if(p->_queue[i]._send_data != NULL)
		{
			v++;
			break;
		}
	}
	if(v>0)
	{
		//有数据。

		return &p->_queue[i];
	}
	else
	return NULL;

   // return 1;
}
//删除del_index的数据
int slc_delete_font_note(int del_index) 
{
	slc_socket_queue *p = &gSocQueue;
	int i;
	signed char v=-1;

	if((del_index < MAX_SOCKET_QUEUE_LIST)&&((p->_queue[del_index]._send_data != NULL)))
	{
		//有数据。
		//slc_release_mem_ptr(p->_queue[v]._send_data);
		p->_queue[v]._send_data = NULL;
		p->_queue[v].CallBack = NULL;
		p->_queue[v]._data_lenth = 0;
	}
	else
	return 0;

	
	return 1;
}
void *slc_get_mem_ptr(kal_int32 size)
{
	return med_alloc_ext_mem(size);
}
void slc_release_mem_ptr(void *pointer)
{
	if(pointer != NULL)
	med_free_ext_mem((void * *)&pointer);
}
/*
	删掉队列之后要检查队列是否为空。不为空则需要再次发送到SOCKET
*/
void slc_delete_queue_release(slc_soc_queue *ptr)
{
	slc_socket_queue *p = &gSocQueue;
	slc_soc_queue *_data = NULL;
	sockaddr_struct *_addr;

	slc_print("slc_delete_queue_release");
	if(ptr->_send_data != NULL)
	{
		//slc_release_mem_ptr(ptr->_send_data);
		ptr->_send_data = NULL;
	}
	ptr->CallBack = NULL;
	ptr->_data_lenth = 0;
	ptr->_send_count = 0;
	if((_data = slc_get_font_note())!= NULL)
	{
		//非空。
		_addr = slc_get_soc_addr();
		slc_print("soc not empty continue:%c%c%c%c%c%c%c%c%c%c",_data[0],_data[1],_data[2],_data[3],_data[4],_data[5],_data[6],_data[7],_data[8],_data[9]);
		slc_send_data(*_addr,_data);
	}
}
/*
	获取addr ip
*/
sockaddr_struct * slc_get_soc_addr(void)
{
	sockaddr_struct _addr={0};
	//112.74.97.45 F1
	//119.139.242.203
	//59.40.117.135
	_addr.addr[0] = 112;
	_addr.addr[1] = 74;
	_addr.addr[2] = 97;
	_addr.addr[3] = 45;

	_addr.addr_len = 4;
	_addr.port = 5555;//9500;
	_addr.sock_type = SOC_SOCK_STREAM;

	return &_addr;
}
#endif
