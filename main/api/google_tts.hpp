#pragma once
#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <functional>
#include "app_common.h"
extern "C" {
    #include "libb64/cdecode.h"
    #include "libb64/cencode.h"
}

class GoogleTTS: private HTTPClient
{
public:
	typedef std::function<void(int)> speaker_callback_t;

private:
	static constexpr char* TAG = "GoogleTTS";
	static constexpr char* serverName = "https://texttospeech.googleapis.com/v1/text:synthesize?key=";
	static const uint16_t DECODE_BUF_SIZE = 2048;
	SpiRamJsonDocument *_req_json_doc;
	speaker_callback_t _speaker_callback;
	// StaticJsonDocument<1024> _req_json_doc;
	StaticJsonDocument<600> _resp_json_doc;
	String _api_key;
	uint8_t _decode_buffer[3][DECODE_BUF_SIZE];
	uint8_t _decode_buffer_sel = 0;
	base64_decodestate _base64_state;
	uint32_t _last_offset = 0;
	int _cnt = 0;

	enum {
			STAGE_WAIT_START = 0,
			STAGE_PLAYING
	};
	uint8_t _recv_stage = STAGE_WAIT_START;
public:
	

	GoogleTTS(const char* api_key)
	{
		_req_json_doc = new SpiRamJsonDocument(32 * 1024);
		_api_key = api_key;
		_speaker_callback = nullptr;
	}
	~GoogleTTS() 
	{
		delete _req_json_doc;
	}

	void setSpeakerStartCallback(speaker_callback_t callback)
	{
		_speaker_callback = callback;
	}

	esp_err_t request(String text, String lang, String model, String gender = "FEMALE")
	{
		if (WiFi.status() != WL_CONNECTED) {
			ESP_LOGE(TAG, "Please connect to Wi-Fi.");
			return ESP_FAIL;
		}

		_req_json_doc->clear();
		_resp_json_doc.clear();

		begin(serverName + _api_key);
		addHeader("Content-Type", "application/json; charset=utf-8");

		(*_req_json_doc)["input"]["ssml"] = text;
		(*_req_json_doc)["voice"]["languageCode"] = lang;
		(*_req_json_doc)["voice"]["name"] = model;
		(*_req_json_doc)["voice"]["ssmlGender"] = gender;
		(*_req_json_doc)["audioConfig"]["audioEncoding"] = "LINEAR16";
		(*_req_json_doc)["audioConfig"]["sampleRateHertz"] = 16000;
		(*_req_json_doc)["audioConfig"]["speakingRate"] = 1.25;
		(*_req_json_doc)["audioConfig"]["pitch"] = 0;
		String requestBody;
        serializeJson(*_req_json_doc, requestBody);
		ESP_LOGI(TAG, "Request JSON: %s", requestBody.c_str());

		int httpResponseCode = POST(requestBody);

		if (httpResponseCode < 0) {
			ESP_LOGE(TAG, "HTTP request failed, code = %d", httpResponseCode);
			end();
			return ESP_FAIL;
		}

		// String response = getString();
		// ESP_LOGI(TAG, "Resp Payload: %d", response.length());
		MEMCHECK
		httpResponseCode = writeToSpeaker();
		MEMCHECK

		if (httpResponseCode < 0) {
			ESP_LOGE(TAG, "HTTP fetch failed, code = %d", httpResponseCode);
			end();
			return ESP_FAIL;
		}

		end();

		while (M5.Speaker.isPlaying()) { M5.delay(1); }

		return ESP_OK;
	}

