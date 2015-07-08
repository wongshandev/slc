#ifndef __REC_SVSAGSECA__
#define __REC_SVSAGSECA__
#include "slc_port.h"
#include "slc_main.h"

#define SLC_MAX_REC_FILE_NAME_LEN   (80)
#define SLC_REC_FILEPATH       L"SlcAudio"
#define SLC_REC_MAX_TIME			(15)
#define SLC_REC_MAX_SIZE			(15*1024)
#define SLC_MAX_FILENAME_LIST     (10)//最大记录10 组
#define SLC_MAX_FILEPATH     (128)

void slc_init_rec_dir(void);
void slc_init_all(void);
extern void slc_msg_send(msg_type msg_id, void *_data);
void slc_delete_rec_forder(void);
void slc_record_callback(MDI_RESULT result, void *_data);
extern void slc_set_record_packet(void);
extern mdi_result mdi_audio_snd_stop(void);
void slc_record_delete_all_file_and_list(void);
#endif
