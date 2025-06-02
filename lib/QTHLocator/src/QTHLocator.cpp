/*
 * Get the QTH locator by the internet connection
 * F.W. Krom, PE0FKO, April 2025
 * 
 * Website: ipinfo.io json format.
 *  {
 *	  "ip": "89.99.108.64",
 *	  "hostname": "89-99-108-64.cable.dynamic.v4.ziggo.nl",
 *	  "city": "Zutphen",
 *	  "region": "Gelderland",
 *	  "country": "NL",
 *	  "loc": "52.1383,6.2014",
 *	  "org": "AS33915 Vodafone Libertel B.V.",
 *	  "postal": "7201",
 *	  "timezone": "Europe/Amsterdam",
 *	  "readme": "https://ipinfo.io/missingauth"
 *	}
 */
#include <Arduino.h>
#include <QTHLocator.h>
#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#endif


#ifdef DEBUG_ESP_PORT
// Put the strings in PROGMEM, slow but free some (constant) ram memory.
  #define LOG_I(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR(F), ## __VA_ARGS__); }
//  #define LOG_I(F, ...)		{ if (display_status == DISPLAY_ON) DEBUG_ESP_PORT.printf(PSTR(F), ## __VA_ARGS__); }
  #define LOG_D(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR("DEBUG: " F), ## __VA_ARGS__); }
  #define LOG_W(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR("W: "     F), ## __VA_ARGS__); }
  #define LOG_E(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR("ERROR: " F), ## __VA_ARGS__); DEBUG_ESP_PORT.flush(); delay(100); }	// delay!!
  #define LOG_F(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR("FAIL: "  F), ## __VA_ARGS__); DEBUG_ESP_PORT.flush(); ESP.restart(); while(1); }
#else
  #define LOG_I(...)		{ }
  #define LOG_D(...)		{ }
  #define LOG_W(...)		{ }
  #define LOG_E(...)		{ }
  #define LOG_F(...)		{ }
#endif


QTHLocator::QTHLocator() 
{
}

QTHLocator::~QTHLocator() 
{
}

// bool QTHLocator::begin() 
// {
// 	if (WiFi.status() == WL_CONNECTED) 
// 	{
// 	}
// 	else 
// 	{
// 		LOG_E("No WiFi connection.\n");
// 		return false;
// 	}
// 	return true;
// }

bool	QTHLocator::load_api_location()
{
	String URL = "https://ipinfo.io/json";

	String data;
	if (fetchWebPage(data, URL) == HTTP_CODE_OK)
	{
		// Parse the JSON data
		DeserializationError error = deserializeJson(jsonQTH, data);
		if (error) 
		{
			LOG_E("Failed to parse JSON: %s\n", error.f_str());
			return false;
		}
		// LOG_I("JSON:"); serializeJson(jsonQTH, Serial);	LOG_I("\n");
		// Extract the data from the JSON object

		LOG_I("QTHLocator: IP %s\n", jsonQTH["ip"].as<String>().c_str());
		LOG_I("QTHLocator: Loc %s\n", jsonQTH["loc"].as<String>().c_str());
		// LOG_I("QTHLocator: %s\n", jsonQTH["hostname"].as<String>().c_str());
		LOG_I("QTHLocator: City %s\n", jsonQTH["city"].as<String>().c_str());
		// LOG_I("QTHLocator: %s\n", jsonQTH["region"].as<String>().c_str());
		// LOG_I("QTHLocator: %s\n", jsonQTH["country"].as<String>().c_str());
		// LOG_I("QTHLocator: %s\n", jsonQTH["org"].as<String>().c_str());
		// LOG_I("QTHLocator: %s\n", jsonQTH["postal"].as<String>().c_str());
		// LOG_I("QTHLocator: %s\n", jsonQTH["timezone"].as<String>().c_str());

		return true;
	}
	else
	{
		LOG_E("Get IP Info, locator, Error: %s\n", data.c_str());
	}

	return false;
}

