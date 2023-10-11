#pragma once

#include "SPIString.hpp"

class StreamSPIString : public Stream, public SPIString
{
public:
	size_t write(const uint8_t *data, size_t size) override
	{
		if (size && data)
		{
			const unsigned int newlen = length() + size;
			if (reserve(newlen + 1))
			{
				memcpy((void *)(wbuffer() + len()), (const void *)data, size);
				setLen(newlen);
				*(wbuffer() + newlen) = 0x00; // add null for string end
				return size;
			}
		}
		return 0;
	}

	size_t write(uint8_t data) override
	{
		return concat((char)data);
	}

	int available() override
	{
		return length();
	}

	int read() override
	{
		if (length())
		{
			char c = charAt(0);
			remove(0, 1);
			return c;
		}
		return -1;
	}

	int peek() override
	{
		if (length())
		{
			char c = charAt(0);
			return c;
		}
		return -1;
	}

	void flush() override
	{
	}
};
