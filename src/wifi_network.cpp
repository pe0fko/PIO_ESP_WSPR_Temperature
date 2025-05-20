#include "header.h"

volatile	bool	semaphore_wifi_connected		= false;
volatile	bool	semaphore_wifi_ip_address		= false;
volatile	bool	semaphore_wifi_ntp_received		= false;

static		WiFiEventHandler	mConnectHandler;							// WiFi event handler for the Connect
static		WiFiEventHandler	mDisConnectHandler;							// WiFi event handler for the Disconnect
static		WiFiEventHandler	mGotIpHandler;								// WiFi event handler for the GotIP


//---------------------------------------------------------------------------------
//---- Setup WiFi Station Mode
//---------------------------------------------------------------------------------

void init_wifi()
{
	LOG_I("WiFi setup station mode\n");

	semaphore_wifi_connected = false;
	semaphore_wifi_ip_address = false;
	semaphore_wifi_ntp_received = false;

	// WiFi.disconnect(false);										// Cleanup old info
	WiFi.mode(WIFI_STA);										// Set WiFi to station mode
	WiFi.setHostname(config.hostname.c_str());					// Set Hostname.
	WiFi.setAutoReconnect(true);								// Keep WiFi connected

	// Register WiFi event handlers
	mConnectHandler		= WiFi.onStationModeConnected(onWifiConnect);
	mGotIpHandler		= WiFi.onStationModeGotIP(onWiFiGotIP);
	mDisConnectHandler	= WiFi.onStationModeDisconnected(onWifiDisconnect);

	// Try to startup the WiFi Multi connection with the strongest AP found.
	addAllAPs();												// Add all APs to the list
}

void setup_wifi()
{
#ifdef FEATURE_OTA
	if ( ! config.hostname.isEmpty())
	{
		LOG_I("Setup OTA: %s\n", config.hostname.c_str());
		ArduinoOTA.setHostname(config.hostname.c_str());
#ifdef OTAPASSWD
		ArduinoOTA.setPassword(OTAPASSWD);
#endif
		ArduinoOTA.setRebootOnSuccess(true);
		ArduinoOTA.onStart([]() 
			{	ssd1306_printf_P(1000, PSTR("OTA update\nRunning")); 
				wspr_tx_disable(SI5351_CLK0);
				wspr_tx_disable(SI5351_CLK1);
				wspr_tx_disable(SI5351_CLK2);
			});
		ArduinoOTA.onEnd([]()   
			{	ssd1306_printf_P(1000, PSTR("OTA update\nReboot"));
				ESP.restart();
			});
		ArduinoOTA.begin();
	}
#endif
}

void loop_wifi()
{
	wifiMulti.run((120 - 1 - wspr_free_second) * 1000UL);

#ifdef FEATURE_OTA
	ArduinoOTA.handle();
#endif
}

void stop_wifi()
{
	WiFi.disconnect(true);		// WiFi off set true
	WiFi.mode(WIFI_OFF);		// Needed??
}

void onWifiConnect(const WiFiEventStationModeConnected& ssid)
{
	LOG_I("WiFi connected: SSID %s, channel %d\n", ssid.ssid.c_str(), ssid.channel);
	semaphore_wifi_connected = true;
}

void onWiFiGotIP(const WiFiEventStationModeGotIP& ipInfo)
{
	// LOG_I("WiFi IP %s, mask %s, GW:%s\n",
	// 	ipInfo.ip.toString().c_str(),
	// 	ipInfo.mask.toString().c_str(),
	// 	ipInfo.gw.toString().c_str()
	// );

	// // TESTING
	// Serial.println("==== Test DNS: ");
	ip_addr_t hostIP;
	dns_gethostbyname("time.google.com", &hostIP, NULL, 0);	// Get the DNS IP address
	// // IPAddress hostIP;
	// // WiFi.hostByName("ipinfo.io", hostIP);
	// // Serial.println(hostIP);
	// Serial.println(" ====");
	// // TESTING

	// sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);	// Smooth time sync
	// sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);	// Immediate time sync
	settimeofday_cb(cb_ntp_time_is_set);				// Call-back NTP function
	configTime(MYTZ, "time.google.com", "nl.pool.ntp.org");

	semaphore_wifi_ip_address = true;				// There is a new IP address
}

// callback routine - arrive here whenever a successful NTP update has occurred
void cb_ntp_time_is_set(bool from_sntp)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);					// Get the current time in sec and usec
	timer_us_one_second = micros();				// Initialize the timer only ones!



	timer_us_one_second -= tv.tv_usec;			// Correct the us to the sec tick





	LOG_I("NTP update at [%dus] [%d] %s", tv.tv_usec, timer_us_one_second, ctime(&tv.tv_sec));







	semaphore_wifi_ntp_received = true;
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& disconnectInfo)
{
	LOG_I("WiFi disconnected from SSID: %s, Reason: %d\n"
	,	disconnectInfo.ssid.c_str()
	,	disconnectInfo.reason
	);

	sntp_stop();

	semaphore_wifi_connected = false;
	semaphore_wifi_ip_address = false;
	semaphore_wifi_ntp_received = false;
}

uint32_t sntp_update_delay_MS_rfc_not_less_than_15000 ()
{
	return sntp_update_delay;
}
