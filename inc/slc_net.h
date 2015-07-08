#ifndef __SOLDIELXJFD__
#define __SOLDIELXJFD__
#include "slc_port.h"
#define MAX_SOC_BUFFER_SIZE 1500
#define MIN_IPADDR_STREN_LEN   7
#define SLC_MAX_RCV_BUFFER_SIZE 1500

#define MAX_SOCKET_QUEUE_LIST   (10) //最大支持10组

typedef enum{
	SLC_CONECT_SUCCEED = 0,//链接成功。
	
	SLC_RECONECT,//不成功，重连.

	SLC_CONNECT_FIALD,
	
	SLC_VRG_VAIL,//参数错误
	
	SLC_READ_DATA,//返回数据
	
	SLC_SEND_SUCCEED,//发送成功
	
	SLC_SEND_FAILD,//发送失败
	
	SLC_CLOSED,//关闭

	SLC_FAIL,//失败

}SLC_SEND_DATA_RESULT;


typedef void (*slc_set_data_cb)(SLC_SEND_DATA_RESULT result,void *data);

typedef struct _slc_soc_queue{
	
		U8 * _send_data;//准备送出的buffer 最大10组。

		U32 _data_lenth;//数据长度。

		int _send_count;//已经发送的次数，如果发送失败就得尝试3次，如果3次都失败，就放弃。
		
		slc_set_data_cb CallBack; //回调

		U32 flag;
}slc_soc_queue;




typedef struct _slc_socket_queue{
	
	slc_soc_queue	_queue[MAX_SOCKET_QUEUE_LIST];
	
	signed char _index; //当前叠加的索引。最大MAX_SOCKET_QUEUE_LIST  抛弃了。

}slc_socket_queue;

typedef struct _slc_NetStruct{

	sockaddr_struct addr2;
	
	signed Soc_hand;

	BOOL isConnect;

	U8 app_id;

	U8*  SendBuffer;

	U32 SendBuffSize;

	U8*  RecveBuffer;

	U32  RecvBuffSize;

	kal_uint32 account_id;

	slc_set_data_cb CallBack;

	slc_soc_queue *_queue;

}slc_NetStruct;
void slc_GetHostByName(char *domain_url);
void slc_release_mem_ptr(void *pointer);
void *slc_get_mem_ptr(kal_int32 size);
extern U16 *slc_get_will_send_file_full_name(void);
BOOL slc_send_data(sockaddr_struct _soc,slc_soc_queue *_ptr);
void slc_rec_soc_call_back(SLC_SEND_DATA_RESULT result,void *data);
slc_soc_queue *slc_get_font_note(void) ;
sockaddr_struct * slc_get_soc_addr(void);
#endif
