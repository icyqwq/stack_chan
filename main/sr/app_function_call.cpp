#include "app_function_call.h"

#include "declaration.h"
#include "anime.h"
#include "app_motion.h"
#include "app_task.h"

#define TAG "FuncCall"

const char * k_lang_postfix[3] = {
    "en.wav\0","cn.wav\0", "jp.wav\0",
};

const char * k_lang_name[3] = {
    "en-US\0","cmn-CN\0", "ja-JP\0",
};

const char * k_lang_model[3] = {
    "en-US-Wavenet-I\0", "cmn-CN-Wavenet-A\0", "ja-JP-Wavenet-A\0",
};

static Whisper::language_t chat_lang;

void startTalk()
{
	layer_hand_left->endMove();
	layer_sweat_0->endMove();
	layer_sweat_1->endMove();
	layer_sweat_2->endMove();
	layer_sweat_3->endMove();
	layer_eye_left->enable();
	layer_eye_right->enable();
	layer_mouth->startTalk();
	motion_send_cmd(MOTION_CMD_TALK_START);
}

void endTalk()
{
	layer_mouth->endTalk();
	app_eye_start();
	motion_send_cmd(MOTION_CMD_TALK_END);
}

void sayOK()
{
	startTalk();
	// motion_send_cmd(MOTION_CMD_NOD, 2);
	wav_player.playFromSD(String("/ok_") + k_lang_postfix[chat_lang]);
	endTalk();
}

esp_err_t func_call_snapshot()
{
	startTalk();
	wav_player.playFromSD(String("/camera-") + k_lang_postfix[chat_lang]);
	endTalk();

	if (suspendAnimeTask(2000) != ESP_OK) {
		ESP_LOGE(TAG, "Failed to suspend anime task");
		return ESP_FAIL;
	}
	cam.getFb();
	cam.releaseFb();
	cam.captureToScreen(0, 0, 320, 240);
	vTaskDelay(3000);
	resumeAnimeTask();
	return ESP_OK;
}

esp_err_t func_call_motion(String args)
{
	DynamicJsonDocument json(512);
	deserializeJson(json, args);
	if (!(json.containsKey("degrees") && json.containsKey("direction"))) {
		return ESP_FAIL;
	}
	sayOK();
	String dir = json["direction"].as<String>();
	if (dir.indexOf("left") >= 0) {
		layer_eye_left->setDstPos(-15, 0);
		layer_eye_right->setDstPos(-15, 0);
		motion_send_cmd(MOTION_CMD_TURN_LEFT, json["degrees"].as<int>());
	} else {
		layer_eye_left->setDstPos(15, 0);
		layer_eye_right->setDstPos(15, 0);
		motion_send_cmd(MOTION_CMD_TURN_RIGHT, json["degrees"].as<int>());
	}
	return ESP_OK;
}

esp_err_t func_call_self_destruction()
{
	layer_hand_left->endMove();
	layer_sweat_0->endMove();
	layer_sweat_1->endMove();
	layer_sweat_2->endMove();
	layer_sweat_3->endMove();
	layer_eye_left->enable();
	layer_eye_right->enable();

	switch_anime(ANIME_LAYER_RADIATION);
    layer_radiation->startMove();
	
	wav_player.playFromSD("/nuclear.wav");

	switch_anime(ANIME_LAYER_NORMAL);

	return ESP_OK;
}

esp_err_t func_call_set_vol(String args)
{
	DynamicJsonDocument json(512);
	deserializeJson(json, args);
	
	// if (json.containsKey("op")) {
	// 	String op = json["op"].as<String>();
	// 	if (op.indexOf("+") >= 0) {
	// 		int16_t volume = M5.Speaker.getVolume() + 20;
	// 		if (volume > 255) {
	// 			volume = 255;
	// 		}
	// 		M5.Speaker.setVolume(volume);
	// 		goto done;
	// 	}
	// 	else if (op.indexOf("-") >= 0) {
	// 		int16_t volume = M5.Speaker.getVolume() - 20;
	// 		if (volume < 0) {
	// 			volume = 0;
	// 		}
	// 		M5.Speaker.setVolume(volume);
	// 		goto done;
	// 	}
	// }

	if (json.containsKey("vol")) {
		int vol = json["vol"].as<int>();
		if (vol < 0) {
			vol = 0;
		}
		if (vol > 100) {
			vol = 100;
		}
		settings_get()->volume = (vol / 100.0f) * 255;
		M5.Speaker.setVolume(settings_get()->volume);
		settings_write_to_nvs();
		goto done;
	}
	
	ESP_LOGE(TAG, "Invalid arguments");
	return ESP_FAIL;

	done:
	sayOK();
	return ESP_OK;
}

esp_err_t invoke_function(int id, String args, int lang)
{
	chat_lang = (Whisper::language_t)lang;
	ESP_LOGI(TAG, "Funccall: %d, arg: %s", id, args.c_str());
    switch (id)
    {
    case APP_FUNC_CALL_ID_CAMERA:
        return func_call_snapshot();

	case APP_FUNC_CALL_ID_MOTION:
		return func_call_motion(args);

	case APP_FUNC_CALL_ID_SELF_DESTRUCTION:
		return func_call_self_destruction();

	case APP_FUNC_CALL_ID_VOLUME:
		return func_call_set_vol(args);
    
    default:
		ESP_LOGE(TAG, "Invalid function call ID = %d", id);
        return ESP_FAIL;
    }
    return ESP_OK;
}