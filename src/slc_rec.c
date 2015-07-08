#ifdef __SLC_CUSTOM_FUN__
#include "slc_rec.h"


/*
	全局变量及宏控制。
*/

U16 gRecListName[SLC_MAX_FILENAME_LIST][SLC_MAX_REC_FILE_NAME_LEN]={0};//记录文件list
U16 gRecListActive[SLC_MAX_REC_FILE_NAME_LEN]={0};//记录当前文件list
U16 gRecFileNameSendActive[SLC_MAX_REC_FILE_NAME_LEN]={0};
U16 gRcvFileList[SLC_MAX_FILENAME_LIST][SLC_MAX_REC_FILE_NAME_LEN]={0};//下发的文件列表。

U8 gSocSendBuffer[1024*15]={0}; //用来发送的buffer 15K
U8 gSocReciveBuffer[1024*15]={0}; //用来收到的buffer 15K

/*
	删掉录音整个文件夹。初步设计是在每次开机要删掉这个文件夹。
*/
void slc_delete_rec_forder(void)
{
	U16 path[SLC_MAX_REC_FILE_NAME_LEN] = {0};

	kal_wsprintf(
	(kal_wchar*) path,
	"%c:\\%w",
	SRV_FMGR_PUBLIC_DRV,
	SLC_REC_FILEPATH);
	applib_file_delete_folder(path);
}
/*
	创建录音文件夹。开机时可创建一次。
*/
void slc_init_rec_dir(void)
{
	U16 filepath[127]={0};
	S32 attribute;

	kal_wsprintf(
	(kal_wchar*) filepath,
	"%c:\\%w\\",
	SRV_FMGR_PUBLIC_DRV,
	SLC_REC_FILEPATH);
    attribute = FS_GetAttributes(filepath);
    if (((FS_ATTR_DIR & attribute) == 0) || attribute < 0)
    {
        FS_CreateDir(filepath);
    }
}
/*
	检查某个文件是否存在。
	filename: 必须是FULL路径。
*/
MMI_BOOL slc_check_file_exist(UI_string_type filename)
{

    FS_HANDLE fd;

    fd = FS_Open(filename, FS_READ_ONLY);
    if (fd >= 0)
    {
        FS_Close(fd);
        return MMI_TRUE;
    }
    else
    {
        return MMI_FALSE;
    }
}
/*
	新增文件名。
*/
void slc_increase_file_name(UI_string_type filename)
{
    UI_string_type p;
    U16 fileCount = 0;

    p = filename + mmi_ucs2strlen((const S8*)filename); /* UCS2Strlen((const S8*)filename); */
    while (p > filename)
    {
        if (*p == L'.')
        {
            fileCount = (*(p - 2) - '0') * 10 + (*(p - 1) - '0');
            fileCount++;
            *(p - 1) = fileCount % 10 + '0';
            *(p - 2) = fileCount / 10 + '0';
            break;
        }
        p--;
    }
}
/*
	获取一个按时间排序的文件名:日时分秒格式。
	fullname: 绝对路径。
	eg: c:\SlcAudio\24145520.amr
*/
void slc_get_new_file(U16 *fullname)
{
    RTC_CTRL_GET_TIME_T   rtc_time;
    DCL_HANDLE rtc_handler = 0;
    int i, j;
    U16 filename[SLC_MAX_REC_FILE_NAME_LEN];
    U16 filepath[SLC_MAX_REC_FILE_NAME_LEN];
    FS_HANDLE file_handle=-1;
    
    rtc_handler = DclRTC_Open(DCL_RTC,FLAGS_NONE); 
    DclRTC_Control(rtc_handler, RTC_CMD_GET_TIME, (DCL_CTRL_DATA_T *)& rtc_time);
    DclRTC_Close(rtc_handler);
    kal_wsprintf(
        (kal_wchar*) filename,
        "%02d%02d%02d%02d",
        rtc_time.u1Day,
        rtc_time.u1Hour,
        rtc_time.u1Min,
        rtc_time.u1Sec);
        
	kal_wsprintf(
	(kal_wchar*) filepath,
	"%c:\\%w\\",
	SRV_FMGR_PUBLIC_DRV,
	SLC_REC_FILEPATH);
	kal_wsprintf(
	(kal_wchar*) fullname,
	"%w%w",
	filepath,
	(const kal_wchar*)filename);

    mmi_ucs2ncat((S8*) fullname, (S8*) L".amr", SLC_MAX_REC_FILE_NAME_LEN);

	file_handle = FS_Open(fullname, FS_READ_ONLY);

    for (i = j = 0; slc_check_file_exist(fullname); i++)
    {
        if (i == 100)
        {
            j++;
            kal_wsprintf(
                (kal_wchar*) filename,
                "%02d%02d%02d%02d%d00",
								rtc_time.u1Day,
								rtc_time.u1Hour,
								rtc_time.u1Min,
								rtc_time.u1Sec,
                j);
            i = -1;
        }
        else
        {
            slc_increase_file_name(fullname);
        }
    }
    FS_Close(file_handle);
}
/*
	在录音文件列表里清掉某一个路径。
*/
BOOL slc_del_filename_from_list(U16 *filepath)
{
	int i =0;
	BOOL isDel=FALSE;
	if(filepath == NULL) return;

	for(i=0;i<SLC_MAX_FILENAME_LIST;i++)
	{
		if(!mmi_wcscmp(gRecListName[i],filepath))
		{
			mmi_wcscpy(gRecListName[i], L"");
			isDel = TRUE;
			return TRUE;
		}
		else
		continue;
	}
	if(isDel == FALSE)
	{
		return FALSE;
	}
}
/*
	压入新增文件到文件LIST，操作C盘下的*.amr将会以这个gRecListName为依据进行删除与统计。
*/
BOOL slc_add_filename_to_list(U16 *filepath)
{
	int i =0;
	BOOL isAdd=FALSE;
	
	if(filepath == NULL) return;
	for(i=0;i<SLC_MAX_FILENAME_LIST;i++)
	{
		if(mmi_wcslen(gRecListName[i]) == 0)
		{
			mmi_wcscpy(gRecListName[i], (const WCHAR *) filepath);
			isAdd = TRUE;
			return TRUE;
		}
		else
		continue;
	}
	if(isAdd == FALSE)
	{
		return FALSE;
	}
}
/*
	得到一条文件记录。
*/
BOOL slc_get_file_from_list(U16 *filepath)
{
	int i =0;

	for(i=0;i<SLC_MAX_FILENAME_LIST;i++)
	{
#ifdef WIN32
	       mmi_asc_to_wcs(filepath, "c:\\01000016.amr");
	       return TRUE;
#endif
		if(mmi_wcslen(gRecListName[i]) != 0)
		{
			mmi_wcscpy(filepath,gRecListName[i]);
			return TRUE;
		}
	}
	return FALSE;
}
/*
	删除某一文件。绝对路径。
*/
BOOL slc_delete_file_from_filepath(U16 *filepath)
{
	char active_del[128]={0};
	FS_HANDLE file_handle;
	
	if(mmi_wcslen(filepath)== 0) return FALSE;

	mmi_wcs_to_asc(active_del,filepath);
	file_handle = FS_Delete(filepath);
	slc_print("slc_delete_file_from_filepath %s result:%d",active_del,file_handle);
	if(FS_NO_ERROR == file_handle)
	{
		mmi_wcscpy(filepath, L"");
		return TRUE;
	}
	else
	return FALSE;
}
/*
	录音接口。
	size_limit:   文件达到最大值会停止录音。
	time_limit:   时间限制。
*/
void slc_sndrec_entry_record(
        U16 *file_path,
        U32 size_limit,
        U32 time_limit,
        void (*callback_func) (BOOL result, void *_data))
{
	BOOL result = 0;
	/*
	 if (mmi_sndrec_check_usb_mode())
    {
		slc_print("USB mode filad!");
       return;
    }
    */
    result = mdi_audio_start_record_with_limit(
                file_path,
                MEDIA_AMR,
                0,
                slc_record_callback,
                NULL,
                size_limit,
                time_limit);
    if (!result)
    {
    	slc_print("start rec.............................");
		/*
			这里将显示UI，录音界面。
		*/
    }
    else
    {
    	memset(gRecListActive,0,sizeof(gRecListActive));
		slc_print("录音失败，全部删除了。:%d",result);
		slc_record_delete_all_file_and_list();
    }
}
/*
	得到将要发送语音的文件路径。
*/
U16 *slc_get_will_send_file_full_name(void)
{
#ifdef WIN32
	return L"c:\\01000016.amr";
#endif
	return gRecFileNameSendActive;
}
void slc_send_amr_to_ser(void)
{
	//step 1:得到 gRecListName 里的一条文件记录。
	memset(gRecFileNameSendActive,0,sizeof(gRecFileNameSendActive));
	if(slc_get_file_from_list(gRecFileNameSendActive) == TRUE)
	{
		char tempPath[127]={0};

		mmi_wcs_to_asc(tempPath, gRecFileNameSendActive);
		slc_print("will send %s ",tempPath);

		//step 2:根据gRecFileNameSendActive的路径，读取文件内容，组合packet
		slc_set_record_packet();
	}
	else
	{
		slc_print("empty gRecFileNameSendActive!!!");
	}
}
BOOL slc_record_save_to_file(void)
{
	char tempPath[50]={0};
	int time_duration=0;
	int i;
	BOOL result;
	
	mmi_wcs_to_asc(tempPath,gRecListActive);
	slc_print("rec ,filepath:%s size:%d",tempPath,applib_get_file_size(gRecListActive));

	mdi_audio_get_duration(gRecListActive,&time_duration);
	{	//if(result == MDI_AUDIO_SUCCESS)

		//#ifdef WIN32
		//if(0) 
		//#else
	    if ((applib_get_file_size(gRecListActive) <= 500)||(time_duration <= 1000))
		//#endif
		//if(0)
	    {
			slc_print("time_duration:%d  size:%d   delete!!!!!!!!",time_duration,applib_get_file_size(gRecListActive));
			slc_delete_file_from_filepath(gRecListActive);
			result = FALSE;
	    }
	    else
	    {
			//加到录音队列里。
			if(slc_add_filename_to_list(gRecListActive) == FALSE)
			{
				slc_print("list is full!! delete,re add.");
				//队列已经满了。删掉之前的保存数据。
				for(i=0;i<SLC_MAX_FILENAME_LIST;i++)
				{
					slc_delete_file_from_filepath(gRecListName[i]);
					memset(gRecListName[i],0,SLC_MAX_REC_FILE_NAME_LEN);
				}
				//然后再保存。
				if(TRUE==slc_add_filename_to_list(gRecListActive))
				{
					result = TRUE;
					slc_print("delete file,add list OK!!");
				}
				else
					result = FALSE;
			}
			else
			{
				result = TRUE;
				slc_print("add file list OK!!");
			}

	    }
    }
	memset(gRecListActive,0,sizeof(gRecListActive));
	return result;
}
void slc_record_delete_all_file_and_list(void)
{
	int i;

	slc_delete_rec_forder();
	for(i=0;i<(SLC_MAX_FILENAME_LIST);i++)
	{
		memset(gRecListName[i],0,sizeof(gRecListName[0]));
	}
}
/*
	录音回调函数。
*/
void slc_record_callback(MDI_RESULT result, void *_data)
{
//MDI_AUDIO_SUCCESS
	slc_print("slc_record_callback result %d",result);

	if((MDI_AUDIO_TERMINATED == result)||(MDI_AUDIO_SUCCESS == result))
	{
		if(TRUE == slc_record_save_to_file())
		{
			slc_msg_send(SLC_MSG_SEND_AMR_FILE,NULL);
		}
	}
	else if(MDI_AUDIO_DISC_FULL == result)
	{
		int i,res;
		char tmpPath[125]={0};
		for(i=0;i<(SLC_MAX_FILENAME_LIST-1);i++)
		{
			//删掉除最后一条。所有的文件
			res=slc_delete_file_from_filepath(gRecListName[i]);
			if(res == TRUE)
			{
				mmi_wcs_to_asc(tmpPath,gRecListName[i]);
				slc_print("delete file :",tmpPath);
			}
			memset(gRecListName[i],0,sizeof(gRecListName[0]));
		}
		slc_msg_send(SLC_MSG_SEND_AMR_FILE,NULL);
	}
	else
	{
		//其它错误:全部删掉。
		slc_record_delete_all_file_and_list();
	}
}
mdi_result slc_get_record_status(void)
{
	mdi_result result;

	#ifdef WIN32
		return MDI_AUDIO_BUSY;
	#endif
	if (!slc_mdi_audio_check_states_and_handlers(&result))
    {
        return result;
    }
}
void slc_record_end(void)
{

	slc_print("key up!! :%d",slc_get_record_status());
	if(slc_get_record_status() == MDI_AUDIO_BUSY)
	{
		slc_print("stop rec...");
		mdi_audio_stop_record();
		#ifdef WIN32
		slc_msg_send(SLC_MSG_SEND_AMR_FILE,NULL);
		#endif
		/*
			这里将增加录音成功后的处理。
		*/
		/*
		if(TRUE==slc_record_save_to_file())
		{
			slc_msg_send(SLC_MSG_SEND_AMR_FILE,NULL);
		}
		else
		{
			slc_print("slc_record_end add file faild...");
		}
		*/
	}
	else
	{
		
	}
}
/*******************************************************************************
** 函数: slc_GetDiskFreeSize
** 功能: 获取磁盘剩余空间
** 参数: drive, 磁盘盘符, 0x43~0x47
** 返回: 剩余空间的大小, byte为单位
*******/
U32 slc_GetDiskFreeSize(S32 drive)
{
    FS_DiskInfo disk_info;
    WCHAR path[8] = {0};
    U32 size;

    kal_wsprintf(path, "%c:\\", drive);
    
    if (FS_GetDiskInfo(
            path,
            &disk_info,
            FS_DI_BASIC_INFO | FS_DI_FREE_SPACE) >= FS_NO_ERROR)
    {
        size = disk_info.FreeClusters * disk_info.SectorsPerCluster * disk_info.BytesPerSector;
        return size;
    }

    return 0;
}
/*
	获取最后一次压入list的amr文件名。
*/
BOOL slc_GetAMRFileFromFilelist(U16 *LastFilePath)
{
	int i = SLC_MAX_FILENAME_LIST;

	for(;i>0;i--)
	{
		if(mmi_wcslen(gRcvFileList[i]) != 0)
		{
			mmi_wcscpy(LastFilePath,gRcvFileList[i]);
		}
	}
}
/*
	删除接收到的某一文件。绝对路径。
*/
BOOL slc_delete_file_from_receive_filepath(U16 *filepath)
{
	char active_del[128]={0};
	FS_HANDLE file_handle;
	
	if(mmi_wcslen(filepath)== 0) return FALSE;

	mmi_wcs_to_asc(active_del,filepath);
	file_handle = FS_Delete(filepath);
	slc_print("slc_delete_file_from_receive_filepath %s result:%d",active_del,file_handle);
	if(FS_NO_ERROR == file_handle)
	{
		mmi_wcscpy(filepath, L"");
		return TRUE;
	}
	else
	return FALSE;
}/*
	在接收到文件列表里清掉某一个路径。
*/
BOOL slc_del_filename_from_receive_list(U16 *filepath)
{
	int i =0;
	BOOL isDel=FALSE;
	if(filepath == NULL) return;

	for(i=0;i<SLC_MAX_FILENAME_LIST;i++)
	{
		if(!mmi_wcscmp(gRcvFileList[i],filepath))
		{
			mmi_wcscpy(gRcvFileList[i], L"");
			isDel = TRUE;
			return TRUE;
		}
		else
		continue;
	}
	if(isDel == FALSE)
	{
		return FALSE;
	}
}
BOOL slc_save_data_to_file(U8 *pData,U32 pDataLenth)
{	
	int i=0;
    FS_HANDLE file_handle;//文件句柄
	U32 LENG=0;
	char tmpPath[SLC_MAX_FILEPATH]={0};
	U32 size;
	
	if(pData == NULL) return FALSE;

	//step 1:计算剩余空间:
	if((size = slc_GetDiskFreeSize(SRV_FMGR_PUBLIC_DRV)) < 10*1024)
	{
		slc_print("获取空间，太小。%d K  失败 全部删掉。",size/1024);
		//全部删掉
		slc_record_delete_all_file_and_list();
	}

	for(i=0;i<SLC_MAX_FILENAME_LIST;i++)
	{
		if(mmi_wcslen(gRcvFileList[i]) == 0)
		{
			kal_wsprintf(
			(kal_wchar*) gRcvFileList[i],
			"%c:\\%w\\%02d",
			SRV_FMGR_PUBLIC_DRV,
			SLC_REC_FILEPATH,
			i);
			mmi_ucs2ncat((S8*) gRcvFileList[i], (S8*) L".amr", SLC_MAX_REC_FILE_NAME_LEN);
			break;
		}
	}

	file_handle = FS_Open(gRcvFileList[i],FS_CREATE|FS_READ_WRITE);
	if (file_handle >= FS_NO_ERROR)
	{
	    if(FS_Write(file_handle, pData, pDataLenth, &LENG) >= FS_NO_ERROR)
	    {
	    	mmi_wcs_to_asc(tmpPath, gRcvFileList[i]);
			slc_print("写入语音文件:%s,  size:%d",tmpPath,pDataLenth);
	    }
	}
	FS_Close(file_handle);
	return TRUE;
}
void slc_play_rec_callback_hdlr(mdi_result aud_result, void *user_data)
{
	U16 *AmrFilePath = (U16 *)user_data;
	char tmpFilePath[128]={0};
    
    switch (aud_result)
    {
    case MDI_AUDIO_END_OF_FILE:
    	mmi_wcs_to_asc(tmpFilePath,AmrFilePath);
    	slc_print("删掉路径和list:%s",tmpFilePath);
 		slc_del_filename_from_receive_list(AmrFilePath);
 		slc_delete_file_from_receive_filepath(AmrFilePath);
 		
 		if(TRUE == slc_GetAMRFileFromFilelist(AmrFilePath))
		slc_msg_send(SLC_MSG_PLAY_REC, NULL);
		else
		{
			slc_print("已经没有语音存在。停下。");
		}
		
        break;
    case MDI_AUDIO_TERMINATED:
    	{
			slc_print("播放声音被中断");
    	}
        break;
    default:
        break;
    }
}
/*******************************************************************************
** 函数: dtmf_PlayCustomVoiceFile
** 功能: 播放用户提示语
** 参数: filepath  -- 语音资源绝对路径。
**       cbf  -- 回调函数
** 返回: 是否播放成功
*******/
static MMI_BOOL dtmf_PlayCustomVoiceFile(U16* filepath)
{
    mdi_result result;

	if(!mdi_audio_is_playing(MDI_AUDIO_PLAY_FILE))
    {
    	slc_print("正在播放file??????Stop!!!!!!!");
        mdi_audio_stop_file();
    }
    
    //result = mdi_audio_snd_check_file_format(filepath);
    result = mdi_audio_play_file_with_vol_path(
                (void *)filepath,
                DEVICE_AUDIO_PLAY_ONCE,
                NULL,
                slc_play_rec_callback_hdlr,
                (void *)filepath,
                6,
                MDI_DEVICE_SPEAKER2);

    /*
    if (result == MDI_AUDIO_SUCCESS)
    {
        result = mdi_audio_snd_play_file_with_vol_path(
                    (void*)filepath,
                    1,
                    NULL,
                    NULL,
                    6,
                    MDI_DEVICE_SPEAKER2);
    }
	*/
    return (MMI_BOOL)(MDI_AUDIO_SUCCESS == result);
}
void slc_PlayAmrFile(void)
{
	U16 AmrFilePath[128]={0};
	//播放声音:
	if(TRUE == slc_GetAMRFileFromFilelist(AmrFilePath))
	{
		if(TRUE == dtmf_PlayCustomVoiceFile(AmrFilePath))
		{
			slc_print("正在播放...");
		}
		else
		{
			slc_print("播放失败。");
		}
	}
	else
	{
		slc_print("获取录音文件列表失败。");
	}
}
void slc_recive_vioce(void *data)
{
	slc_task_struct* pData=(slc_task_struct*)data;

	if(slc_save_data_to_file((U8 *)pData->_msg,pData->_msg_lenth)==TRUE)
	{
		if(!mdi_audio_is_playing(MDI_AUDIO_PLAY_FILE))
		{
			slc_msg_send(SLC_MSG_PLAY_REC, NULL);
		}
		else
		{
			slc_print("已经收到语音。但正在播放语音。播放完之后，再发消息吧。");
		}
	}
	//slc_release_mem_ptr((void *)pData->_msg);
	//pData->_msg = NULL;
}
/*
开始录音
*/
void slc_start_record_audio(void)
{
	U32 size;
	U16 tmpPath[128]={0};


	if(slc_IsSimUsable(MMI_SIM1) == FALSE)
	{
		slc_print("SIM卡不可用。");
		//do 
	}
	//step 1:计算剩余空间:
	if((size = slc_GetDiskFreeSize(SRV_FMGR_PUBLIC_DRV)) < 10*1024)
	{
		slc_print("获取空间，太小。%d K  失败 全部删掉。",size/1024);
		//全部删掉
		slc_record_delete_all_file_and_list();
	}
	
	//step 2:得到一个绝对路径的文件。
    memset(gRecListActive,0,sizeof(gRecListActive));
	slc_get_new_file(gRecListActive);
	//step 3:设置好录音最大时间:SLC_REC_MAX_TIME 和回调。
	slc_sndrec_entry_record((U16*) gRecListActive, 0,SLC_REC_MAX_TIME, slc_record_callback);
}
#endif
