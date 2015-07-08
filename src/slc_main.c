#ifdef __SLC_CUSTOM_FUN__
#include "slc_port.h"
#include "slc_main.h"



void slc_test_rec_to_ser(void)
{
	slc_print("test voice.............");
	slc_start_record_audio();	
	slc_send_amr_to_ser();
}
void slc_make_call(void)
{
	char tempCall[100]={0};

	mmi_asc_to_ucs2(tempCall,"1008611");
	MakeCall(tempCall);
}
void slc_mmi_init_all(void)
{
	static BOOL flg=FALSE;

	if(flg == FALSE)
	{
		slc_print("slc_init_all");

		//初始化录音。
		//slc_delete_rec_forder();
		//slc_init_rec_dir();

		flg = TRUE;
		slc_print("init wifi.....");
		#ifndef WIN32
		srv_dtcnt_wlan_init(slc_wifi_init_cb, (void *)NULL);
		#endif
		//srv_dtcnt_wlan_hw_reset_off(NULL,NULL);
		slc_init_socket();
	}
}
/*
	初始化NV值，设置项等...
*/
void slc_boot_up_init_config(void)
{
	//系统启动时初始化
}

void scl_mmi_task_process(ilm_struct *current_ilm)
{
	#define TASK_DATA       ((slc_task_struct *)current_ilm->local_para_ptr)
	switch(current_ilm->msg_id)
	{
		case SLC_MSG_INIT_ALL_ID:
		{
			//初始化
			//放到idle mmi_idle_classic_on_enter里调用。
			//slc_mmi_init_all();
		}
		break;
		case SLC_MSG_MAIN_ID:
		{
			//处理主线程。如:socket通信。进入UI
		}
		break;
		case SLC_MSG_SMS_IND_ID:
		{
			//来短信
		}break;
		case SLC_MSG_INCOMING_CALL_ID:
		{
			//来电话
		}break;
		 case SLC_MSG_GSENSOR_INTERRUPT:
		{
			//Gsensor中断
		}break;
		case SLC_MSG_UPDATA_UI:
		{
			//更新UI
		}break;
		case SLC_MSG_SEND_AMR_FILE:
		{
			//发送AMR
			slc_print("send amr file.");
			slc_send_amr_to_ser();
		}
		break;
		case SLC_MSG_START_REC:
		{	
			//开始录音。
			slc_print("start record!");
			slc_start_record_audio();
		}
		break;
		case SLC_MSG_RCV_REC:
		{
			slc_print("rcv voice!!");
			slc_recive_vioce(TASK_DATA);
		}	
		break;
		case SLC_MSG_PLAY_REC:
		{
			slc_print("play voice!!");
			slc_PlayAmrFile();
		}
		break;
		default:
		break;
	}
}

void slc_msg_send(msg_type msg_id, void *_data)
{
    ilm_struct* ilm = NULL;
    module_type src_mod_id;
	//slc_task_struct* p=NULL;

	//p = (slc_task_struct*) construct_local_para(sizeof(slc_task_struct), TD_CTRL);
	//p->_msg= _data;
    src_mod_id = MOD_MMI;
    ilm = allocate_ilm(src_mod_id);
    ilm->src_mod_id = src_mod_id;
    ilm->sap_id = 0;
    ilm->msg_id = (msg_type) msg_id;
    ilm->local_para_ptr = (local_para_struct*) _data;
    ilm->peer_buff_ptr = NULL;
    ilm->dest_mod_id = MOD_MMI;

    msg_send_ext_queue(ilm);
}


#endif