bool	QTHLocator::load_api_sunrise(String location, const char* when)
{
	// URL = "http://api.sunrise-sunset.org/json?lat=52.1383&lng=6.2014&tzid=Europe/Amsterdam";

	// Get the sunrise/sunset data from the web
	String URL = "https://api.sunrise-sunset.org/json?lat=";
	// String loc = jsonQTH["loc"].as<String>();
	int commaIndex = location.indexOf(",");
	// if (commaIndex == -1) return HTTP_CODE_BAD_REQUEST;
	if (commaIndex == -1) 
		return false;

	String lat = location.substring(0, commaIndex);
	String lon = location.substring(commaIndex + 1);
	URL += lat;
	URL += "&lng=";
	URL += lon;
	URL += "&date=";				// today, tomorrow, or YYYY-MM-DD
	URL += when;					// when = "today", "tomorrow", or "YYYY-MM-DD"
	URL += "&formatted=0";			// 0 = ISO 8601 format, 1 = human readable format
	URL += "&tzid=Europe/Amsterdam";

	String data;
	if (fetchWebPage(data, URL) == HTTP_CODE_OK)
	{
		// LOG_I("Data: %s\n", data.c_str());
		DeserializationError error = deserializeJson(jsonSun, data);
		if (error) 
		{
			LOG_E("Failed to parse JSON: %s\n", error.f_str());
			return false;
		}
		else
		{
			LOG_I("QTHLocator: sunrise: %s\n", jsonSun["results"]["sunrise"].as<String>().c_str());
			LOG_I("QTHLocator: sunset : %s\n", jsonSun["results"]["sunset"].as<String>().c_str());
			LOG_I("QTHLocator: status : %s\n", jsonSun["status"].as<String>().c_str());
			// LOG_I("QTHLocator: %s\n", jsonSun["results"]["day_length"].as<String>().c_str());
			// LOG_I("QTHLocator: %s\n", jsonSun["results"]["solar_noon"].as<String>().c_str());
			// LOG_I("QTHLocator: %s\n", jsonSun["results"]["civil_twilight_begin"].as<String>().c_str());
			// LOG_I("QTHLocator: %s\n", jsonSun["results"]["civil_twilight_end"].as<String>().c_str());
			// LOG_I("QTHLocator: %s\n", jsonSun["results"]["nautical_twilight_begin"].as<String>().c_str());
			// LOG_I("QTHLocator: %s\n", jsonSun["results"]["nautical_twilight_end"].as<String>().c_str());
			// LOG_I("QTHLocator: %s\n", jsonSun["results"]["astronomical_twilight_begin"].as<String>().c_str());
			// LOG_I("QTHLocator: %s\n", jsonSun["results"]["astronomical_twilight_end"].as<String>().c_str());
			LOG_I("QTHLocator: tzid   : %s\n", jsonSun["tzid"].as<String>().c_str());

			// LOG_I("JSON:"); serializeJson(jsonSun, Serial);	LOG_I("\n");
			// LOG_I("JSON SunRise/SunSet:"); serializeJson(jsonSun, Serial);	LOG_I("\n");
			// LOG_I("SunRise: %s\n", jsonSun["results"]["sunrise"].as<String>().c_str());
			// LOG_I("SunSet:  %s\n", jsonSun["results"]["sunset"].as<String>().c_str());

			if (jsonSun["status"].as<String>() == "OK")
				return true;
		}
	}
	else
	{
		LOG_E("Https get SunRise/SunSet Error: %s\n", data.c_str());
	}

	return false;
}


String QTHLocator::latLonToMaidenhead(String loc) 
{
	int A = 'A';
	int commaIndex = loc.indexOf(",");
	if (commaIndex == -1) return String();

	float lat = loc.substring(0, commaIndex).toFloat();
	float lon = loc.substring(commaIndex + 1).toFloat();

    lon += 180.0;
    lat += 90.0;

    int fieldLon = lon / 20;
    int fieldLat = lat / 10;
    int squareLon = (lon - (fieldLon * 20)) / 2;
    int squareLat = (lat - (fieldLat * 10)) / 1;
    int subsquareLon = ((lon - (fieldLon * 20) - (squareLon * 2)) * 12);
    int subsquareLat = ((lat - (fieldLat * 10) - (squareLat * 1)) * 24);

    String maidenhead = "";
    maidenhead += char(A + fieldLon);
    maidenhead += char(A + fieldLat);
    maidenhead += String(squareLon);
    maidenhead += String(squareLat);
    maidenhead += char(A + subsquareLon);
    maidenhead += char(A + subsquareLat);

	// LOG_I("Maidenhead QTH: %s\n", maidenhead.c_str());

    return maidenhead;
}


// 21:24:27.830 > QTHLocator: 48.8534,2.3488
// 21:24:31.697 > >>>>> MaidenheadTolatLon: lon=2.375000, lat=48.854164
// 21:24:31.703 > QTH locator: by IP address config, JN18EU