	esp_err_t testGoogle()
	{
		HTTPClient http;
		http.begin("https://www.google.com");
		int httpResponseCode = http.GET();
		if (httpResponseCode > 0) {
			String response = http.getString();
			ESP_LOGI(TAG, "HTTP Response code: %d", httpResponseCode);
			ESP_LOGI(TAG, "Google test passed");
			// ESP_LOGI(TAG, "Response: %s", response.c_str());
		} else {
			ESP_LOGE(TAG, "Error on sending GET request: %s", http.errorToString(httpResponseCode).c_str());
			ESP_LOGE(TAG, "Cannot reach real internet.");
			return ESP_FAIL;
		}
		http.end();
		return ESP_OK;
	}


private:
	int parseWav(const uint8_t* wav_data)
	{
		struct __attribute__((packed)) wav_header_t
		{
			char RIFF[4];
			uint32_t chunk_size;
			char WAVEfmt[8];
			uint32_t fmt_chunk_size;
			uint16_t audiofmt;
			uint16_t channel;
			uint32_t sample_rate;
			uint32_t byte_per_sec;
			uint16_t block_size;
			uint16_t bit_per_sample;
		};
		struct __attribute__((packed)) sub_chunk_t
		{
			char identifier[4];
			uint32_t chunk_size;
			uint8_t data[1];
		};

		auto wav = (wav_header_t*)wav_data;
		
		ESP_LOGI("wav", "RIFF           : %.4s" , wav->RIFF          );
		ESP_LOGI("wav", "chunk_size     : %d"   , wav->chunk_size    );
		ESP_LOGI("wav", "WAVEfmt        : %.8s" , wav->WAVEfmt       );
		ESP_LOGI("wav", "fmt_chunk_size : %d"   , wav->fmt_chunk_size);
		ESP_LOGI("wav", "audiofmt       : %d"   , wav->audiofmt      );
		ESP_LOGI("wav", "channel        : %d"   , wav->channel       );
		ESP_LOGI("wav", "sample_rate    : %d"   , wav->sample_rate   );
		ESP_LOGI("wav", "byte_per_sec   : %d"   , wav->byte_per_sec  );
		ESP_LOGI("wav", "block_size     : %d"   , wav->block_size    );
		ESP_LOGI("wav", "bit_per_sample : %d"   , wav->bit_per_sample);
		
		if ( !wav_data
			|| memcmp(wav->RIFF,    "RIFF",     4)
			|| memcmp(wav->WAVEfmt, "WAVEfmt ", 8)
			|| wav->audiofmt != 1
			|| wav->bit_per_sample < 8
			|| wav->bit_per_sample > 16
			|| wav->channel == 0
			|| wav->channel > 2
			)
		{
			return 0;
		}

		sub_chunk_t* sub = (sub_chunk_t*)(wav_data + offsetof(wav_header_t, audiofmt) + wav->fmt_chunk_size);
		/*
		ESP_LOGD("wav", "sub id         : %.4s" , sub->identifier);
		ESP_LOGD("wav", "sub chunk_size : %d"   , sub->chunk_size);
		*/
		while(memcmp(sub->identifier, "data", 4) && (uint8_t*)sub < wav_data + wav->chunk_size + 8)
		{
			sub = (sub_chunk_t*)((uint8_t*)sub + offsetof(sub_chunk_t, data) + sub->chunk_size);
			/*
			ESP_LOGD("wav", "sub id         : %.4s" , sub->identifier);
			ESP_LOGD("wav", "sub chunk_size : %d"   , sub->chunk_size);
			*/
		}
		if (memcmp(sub->identifier, "data", 4))
		{
			return 0;
		}

		return sub->data - wav_data;
	}

	int writeToSpeaker()
	{
		_cnt = 0;
		_last_offset = 0;
		_decode_buffer_sel = 0;
		_recv_stage = STAGE_WAIT_START;
		base64_init_decodestate(&_base64_state);

		if(!connected()) {
			return returnError(HTTPC_ERROR_NOT_CONNECTED);
		}

		// get length of document (is -1 when Server sends no Content-Length header)
		int len = _size;
		int ret = 0;

		if(_transferEncoding == HTTPC_TE_IDENTITY) {
			ret = writeDataToSpeaker(len);

			// have we an error?
			if(ret < 0) {
				return returnError(ret);
			}
		} else if(_transferEncoding == HTTPC_TE_CHUNKED) {
			int size = 0;
			while(1) {
				if(!connected()) {
					return returnError(HTTPC_ERROR_CONNECTION_LOST);
				}
				String chunkHeader = _client->readStringUntil('\n');

				if(chunkHeader.length() <= 0) {
					return returnError(HTTPC_ERROR_READ_TIMEOUT);
				}

				chunkHeader.trim(); // remove \r

				// read size of chunk
				len = (uint32_t) strtol((const char *) chunkHeader.c_str(), NULL, 16);
				size += len;
				log_d(" read chunk len: %d", len);

				// data left?
				if(len > 0) {
					int r = writeDataToSpeaker(len);
					if(r < 0) {
						// error in writeToStreamDataBlock
						return returnError(r);
					}
					ret += r;
				} else {

					// if no length Header use global chunk size
					if(_size <= 0) {
						_size = size;
					}

					// check if we have write all data out
					if(ret != _size) {
						return returnError(HTTPC_ERROR_STREAM_WRITE);
					}
					break;
				}

				// read trailing \r\n at the end of the chunk
				char buf[2];
				auto trailing_seq_len = _client->readBytes((uint8_t*)buf, 2);
				if (trailing_seq_len != 2 || buf[0] != '\r' || buf[1] != '\n') {
					return returnError(HTTPC_ERROR_READ_TIMEOUT);
				}

				delay(0);
			}
		} else {
			return returnError(HTTPC_ERROR_ENCODING);
		}

	//    end();
		disconnect(true);
		return ret;
	}

