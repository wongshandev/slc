#ifdef __SLC_CUSTOM_FUN__
#include "slc_port.h"
extern kal_char* release_verno(void);


char * slc_get_string_cs(void)
{
	return "SLC";
}
char * slc_get_string_imei(void)
{
	return "752215580000770";
}
char * slc_get_product_name(void)
{
	return "F1";
}
char * slc_get_release_verno(void)
{
	return "V1.0";
}
/*
	ÅÐ¶Ïsim¿¨ÊÇ·ñ¿ÉÓÃ
*/
MMI_BOOL slc_IsSimUsable(mmi_sim_enum sim)
{
	MMI_BOOL exist;
	
	if (MMI_FALSE == (exist = srv_sim_ctrl_is_available(sim)))
    {   
	    
    }
    
	return exist;
}

S32 slc_chset_convert(char *outbuf, S32 outbuflen, const char *inbuf, S32 inlen, mmi_chset_enum dest_chset)
{
	S32 actuallength;
	mmi_chset_enum ret_charset;
	
	ret_charset = mmi_chset_guess(
                   inbuf,
                   inlen,
                   MMI_CHSET_BASE,
                   CHSET_GUESS_ALL);

	if (ret_charset == dest_chset)
	{
		memset(outbuf, 0, outbuflen);
		actuallength = inlen > outbuflen ? outbuflen : inlen;
		memcpy(outbuf, inbuf, actuallength);
		return actuallength;
	}
	
	actuallength = mmi_chset_convert(
				ret_charset,
				dest_chset,
				(char*)inbuf,
				(char*)outbuf,
				outbuflen);
		
    return actuallength;
}
S32 slc_convert_to_utf8(char *outbuf, S32 outbuflen, const char *inbuf, S32 inlen)
{
	S32 actuallength;

	actuallength = slc_chset_convert(outbuf, outbuflen, inbuf, inlen, CHSET_UTF8);
    return actuallength;
}

#endif
