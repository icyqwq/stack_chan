#pragma once

#include <Dynamixel2Arduino.h>

class servo
{
private:
	static constexpr char* TAG = "SERVO";
	uint8_t _servo_id = 1;
	uint32_t _servo_baud = 57600;
	Dynamixel2Arduino *_dxl;
	int _default_position = 0;
	int _last_position = 0;
	int _last_pos = 0;
	int _acc_delta = 0;
	int _min, _max;
	bool _failed = true;
public:
	servo(Dynamixel2Arduino& dxl, uint8_t id)
	{
		_servo_id = id;
		_dxl = &dxl;
		_default_position= 0;
		_min = 0;
		_max = 0;
	}
	~servo()
	{

	}

	bool ping()
	{
		_failed = !_dxl->ping(_servo_id);
		return !_failed;
	}

	void feed(int delta)
	{
		if (abs(getPosition() - _max) < 100) {
			return;
		}
		if (abs(getPosition() - _max) < 100) {
			return;
		}
		if (_last_position == getPosition()) {
			_acc_delta += delta;
			setPosition(_last_position + _acc_delta);
		} else {
			_last_position = getPosition();
			_acc_delta = delta;
			setPosition(_last_position + _acc_delta);
		}
	}

	void blink()
	{
		for (int i = 0; i < 4; i++)
		{
			_dxl->ledOff(_servo_id);
			delay(100);
			_dxl->ledOn(_servo_id);
			delay(100);
		}
	}

	int changeIDto(uint8_t new_id)
	{
		_dxl->torqueOff(_servo_id);
		if (_dxl->setID(_servo_id, new_id) == true)
		{
			_servo_id = new_id;
			ESP_LOGI(TAG, "ID has been successfully changed to %d", new_id);
			return 0;
		}
		else
		{
			ESP_LOGI(TAG, "Failed to change ID to %d", new_id);
			return -1;
		}
	}

	
	int scan(int baud)
	{
		int found_dynamixel = 0;
		_dxl->setPortProtocolVersion((float)2.0);
		_dxl->begin(baud);
		for (int id = 0; id < DXL_BROADCAST_ID; id++)
		{
			if (_dxl->ping(id))
			{
				ESP_LOGI(TAG, "FOUND: %d, model %d", id, _dxl->getModelNumber(id));
				_dxl->ledOn(id);
				found_dynamixel++;
			}
		}
		return found_dynamixel;
	}

	
	esp_err_t setup(int max, int min, int speed)
	{
		torqueOff();
		if (!_dxl->setOperatingMode(_servo_id, OP_CURRENT_BASED_POSITION)) {
			_failed = true;
			return ESP_FAIL;
		}
		_max = max;
		_min = min;
		_dxl->writeControlTableItem(ControlTableItem::CURRENT_LIMIT, _servo_id, 40);
		_dxl->writeControlTableItem(ControlTableItem::MAX_POSITION_LIMIT, _servo_id, max);
		_dxl->writeControlTableItem(ControlTableItem::MIN_POSITION_LIMIT, _servo_id, min);
		_dxl->writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, _servo_id, speed);
		_failed = false;
		return ESP_OK;
	}

	void setSpeed(int speed)
	{
		if (_failed) {
			return;
		}
		torqueOff();
		_dxl->writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, _servo_id, speed);
		torqueOn();
	}

	void setPosition(float x, uint8_t unit = 0)
	{
		if (_failed) {
			return;
		}
		_dxl->setGoalPosition(_servo_id, x, unit);
	}

	int getPosition(uint8_t unit = 0)
	{
		if (_failed) {
			return _last_pos;
		}
		int pos = _dxl->getPresentPosition(_servo_id, unit);
		if (pos < _min || pos > _max) {
			pos = _last_pos;
		}
		_last_pos = pos;
		return pos;
	}

	void torqueOff()
	{
		if (_failed) {
			return;
		}
		_dxl->torqueOff(_servo_id);
	}

	void torqueOn()
	{
		if (_failed) {
			return;
		}
		_dxl->torqueOn(_servo_id);
	}

	void setupDefaultPosition(int default_pos)
	{
		_default_position = default_pos;
	}

	int getDefaultPosition()
	{
		return _default_position;
	}

	void backToDefault()
	{
		setPosition(_default_position);
	}

	int getMin() { return _min; }
	int getMax() { return _max; }
};