	int writeDataToSpeaker(int size)
	{
		if (_speaker_callback) {
			_speaker_callback(0);
		}

		int buff_size = HTTP_TCP_BUFFER_SIZE;
		int len = size;
		int bytesWritten = 0;

		// if possible create smaller buffer then HTTP_TCP_BUFFER_SIZE
		if((len > 0) && (len < HTTP_TCP_BUFFER_SIZE)) {
			buff_size = len;
		}

		// create buffer for read
		uint8_t * buff = (uint8_t *) malloc(buff_size + 8);

		if(buff) {
			// read all data from server
			while(connected() && (len > 0 || len == -1)) {

				// get available data size
				size_t sizeAvailable = _client->available();

				if(sizeAvailable) {

					int readBytes = sizeAvailable;

					// read only the asked bytes
					if(len > 0 && readBytes > len) {
						readBytes = len;
					}

					// not read more the buffer can handle
					if(readBytes > buff_size) {
						readBytes = buff_size;
					}
				
					// stop if no more reading    
					if (readBytes == 0)
						break;

					// read data
					int bytesRead = _client->readBytes(buff, readBytes);
					bytesWritten += bytesRead;

					switch (_recv_stage)
					{
					case STAGE_WAIT_START: {
						char * p_start = strnstr((char*)buff, "UklGR", bytesRead);
						if (p_start == NULL) {
							goto end;
						}

						int cnt = base64_decode_block(p_start, bytesRead - (p_start - (char*)buff), (char*)(_decode_buffer[0]), &_base64_state);
						int offset = parseWav(_decode_buffer[0]);

						_cnt = cnt - offset;
						memcpy(_decode_buffer[1], _decode_buffer[0] + offset, _cnt);

						_decode_buffer_sel = 1;
						_recv_stage = STAGE_PLAYING;
					}
					break;

					case STAGE_PLAYING: {
						if (_cnt + bytesRead * 0.75 < DECODE_BUF_SIZE) { // not filling up
							_cnt += base64_decode_block((char*)buff, bytesRead, (char*)(_decode_buffer[_decode_buffer_sel] + _cnt), &_base64_state);
							goto end;
						} else {
							uint8_t current_sel = _decode_buffer_sel;
							if (_decode_buffer_sel < 2) { // switch to next buffer
								_decode_buffer_sel++;
							} else {
								_decode_buffer_sel = 0;
							}

							if (_cnt % 2) {
								_decode_buffer[_decode_buffer_sel][0] = _decode_buffer[current_sel][(_cnt / 2) * 2];
								_last_offset = 1;
							} else {
								_last_offset = 0;
							}

							// printf("%d, %d\n", current_sel, _cnt);
							M5.Speaker.playRaw((int16_t*)_decode_buffer[current_sel], (_cnt) / 2, 16000, false, 1, 1, 0);
							_cnt = base64_decode_block((char*)buff, bytesRead, (char*)(_decode_buffer[_decode_buffer_sel] + _last_offset), &_base64_state) + _last_offset;
						}
					}
					break;
					}

				end:
					// count bytes to read left
					if(len > 0) {
						len -= readBytes;
					}

					delay(0);
				} else {
					delay(1);
				}
			}

			free(buff);

			log_d("connection closed or file end (written: %d).", bytesWritten);

			if((size > 0) && (size != bytesWritten)) {
				log_d("bytesWritten %d and size %d mismatch!.", bytesWritten, size);
				return HTTPC_ERROR_STREAM_WRITE;
			}

		} else {
			log_w("too less ram! need %d", HTTP_TCP_BUFFER_SIZE);
			return HTTPC_ERROR_TOO_LESS_RAM;
		}

		return bytesWritten;
	}
};
