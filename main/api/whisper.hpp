#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class Whisper: private HTTPClient
{
public: 
	typedef enum {
		ENGLISH,
		CHINESE,
		JAPANESE
	} language_t;
private:
	static constexpr char* TAG = "Whisper";
	static constexpr char* serverName = "https://api.openai.com/v1/audio/transcriptions";
	StaticJsonDocument<600> _resp_json_doc;
	String _openai_key;
	language_t _lang = ENGLISH;
public:
	String last_result;
	Whisper(const char* openai_key) 
	{
		_openai_key = openai_key;
	};
	~Whisper() {};

	language_t detectLanguage(const uint8_t *buffer) {
		uint32_t unicode = 0;
		int bytes = 0;
		int hasChinese = 0;

		for (int i = 0; buffer[i] != 0; ) {
			if ((buffer[i] & 0x80) == 0) {
				unicode = buffer[i];
				bytes = 1;
			} else if ((buffer[i] & 0xE0) == 0xC0) {
				unicode = (buffer[i] & 0x1F) << 6;
				bytes = 2;
			} else if ((buffer[i] & 0xF0) == 0xE0) {
				unicode = (buffer[i] & 0x0F) << 12;
				bytes = 3;
			} else {
				// Invalid UTF-8 or beyond our need
				return ENGLISH;
			}

			for (int j = 1; j < bytes; j++) {
				if ((buffer[i + j] & 0xC0) != 0x80) {
					// Invalid UTF-8
					return ENGLISH;
				}
				unicode |= (buffer[i + j] & 0x3F) << (6 * (bytes - j - 1));
			}

			if ((unicode >= 0x3040 && unicode <= 0x309F) || 
				(unicode >= 0x30A0 && unicode <= 0x30FF)) {
				return JAPANESE;
			} else if (unicode >= 0x4E00 && unicode <= 0x9FFF) {
				hasChinese = 1;
			}

			i += bytes;
		}

		if (hasChinese) return CHINESE;

		return ENGLISH;
	}

	esp_err_t request(uint8_t * audio_data, uint32_t size)
	{
		if (WiFi.status() != WL_CONNECTED) {
			ESP_LOGE(TAG, "Please connect to Wi-Fi.");
			return ESP_FAIL;
		}

		_resp_json_doc.clear();

		begin(serverName);
		
		int httpResponseCode = sendRequest("POST", audio_data, size);

		if (httpResponseCode < 0) {
			ESP_LOGE(TAG, "HTTP request failed, code = %d", httpResponseCode);
			end();
			return ESP_FAIL;
		}

		String response = getString();
		ESP_LOGI(TAG, "Resp Payload: %s", response.c_str());

		if (response.indexOf("error") >= 0) {
			return ESP_FAIL;
		}

		deserializeJson(_resp_json_doc, response);
		last_result = _resp_json_doc["text"].as<String>();
		ESP_LOGI(TAG, "ChatGPT Resp: %s", last_result.c_str());

		_lang = detectLanguage((uint8_t*)last_result.c_str());

		end();

		return ESP_OK;
	}

	String getLastResult() {
		return last_result;
	}

	language_t getLastLanguage() {
		return _lang;
	}

private:
	int sendRequest(const char * type, uint8_t * payload, size_t size)
	{
		static const char * start_payload = "--boundary1234567890\r\n"
							"Content-Disposition: form-data; name=\"file\"; filename=\"r.wav\"\r\n"
							"Content-Type: application/octet-stream\r\n"
							"\r\n";

		static const char * end_payload = "\r\n"
							"--boundary1234567890\r\n"
							"Content-Disposition: form-data; name=\"model\"\r\n"
							"\r\n"
							"whisper-1\r\n"
							"--boundary1234567890--\r\n";

		int code;
		bool redirect = false;
		uint16_t redirectCount = 0;
		size_t sent_bytes = 0, sent = 0;
		do {
			// wipe out any existing headers from previous request
			for(size_t i = 0; i < _headerKeysCount; i++) {
				if (_currentHeaders[i].value.length() > 0) {
					_currentHeaders[i].value.clear();
				}
			}

			log_d("request type: '%s' redirCount: %d\n", type, redirectCount);
			
			// connect to server
			if(!connect()) {
				return returnError(HTTPC_ERROR_CONNECTION_REFUSED);
			}

			addHeader("Authorization", String("Bearer ") + _openai_key);
			addHeader("Content-Type", "multipart/form-data; boundary=boundary1234567890");

			if(payload && size > 0) {
				addHeader(F("Content-Length"), String(size + strlen(start_payload) + strlen(end_payload)));
			}

			// add cookies to header, if present
			String cookie_string;
			if(generateCookieString(&cookie_string)) {
				addHeader("Cookie", cookie_string);
			}

			// send Header
			if(!sendHeader(type)) {
				return returnError(HTTPC_ERROR_SEND_HEADER_FAILED);
			}

			_client->write(start_payload);

			// send Payload if needed
			if(payload && size > 0) {
				size_t sent_bytes = 0;
				while(sent_bytes < size){
					size_t sent = _client->write(&payload[sent_bytes], size - sent_bytes);
					if (sent == 0){
						log_w("Failed to send chunk! Lets wait a bit");
						delay(100);
						sent = _client->write(&payload[sent_bytes], size - sent_bytes);
						if (sent == 0){
							log_e("Failed to send chunk!");
							break;
						}
					}
					sent_bytes += sent;
				}
				if(sent_bytes != size){
					return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
				}
			}
			delay(1);
			_client->write(end_payload);

			code = handleHeaderResponse();
			log_d("sendRequest code=%d\n", code);

			// Handle redirections as stated in RFC document:
			// https://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
			//
			// Implementing HTTP_CODE_FOUND as redirection with GET method,
			// to follow most of existing user agent implementations.
			//
			redirect = false;
			if (
				_followRedirects != HTTPC_DISABLE_FOLLOW_REDIRECTS && 
				redirectCount < _redirectLimit &&
				_location.length() > 0
			) {
				switch (code) {
					// redirecting using the same method
					case HTTP_CODE_MOVED_PERMANENTLY:
					case HTTP_CODE_TEMPORARY_REDIRECT: {
						if (
							// allow to force redirections on other methods
							// (the RFC require user to accept the redirection)
							_followRedirects == HTTPC_FORCE_FOLLOW_REDIRECTS ||
							// allow GET and HEAD methods without force
							!strcmp(type, "GET") || 
							!strcmp(type, "HEAD")
						) {
							redirectCount += 1;
							log_d("following redirect (the same method): '%s' redirCount: %d\n", _location.c_str(), redirectCount);
							if (!setURL(_location)) {
								log_d("failed setting URL for redirection\n");
								// no redirection
								break;
							}
							// redirect using the same request method and payload, diffrent URL
							redirect = true;
						}
						break;
					}
					// redirecting with method dropped to GET or HEAD
					// note: it does not need `HTTPC_FORCE_FOLLOW_REDIRECTS` for any method
					case HTTP_CODE_FOUND:
					case HTTP_CODE_SEE_OTHER: {
						redirectCount += 1;
						log_d("following redirect (dropped to GET/HEAD): '%s' redirCount: %d\n", _location.c_str(), redirectCount);
						if (!setURL(_location)) {
							log_d("failed setting URL for redirection\n");
							// no redirection
							break;
						}
						// redirect after changing method to GET/HEAD and dropping payload
						type = "GET";
						payload = nullptr;
						size = 0;
						redirect = true;
						break;
					}

					default:
						break;
				}
			}

		} while (redirect);
		// handle Server Response (Header)
		return returnError(code);
	}
};
