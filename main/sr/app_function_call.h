#pragma once

#include "app_common.h"

enum {
	APP_FUNC_CALL_ID_INVALID = 0,
	APP_FUNC_CALL_ID_CAMERA,
	APP_FUNC_CALL_ID_MOTION,
	APP_FUNC_CALL_ID_SELF_DESTRUCTION,
	APP_FUNC_CALL_ID_VOLUME,
	APP_FUNC_CALL_ID_WEATHER,
	APP_FUNC_CALL_ID_PRINTER,
};

extern const char * k_lang_postfix[3];
extern const char * k_lang_name[3];
extern const char * k_lang_model[3];

esp_err_t invoke_function(int id, String args, int lang);
void startTalk(bool motion=true);
void endTalk();
