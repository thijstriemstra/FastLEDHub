#ifndef _CONFIG_H
#define _CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPEssentials.h>

#include "EffectController.h"

class ConfigClass
{
	public:
		// time specific
		int8_t time_zone = 0;
		int8_t summer_time = 0;
		float longitude = 0;
		float latitude = 0;
		// alarm
		bool alarm_enabled = false;
		uint16_t alarm_duration = 0;
		uint8_t alarm_hour = 0;
		uint8_t alarm_minute = 0;
		String alarm_effect = "";
		String post_alarm_effect = "";
		// sunset
		bool sunset_enabled = false;
		uint16_t sunset_duration = 0;
		uint8_t sunset_hour = 0;
		uint8_t sunset_minute = 0;
		int16_t sunset_offset = 0;
		String sunset_effect = "";
		// other
		String startup_effect = "";
		String custom_color = "";
		String custom_color2 = "";
		String current_effect = "";
		uint8_t speed = 0;
		uint8_t saturation = 255;

		bool init();
		bool save();
		bool parseJSON(char* json);
		String getJSON();

	private:
		const String config_filename = "/config_json.txt";
};

extern ConfigClass Config;

#endif