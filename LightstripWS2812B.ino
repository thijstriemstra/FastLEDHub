#include <Arduino.h>
#include <ESPEssentials.h>
#include <WiFiManager.h>

#include "WebSocket.h"
#include "EffectController.h"
#include "Hardware.h"
#include "Config.h"
#include "WebUpdate.h"

WiFiManager wifiManager;

void setup()
{
	Serial.begin(115200);
	Serial.println("");

	Config.init();
	initController();
	initHardware();
	begin(Config.startup_effect);

	wifiManager.autoConnect("Lightstrip AP");
	WebServer.init();

	OTA.init("Lightstrip");
	initWebsocket();
	initWebUpdate();
}

void loop()
{
	OTA.handle();
	webSocket.loop();
	WebServer.handleClient();	
	betterShow();
}