String QTHLocator::MaidenheadTolatLon(String qth) 
{
	float lat, lon;

	int len = qth.length();
	if (len < 4) {
		lat = lon = 0;
		return String();
	}

	// Beginwaarden
	lon = -180 + (qth[0] - 'A') * 20;
	lat = -90 + (qth[1] - 'A') * 10;

	lon += (qth[2] - '0') * 2;
	lat += (qth[3] - '0') * 1;

	if (len >= 6) {
		lon += (qth[4] - 'A') * (2.0 / 24.0);
		lat += (qth[5] - 'A') * (1.0 / 24.0);
	}

	if (len >= 8) {
		lon += ((qth[6] - '0') * (2.0 / 240.0));
		lat += ((qth[7] - '0') * (1.0 / 240.0));
	}

	// Middelpunt van het vak
	if (len == 4) {
		lon += 1.0;
		lat += 0.5;
	} else if (len == 6) {
		lon += 1.0 / 24.0;
		lat += 0.5 / 24.0;
	} else if (len == 8) {
		lon += 1.0 / 240.0;
		lat += 0.5 / 240.0;
	}

	LOG_I("MaidenheadTolatLon: qth=%s, lat=%f, lon=%f\n", qth.c_str(), lat, lon);

	//TODO: Terug naar apparte var voor lat,lon
	return String(lat,4) + ',' + String(lon,4);
}


int QTHLocator::fetchWebPage(String& data, String URL)
{
	HTTPClient	http;
	int			httpCode = HTTPC_ERROR_CONNECTION_FAILED;

	LOG_I("QTHLocator::fetchWebPage: %s\n", URL.c_str());

	std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
	// WiFiClient*	client;

	data.clear();			// Clear the data string
	client->setInsecure();	// Ignore the SSL certificate

	http.setReuse(false);	// Do not reuse the connection
	// http.setTimeout(10000);	// Set the timeout to 10 seconds
	// http.setConnectTimeout(10000);	// Set the connect timeout to 10 seconds
	http.setUserAgent("PE0FKO Lib QTHLocator");
	// http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);	// Do not follow redirects
	// http.setRedirectLimit(0);	// Do not follow redirects
	// http.setUseHTTP10(true);	// Use HTTP/1.0 instead of HTTP/1.1
	// http.setAuthorization("username", "password");	// Set the username and password for basic authentication
	http.addHeader("Accept", "application/json");	// Set the Accept header to application/json
	// http.addHeader("Content-Type", "application/json");	// Set the Content-Type header to application/json
	// http.addHeader("User-Agent", "PE0FKO Lib QTHLocator");	// Set the User-Agent header to PE0FKO Lib QTHLocator

	if (http.begin(*client, URL))				//Initiate connection
	{
		if ((httpCode = http.GET()) > 0)		// Make request
		{
			// LOG_I("HTTP.GET... code: %d\n", httpCode);

			// if (httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
			// 	httpCode = HTTP_CODE_OK;

			if (httpCode == HTTP_CODE_OK)
			{
				data = http.getString();		// Get response
			}
		}
		else {
			LOG_W("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
		}

		http.end();
	}

	return httpCode;
}

/*
curl -s http://ipinfo.io/json|jq
{
  "ip": "89.99.108.64",
  "hostname": "89-99-108-64.cable.dynamic.v4.ziggo.nl",
  "city": "Zutphen",
  "region": "Gelderland",
  "country": "NL",
  "loc": "52.1383,6.2014",
  "org": "AS33915 Vodafone Libertel B.V.",
  "postal": "7201",
  "timezone": "Europe/Amsterdam",
  "readme": "https://ipinfo.io/missingauth"
}

curl -s http://ipinfo.io/json|jq
{
  "ip": "81.65.125.73",
  "hostname": "73.125.65.81.rev.sfr.net",
  "city": "Paris",
  "region": "Île-de-France",
  "country": "FR",
  "loc": "48.8534,2.3488",
  "org": "AS15557 Societe Francaise Du Radiotelephone - SFR SA",
  "postal": "75000",
  "timezone": "Europe/Paris",
  "readme": "https://ipinfo.io/missingauth"
}
*/

/*
curl -s http://api.sunrise-sunset.org/json?lat=52.1383\&lng=6.2014\&formatted=0\&tzid=Europe/Amsterdam|jq
{
  "results": {
    "sunrise": "2025-05-20T05:30:47+02:00",
    "sunset": "2025-05-20T21:32:43+02:00",
    "solar_noon": "2025-05-20T13:31:45+02:00",
    "day_length": 57716,
    "civil_twilight_begin": "2025-05-20T04:49:03+02:00",
    "civil_twilight_end": "2025-05-20T22:14:27+02:00",
    "nautical_twilight_begin": "2025-05-20T03:46:54+02:00",
    "nautical_twilight_end": "2025-05-20T23:16:36+02:00",
    "astronomical_twilight_begin": "1970-01-01T01:00:01+01:00",
    "astronomical_twilight_end": "1970-01-01T01:00:01+01:00"
  },
  "status": "OK",
  "tzid": "Europe/Amsterdam"
}
*/
