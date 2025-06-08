#include "header.h"

volatile	bool	semaphore_wifi_connected		= false;	// WiFi connected semaphore
volatile	bool	semaphore_wifi_ip_address		= false;	// WiFi IP address semaphore
volatile	bool	semaphore_wifi_ntp_received		= false;	// WiFi NTP time received semaphore

static		WiFiEventHandler	mConnectHandler;				// WiFi event handler for the Connect
static		WiFiEventHandler	mDisConnectHandler;				// WiFi event handler for the Disconnect
static		WiFiEventHandler	mGotIpHandler;					// WiFi event handler for the GotIP
static		WiFiEventHandler	mDhcpTimeout;					// WiFi event handler for the DHCP timeout
// static		WiFiEventHandler	mAuthModeChangedHandler;			// WiFi event handler for the AuthModeChanged

static		void	onWifiConnect(const WiFiEventStationModeConnected& ssid);
static		void	onWiFiGotIP(const WiFiEventStationModeGotIP& ipInfo);
static		void	onWifiDisconnect(const WiFiEventStationModeDisconnected& disconnectInfo);
static		void	onWifiDhcpTimeout();

static		void	cb_ntp_time_is_set(bool from_sntp);

//---------------------------------------------------------------------------------
//---- Setup WiFi Station Mode
//---------------------------------------------------------------------------------
void init_wifi()
{
	LOG_I("WiFi setup station mode\n");

	semaphore_wifi_connected = false;
	semaphore_wifi_ip_address = false;
	semaphore_wifi_ntp_received = false;

	// Register WiFi event handlers
	mConnectHandler		= WiFi.onStationModeConnected(onWifiConnect);
	mDisConnectHandler	= WiFi.onStationModeDisconnected(onWifiDisconnect);
	mGotIpHandler		= WiFi.onStationModeGotIP(onWiFiGotIP);
	mDhcpTimeout		= WiFi.onStationModeDHCPTimeout(onWifiDhcpTimeout);

	// WiFi.disconnect(false, true);							// Cleanup old wifi credentials in eeprom
	WiFi.persistent(false);
	WiFi.mode(WIFI_STA);										// Set WiFi to station mode
	WiFi.setHostname(config.hostname.c_str());					// Set Hostname.
	WiFi.setAutoReconnect(true);								// Keep WiFi connected
	// WiFi.waitForConnectResult();

	addAllAPs();												// Add all APs to the list
}

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------
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

	settimeofday_cb(cb_ntp_time_is_set);				// Call-back NTP function
	configTime(MYTZ, "time.google.com", "nl.pool.ntp.org");

	semaphore_wifi_ip_address = true;				// There is a new IP address
}

// callback routine - arrive here whenever a successful NTP update has occurred
void cb_ntp_time_is_set(bool from_sntp)
{
	// tv_usec should be "This is the rest of the elapsed time (a fraction of a second), 
	// represented as the number of microseconds. It is always less than one million."
	struct timeval tv;
	gettimeofday(&tv, NULL);					// Get the current time in sec and usec
	LOG_I("NTP update [%ld us] %s", tv.tv_usec, ctime(&tv.tv_sec));

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

void onWifiDhcpTimeout()
{
	WiFi.disconnect();			// WiFi disconnect, but not WiFi off

	semaphore_wifi_connected = false;
	semaphore_wifi_ip_address = false;
	semaphore_wifi_ntp_received = false;
}


uint32_t sntp_update_delay_MS_rfc_not_less_than_15000 ()
{
	return sntp_update_delay;
}

// WIFI_EVENT_STAMODE_CONNECTED = 0,
// WIFI_EVENT_STAMODE_DISCONNECTED,
// WIFI_EVENT_STAMODE_AUTHMODE_CHANGE,
// WIFI_EVENT_STAMODE_GOT_IP,
// WIFI_EVENT_STAMODE_DHCP_TIMEOUT,
// WIFI_EVENT_SOFTAPMODE_STACONNECTED,
// WIFI_EVENT_SOFTAPMODE_STADISCONNECTED,
// WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED,
// WIFI_EVENT_MODE_CHANGE,
// WIFI_EVENT_SOFTAPMODE_DISTRIBUTE_STA_IP,
// WIFI_EVENT_MAX,
// WIFI_EVENT_ANY = WIFI_EVENT_MAX,
