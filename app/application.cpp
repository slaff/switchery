#include <SmingCore.h>
#include <ConfigManager.h>

#define MODE "AP"
#define WIFI_NAME "switchery"
#define PASSWORD ""
#define HOSTNAME "switchery"
#define IP ""
#define AUTH_USERNAME ""
#define AUTH_PASSWORD ""
#define DEFAULT_PAGE "index.html"
#define CONFIG_PAGE "config.html"
#define OTA_AUTH_HASH ""

// #define LED_PIN 2 // GPIO2

const int JSON_OBJECT_SIZE = 512;

uint8_t switchArray[] = { 14, 12, 13 };
uint8_t switchArraySize = sizeof(switchArray) / sizeof(switchArray[0]);
rst_info* resetInf = system_get_rst_info();

Config defaultValues = { MODE, WIFI_NAME, PASSWORD, HOSTNAME, IP, AUTH_USERNAME, AUTH_PASSWORD, DEFAULT_PAGE, CONFIG_PAGE, OTA_AUTH_HASH };
Config config;

Timer procTimer, staConnectTimer;
bool state = true;

void staFailedToConnect();
void staConnected(IpAddress ip, IpAddress mask, IpAddress gateway);

// void blink()
// {
// 	digitalWrite(LED_PIN, state);
// 	state = !state;
// }

// Will be called when WiFi hardware and software initialization was finished
// And system initialization was completed
void ready() {
	if(WifiAccessPoint.isEnabled()) {
		Serial.print(F("AP. ip: "));
		Serial.println(WifiAccessPoint.getIP());
	}
}

void startAP() {
	Serial.println(F("Start in AP mode"));
	WifiStation.enable(false);
	WifiAccessPoint.enable(true);
	IpAddress ip(config.ip);
	if (!ip.isNull()) {
		WifiAccessPoint.setIP(ip);
	}
	WifiAccessPoint.config(config.ssid, config.password, strcmp(config.password, "") == 0 ? AUTH_OPEN : AUTH_WPA2_PSK);

	// Set system ready callback method
	System.onReady(ready);
}

void startSTA() {
	Serial.println(F("Start in STA mode."));

	staConnectTimer.initializeMs(15000, staFailedToConnect).startOnce();

	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	IpAddress ip(config.ip);
	if (!ip.isNull()) {
		WifiStation.setIP(ip);
	}
	WifiStation.config(config.ssid, config.password, true, false);

	WifiEvents.onStationGotIP(staConnected);
}

// Will be called when WiFi station was connected to AP
void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway) {
	Serial.print(F("Connected to "));
	Serial.println(config.ssid);
	Serial.print("IP address: ");
	Serial.println(ip);
}

void staFailedToConnect() {
	Serial.println(F("Failed to connect as station. Start in AP mode..."));
	config = {};
	ConfigManager::loadConfig(config, true);
	startAP();
}

void staConnected(IpAddress ip, IpAddress mask, IpAddress gateway) {
	staConnectTimer.stop();
	Serial.print("STA ip: ");
	Serial.println(ip);
}

void init() {
	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	// Serial.systemDebugOutput(true);
	Serial.commandProcessing(false);

	// Initialize GPIO pins
	for (uint8_t i = 0; i < switchArraySize; i++) {
		pinMode(switchArray[i], OUTPUT);
		if (resetInf->reason == REASON_DEFAULT_RST || resetInf->reason == REASON_EXT_SYS_RST) {
			digitalWrite(switchArray[i], HIGH);
		}
	}

	if (!spiffs_mount()) {
		return;
	}

	// Load config
	ConfigManager::loadConfig(config);
	if (strcmp(config.mode, "") == 0) {
		Serial.println(F("Could not start without configuration file.\nOperation abroted!"));
		return;
	}

	// Start WiFi
	if (strcmp(config.mode, "AP") == 0) {
		startAP();
	} else {
		startSTA();
	}

	// pinMode(LED_PIN, OUTPUT);
	// procTimer.initializeMs(1000, blink).start();
}
