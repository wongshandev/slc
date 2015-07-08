#ifdef __SLC_CUSTOM_WIFI__
#include "slc_wifi.h"
/*
	获取WIFI SSID 回调。
*/
static void slc_wifi_scan_cb(
    U32 job_id,
    void *user_data,
    srv_dtcnt_wlan_scan_result_struct *scan_res)
{
	slc_ap_param *scan_data = (slc_ap_param*)user_data;
	U32 i, conn_idx;
	U16 tempLen;
	U8 tempBuffer[(SRV_DTCNT_PROF_MAX_WLAN_PROF_NAME_LEN + 1) * ENCODING_LENGTH];

	slc_print("slc_wifi_scan_cb, pass=%s, ssid=%s. ssid_len=%d.", scan_data->passphrase, scan_data->ssid, scan_data->ssid_len);
	//bssinfo_to_profile
	for (i=0, conn_idx=scan_res->ap_list_num; i<scan_res->ap_list_num; i++)
	{
		slc_print("wifi scan, job=%d, [%d/%d] ssid=\"%s\", ssid_len=%d, rssi=%d", 
			job_id, i, scan_res->ap_list_num, 
			scan_res->ap_list[i]->ssid, scan_res->ap_list[i]->ssid_len, scan_res->ap_list[i]->rssi);

		//这里涉及到编码问题, 协议里面给过来的AP中的SSID为UNICODE编码, 而这里扫描出来的SSID的编码是不确定的
		//因此在进行memcmp之前要对编码进行统一处理
#if 0//不比较bssid
		if (_sh_wifi_is_mac_valid(scan_data->bssid, WNDRV_MAC_ADDRESS_LEN))
		{
			if (0 == memcmp(scan_data->bssid, scan_res->ap_list[i]->bssid, WNDRV_MAC_ADDRESS_LEN))
			{
				conn_idx = i;
				break;
			}
		}
#endif
			memset(tempBuffer, 0, (SRV_DTCNT_PROF_MAX_WLAN_PROF_NAME_LEN + 1) * ENCODING_LENGTH);
			tempLen = slc_convert_to_utf8(
					(char *)tempBuffer, 
					SRV_DTCNT_PROF_MAX_WLAN_PROF_NAME_LEN*ENCODING_LENGTH, 
					(const char*) scan_res->ap_list[i]->ssid,
					scan_res->ap_list[i]->ssid_len);
			
			if (0 == memcmp(tempBuffer, scan_data->ssid, scan_res->ap_list[i]->ssid_len))
			{
				conn_idx = i;
				break;
			}

	}
	//not found
	if (conn_idx >= scan_res->ap_list_num)
	{
		slc_print("slc_wifi_scan_cb, not found AP:\"%s\" [0x%x:0x%x:0x%x:0x%x:0x%x:0x%x].",
			scan_data->ssid, scan_data->bssid[0],scan_data->bssid[1],scan_data->bssid[2],scan_data->bssid[3],scan_data->bssid[4],scan_data->bssid[5]);
	}
	else
	{
		//已经扫描到了。
	}
	
}
/*
	wifi 初始化回调。
*/
 void slc_wifi_init_cb(void *user_data, srv_dtcnt_wlan_req_res_enum res)
{
	slc_ap_param *wifi_param = (slc_ap_param *)user_data;
	switch(res)
	{
	case SRV_DTCNT_WLAN_REQ_RES_DONE:
		{
			//slc_print("slc_wifi_init_cb, successful, start scan device. target=\"%s\".", wifi_param->ssid);
			//开始扫描。
			srv_dtcnt_wlan_scan(slc_wifi_scan_cb, (void *)NULL);
		}
		break;
		
	default:
		slc_print("slc_wifi_init_cb, failed. result=%d", res);
		break;
	}
}


#endif