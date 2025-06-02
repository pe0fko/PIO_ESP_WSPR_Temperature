/*
 * Get the QTH locator by the internet connection
 * F.W. Krom, PE0FKO, April 2025
 * 
 * Website: ipinfo.io json format.
 */

#include <Arduino.h>
#include <QTHLocator.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

QTHLocator	QTH;
const	uint32_t	Value_GetQTH = 2*60*1000;	// Check every 2min
static	uint32_t	Timer_GetQTH = 0;			// Timer

const char* ssid = "pe0fko";
const char* password = "NetwerkBeheer114";

void setup() 
{
	Serial.begin(115200);
	delay(20);

	Serial.print("Verbinden met WiFi...");
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500); Serial.print(".");
	}

	Serial.println("\nVerbonden!");

	Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
	Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
	Serial.printf("Hostname: %s\n", WiFi.getHostname());
	Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
	Serial.printf("BSSID: %s\n", WiFi.BSSIDstr().c_str());
	Serial.printf("RSSI: %d\n", WiFi.RSSI());
	Serial.printf("Channel: %d\n", WiFi.channel());
	Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
	Serial.printf("Subnet: %s\n", WiFi.subnetMask().toString().c_str());
	Serial.printf("DNS0: %s\n", WiFi.dnsIP(0).toString().c_str());
	Serial.printf("DNS1: %s\n", WiFi.dnsIP(1).toString().c_str());
	Serial.printf("DNS2: %s\n", WiFi.dnsIP(2).toString().c_str());
	Serial.printf("DNS3: %s\n", WiFi.dnsIP(3).toString().c_str());

	Timer_GetQTH = -Value_GetQTH;
}

void loop() 
{
	if (millis() - Timer_GetQTH > Value_GetQTH) 
	{
		Timer_GetQTH += Value_GetQTH;

		if (QTH.begin()) {
			Serial.printf("QTH-locator from %s (IP:%s) is: %s [%s]\n" 
				, QTH.getOrg().c_str()
				, QTH.getIPAddress().c_str()
				, QTH.getQthLoc().c_str()
				, QTH.getLoc().c_str()
			);
		}
	}
}
