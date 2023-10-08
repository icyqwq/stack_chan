#pragma once

#include "app_task.h"

enum
{
	MOTION_CMD_NONE = 0,
	MOTION_CMD_NOD,
	MOTION_CMD_TURN_RIGHT,
	MOTION_CMD_TURN_LEFT,
	MOTION_CMD_TALK_START,
	MOTION_CMD_TALK_END,
	MOTION_CMD_DIZZY,
	MOTION_CMD_BACK
};

void motion_nod(int times);

void motion_init();
esp_err_t motion_send_cmd(uint8_t cmd, int data);
esp_err_t motion_send_cmd(uint8_t cmd);
esp_err_t motion_send_cmd(app_common_cmd_t motion);