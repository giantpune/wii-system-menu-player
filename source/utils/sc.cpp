#include <ogc/conf.h>

const char* CONF_GetLanguageString()
{
	static int confLang = CONF_GetLanguage();

	switch( confLang )
	{
	case CONF_LANG_JAPANESE:		return "JPN";
	default:
	case CONF_LANG_ENGLISH:			return "ENG";
	case CONF_LANG_GERMAN:			return "GER";
	case CONF_LANG_FRENCH:			return "FRA";
	case CONF_LANG_SPANISH:			return "SPA";
	case CONF_LANG_ITALIAN:			return "ITA";
	case CONF_LANG_DUTCH:			return "NED";
	case CONF_LANG_SIMP_CHINESE:
	case CONF_LANG_TRAD_CHINESE:	return "CHN";
	case CONF_LANG_KOREAN:			return "KOR";
	}
}

s32 _CONF_GetEuRGB60()
{
	static s32 ret = CONF_GetEuRGB60();
	return ret;
}

s32 _CONF_GetAspectRatio()
{
	static s32 ret = CONF_GetAspectRatio();
	return ret;
}

s32 _CONF_GetVideo()
{
	static s32 ret = CONF_GetVideo();
	return ret;
}

