#ifdef __SLC_CUSTOM_FUN__
#include "slc_rec.h"


/*
	ȫ�ֱ���������ơ�
*/

U16 gRecListName[SLC_MAX_FILENAME_LIST][SLC_MAX_REC_FILE_NAME_LEN]={0};//��¼�ļ�list
U16 gRecListActive[SLC_MAX_REC_FILE_NAME_LEN]={0};//��¼��ǰ�ļ�list
U16 gRecFileNameSendActive[SLC_MAX_REC_FILE_NAME_LEN]={0};
U16 gRcvFileList[SLC_MAX_FILENAME_LIST][SLC_MAX_REC_FILE_NAME_LEN]={0};//�·����ļ��б�

U8 gSocSendBuffer[1024*15]={0}; //�������͵�buffer 15K
U8 gSocReciveBuffer[1024*15]={0}; //�����յ���buffer 15K

/*
	ɾ��¼�������ļ��С������������ÿ�ο���Ҫɾ������ļ��С�
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
	����¼���ļ��С�����ʱ�ɴ���һ�Ρ�
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
	���ĳ���ļ��Ƿ���ڡ�
	filename: ������FULL·����
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
	�����ļ�����
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
	��ȡһ����ʱ��������ļ���:��ʱ�����ʽ��
	fullname: ����·����
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
	��¼���ļ��б������ĳһ��·����
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
	ѹ�������ļ����ļ�LIST������C���µ�*.amr���������gRecListNameΪ���ݽ���ɾ����ͳ�ơ�
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
	�õ�һ���ļ���¼��
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
	ɾ��ĳһ�ļ�������·����
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
	¼���ӿڡ�
	size_limit:   �ļ��ﵽ���ֵ��ֹͣ¼����
	time_limit:   ʱ�����ơ�
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
			���ｫ��ʾUI��¼�����档
		*/
    }
    else
    {
    	memset(gRecListActive,0,sizeof(gRecListActive));
		slc_print("¼��ʧ�ܣ�ȫ��ɾ���ˡ�:%d",result);
		slc_record_delete_all_file_and_list();
    }
}
/*
	�õ���Ҫ�����������ļ�·����
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
	//step 1:�õ� gRecListName ���һ���ļ���¼��
	memset(gRecFileNameSendActive,0,sizeof(gRecFileNameSendActive));
	if(slc_get_file_from_list(gRecFileNameSendActive) == TRUE)
	{
		char tempPath[127]={0};

		mmi_wcs_to_asc(tempPath, gRecFileNameSendActive);
		slc_print("will send %s ",tempPath);

		//step 2:����gRecFileNameSendActive��·������ȡ�ļ����ݣ����packet
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
			//�ӵ�¼�������
			if(slc_add_filename_to_list(gRecListActive) == FALSE)
			{
				slc_print("list is full!! delete,re add.");
				//�����Ѿ����ˡ�ɾ��֮ǰ�ı������ݡ�
				for(i=0;i<SLC_MAX_FILENAME_LIST;i++)
				{
					slc_delete_file_from_filepath(gRecListName[i]);
					memset(gRecListName[i],0,SLC_MAX_REC_FILE_NAME_LEN);
				}
				//Ȼ���ٱ��档
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
	¼���ص�������
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
			//ɾ�������һ�������е��ļ�
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
		//��������:ȫ��ɾ����
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
			���ｫ����¼���ɹ���Ĵ���
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
** ����: slc_GetDiskFreeSize
** ����: ��ȡ����ʣ��ռ�
** ����: drive, �����̷�, 0x43~0x47
** ����: ʣ��ռ�Ĵ�С, byteΪ��λ
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
	��ȡ���һ��ѹ��list��amr�ļ�����
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
	ɾ�����յ���ĳһ�ļ�������·����
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
	�ڽ��յ��ļ��б������ĳһ��·����
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
    FS_HANDLE file_handle;//�ļ����
	U32 LENG=0;
	char tmpPath[SLC_MAX_FILEPATH]={0};
	U32 size;
	
	if(pData == NULL) return FALSE;

	//step 1:����ʣ��ռ�:
	if((size = slc_GetDiskFreeSize(SRV_FMGR_PUBLIC_DRV)) < 10*1024)
	{
		slc_print("��ȡ�ռ䣬̫С��%d K  ʧ�� ȫ��ɾ����",size/1024);
		//ȫ��ɾ��
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
			slc_print("д�������ļ�:%s,  size:%d",tmpPath,pDataLenth);
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
    	slc_print("ɾ��·����list:%s",tmpFilePath);
 		slc_del_filename_from_receive_list(AmrFilePath);
 		slc_delete_file_from_receive_filepath(AmrFilePath);
 		
 		if(TRUE == slc_GetAMRFileFromFilelist(AmrFilePath))
		slc_msg_send(SLC_MSG_PLAY_REC, NULL);
		else
		{
			slc_print("�Ѿ�û���������ڡ�ͣ�¡�");
		}
		
        break;
    case MDI_AUDIO_TERMINATED:
    	{
			slc_print("�����������ж�");
    	}
        break;
    default:
        break;
    }
}
/*******************************************************************************
** ����: dtmf_PlayCustomVoiceFile
** ����: �����û���ʾ��
** ����: filepath  -- ������Դ����·����
**       cbf  -- �ص�����
** ����: �Ƿ񲥷ųɹ�
*******/
static MMI_BOOL dtmf_PlayCustomVoiceFile(U16* filepath)
{
    mdi_result result;

	if(!mdi_audio_is_playing(MDI_AUDIO_PLAY_FILE))
    {
    	slc_print("���ڲ���file??????Stop!!!!!!!");
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
	//��������:
	if(TRUE == slc_GetAMRFileFromFilelist(AmrFilePath))
	{
		if(TRUE == dtmf_PlayCustomVoiceFile(AmrFilePath))
		{
			slc_print("���ڲ���...");
		}
		else
		{
			slc_print("����ʧ�ܡ�");
		}
	}
	else
	{
		slc_print("��ȡ¼���ļ��б�ʧ�ܡ�");
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
			slc_print("�Ѿ��յ������������ڲ���������������֮���ٷ���Ϣ�ɡ�");
		}
	}
	//slc_release_mem_ptr((void *)pData->_msg);
	//pData->_msg = NULL;
}
/*
��ʼ¼��
*/
void slc_start_record_audio(void)
{
	U32 size;
	U16 tmpPath[128]={0};


	if(slc_IsSimUsable(MMI_SIM1) == FALSE)
	{
		slc_print("SIM�������á�");
		//do 
	}
	//step 1:����ʣ��ռ�:
	if((size = slc_GetDiskFreeSize(SRV_FMGR_PUBLIC_DRV)) < 10*1024)
	{
		slc_print("��ȡ�ռ䣬̫С��%d K  ʧ�� ȫ��ɾ����",size/1024);
		//ȫ��ɾ��
		slc_record_delete_all_file_and_list();
	}
	
	//step 2:�õ�һ������·�����ļ���
    memset(gRecListActive,0,sizeof(gRecListActive));
	slc_get_new_file(gRecListActive);
	//step 3:���ú�¼�����ʱ��:SLC_REC_MAX_TIME �ͻص���
	slc_sndrec_entry_record((U16*) gRecListActive, 0,SLC_REC_MAX_TIME, slc_record_callback);
}
#endif
