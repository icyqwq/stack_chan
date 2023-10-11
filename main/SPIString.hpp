#pragma once

#include <Arduino.h>
#include "esp_heap_caps.h"

class SPIString : public String
{
private:
	/* data */
public:
	SPIString(/* args */) {}

	using String::operator=;

	SPIString(float value, unsigned int decimalPlaces)
	{
		init();
		char *buf = (char*)heap_caps_malloc(decimalPlaces + 42, MALLOC_CAP_SPIRAM);
		if (buf) {
			*this = dtostrf(value, (decimalPlaces + 2), decimalPlaces, buf);
			free(buf);
		} else {
			*this = "nan";
			log_e("No enought memory for the operation.");
		}
	}

	SPIString(double value, unsigned int decimalPlaces)
	{
		init();
		char *buf = (char*)heap_caps_malloc(decimalPlaces + 312, MALLOC_CAP_SPIRAM);
		if (buf) {
			*this = dtostrf(value, (decimalPlaces + 2), decimalPlaces, buf);
			free(buf);
		} else {
			*this = "nan";
			log_e("No enought memory for the operation.");
		}
	}

protected:
	bool changeBuffer(unsigned int maxStrLen)  override
	{
		// Can we use SSO here to avoid allocation?
		if (maxStrLen < sizeof(sso.buff) - 1) {
			if (isSSO() || !buffer()) {
				// Already using SSO, nothing to do
				uint16_t oldLen = len();
				setSSO(true);
				setLen(oldLen);
			} else { // if bufptr && !isSSO()
				// Using bufptr, need to shrink into sso.buff
				char temp[sizeof(sso.buff)];
				memcpy(temp, buffer(), maxStrLen);
				free(wbuffer());
				uint16_t oldLen = len();
				setSSO(true);
				memcpy(wbuffer(), temp, maxStrLen);
				setLen(oldLen);
			}
			return true;
		}
		// Fallthrough to normal allocator
		size_t newSize = (maxStrLen + 16) & (~0xf);
		// Make sure we can fit newsize in the buffer
		if (newSize > CAPACITY_MAX) {
			return false;
		}
		uint16_t oldLen = len();
		char *newbuffer = (char *) heap_caps_realloc(isSSO() ? nullptr : wbuffer(), newSize, MALLOC_CAP_SPIRAM);
		// char *newbuffer = (char *) realloc(isSSO() ? nullptr : wbuffer(), newSize);
		if (newbuffer) {
			size_t oldSize = capacity() + 1; // include NULL.
			if (isSSO()) {
				// Copy the SSO buffer into allocated space
				memmove(newbuffer, sso.buff, sizeof(sso.buff));
			}
			if (newSize > oldSize) {
				memset(newbuffer + oldSize, 0, newSize - oldSize);
			}
			setSSO(false);
			setCapacity(newSize - 1);
			setBuffer(newbuffer);
			setLen(oldLen); // Needed in case of SSO where len() never existed
			return true;
		}
		return false;
	}

};
