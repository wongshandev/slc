#ifndef __SDSFEGHDHSFO__
#define __SDSFEGHDHSFO__
#include "MMI_include.h"
#include "kal_public_defs.h"
#include "Nvram_user_defs.h"
#include "conversions.h"
#include "task_main_func.h"
#include "app_ltlcom.h"
#include "task_config.h"
#include "syscomp_config.h"
#include "custom_util.h"
#include "stack_ltlcom.h"
#include "kal_non_specific_general_types.h"
#include "stack_config.h"
#include "custom_config.h"
#include "stack_timer.h"
#include "custom_config.h"
#include "med_utility.h"
#include "profilessrvgprot.h"
#include "DateTimeType.h"
#include "mmi_rp_srv_prof_def.h"
#include "gpiosrvgprot.h"
#include "mdi_audio.h"
#include "aud_defs.h"
#include "mmi_rp_app_charger_def.h"
#include "GeneralDeviceGprot.h"
#include "TimerEvents.h"
#include "stack_msgs.h"
#include "soc_api.h"
#include "cbm_api.h"
#include "DtcntSrvGprot.h"
#include "wndrv_cnst.h"
#include "FileMgrSrvGProt.h"
extern U32 slc_get_base_msg_id(void);
extern void slc_print(char * fmt,...);
extern char * slc_get_string_cs(void);
extern S32 slc_chset_convert(char *outbuf, S32 outbuflen, const char *inbuf, S32 inlen, mmi_chset_enum dest_chset);
extern S32 slc_convert_to_utf8(char *outbuf, S32 outbuflen, const char *inbuf, S32 inlen);
extern  void slc_wifi_init_cb(void *user_data, srv_dtcnt_wlan_req_res_enum res);
extern void slc_start_record_audio(void);
extern void slc_send_amr_to_ser(void);
extern   MMI_BOOL slc_mdi_audio_check_states_and_handlers(mdi_result *result_p);
extern mdi_result mdi_audio_stop_record(void);
extern BOOL slc_del_filename_from_list(U16 *filepath);
extern MMI_BOOL slc_IsSimUsable(mmi_sim_enum sim);
extern U8 gSocSendBuffer[1024*15]; //用来发送的buffer 15K
extern U8 gSocReciveBuffer[1024*15]; //用来收到的buffer 15K
#endif
