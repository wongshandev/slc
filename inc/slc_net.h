#ifndef __SOLDIELXJFD__
#define __SOLDIELXJFD__
#include "slc_port.h"
#define MAX_SOC_BUFFER_SIZE 1500
#define MIN_IPADDR_STREN_LEN   7
#define SLC_MAX_RCV_BUFFER_SIZE 1500

#define MAX_SOCKET_QUEUE_LIST   (10) //���֧��10��

typedef enum{
	SLC_CONECT_SUCCEED = 0,//���ӳɹ���
	
	SLC_RECONECT,//���ɹ�������.

	SLC_CONNECT_FIALD,
	
	SLC_VRG_VAIL,//��������
	
	SLC_READ_DATA,//��������
	
	SLC_SEND_SUCCEED,//���ͳɹ�
	
	SLC_SEND_FAILD,//����ʧ��
	
	SLC_CLOSED,//�ر�

	SLC_FAIL,//ʧ��

}SLC_SEND_DATA_RESULT;


typedef void (*slc_set_data_cb)(SLC_SEND_DATA_RESULT result,void *data);

typedef struct _slc_soc_queue{
	
		U8 * _send_data;//׼���ͳ���buffer ���10�顣

		U32 _data_lenth;//���ݳ��ȡ�

		int _send_count;//�Ѿ����͵Ĵ������������ʧ�ܾ͵ó���3�Σ����3�ζ�ʧ�ܣ��ͷ�����
		
		slc_set_data_cb CallBack; //�ص�

		U32 flag;
}slc_soc_queue;




typedef struct _slc_socket_queue{
	
	slc_soc_queue	_queue[MAX_SOCKET_QUEUE_LIST];
	
	signed char _index; //��ǰ���ӵ����������MAX_SOCKET_QUEUE_LIST  �����ˡ�

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
