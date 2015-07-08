#ifndef __SLC_MAIN_SSCSFAEW__
#define __SLC_MAIN_SSCSFAEW__
#include "slc_port.h"

void slc_msg_send(msg_type msg_id, void *_data);
typedef enum{

SLC_MSG_BASE_ID = MSG_ID_CUSTOM1_A,

SLC_MSG_INIT_ALL_ID,//初始化

SLC_MSG_SMS_IND_ID, //来短信

SLC_MSG_INCOMING_CALL_ID, //来电话

SLC_MSG_GSENSOR_INTERRUPT,//GSENSOR

SLC_MSG_MAIN_ID,//处理主线程。

SLC_MSG_UPDATA_UI,//更新UI

SLC_MSG_SEND_AMR_FILE,//发送AMR文件。

SLC_MSG_START_REC,//开始采样录音。

SLC_MSG_RCV_REC,//来语音。

SLC_MSG_PLAY_REC,//播放语音。

SLC_MSG_MAX_MSG_ID
/*custom msg id above!*/
}SCL_MSG_ID_ENUM;

typedef struct _slc_task_struct{
   LOCAL_PARA_HDR
   void *_msg;
   U32 _msg_lenth;
}slc_task_struct;


typedef struct {
    U8  bssid[WNDRV_MAC_ADDRESS_LEN]; /* MAC address */
	U8  ssid[WNDRV_SSID_MAX_LEN+1];
	U8  passphrase[SRV_DTCNT_PROF_MAX_PASSPHRASE_LEN+1];
	U16 key_len;
	U16 ssid_len;
}slc_ap_param;

extern void slc_delete_rec_forder(void);
extern void slc_init_rec_dir(void);
extern void slc_init_socket(void);
extern srv_dtcnt_result_enum srv_dtcnt_wlan_init(srv_dtcnt_wlan_cb_func_ptr callback, void *user_data);

#endif
