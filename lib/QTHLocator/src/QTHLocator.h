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

#ifndef GetQTHLocator_h
#define GetQTHLocator_h

#include <Arduino.h>
#include <ArduinoJson.h>

class QTHLocator
{
public:
	QTHLocator();
	~QTHLocator();

	// bool	begin();		// Start with begin after a valid internet IP address
	
	bool	load_api_location();
	bool	load_api_sunrise(String location, const char* when = "today");

	// String	getQthLoc()			{ return latLonToMaidenhead(getLoc()); }
	// String	getLocation()		{ return MaidenheadTolatLon(qth); }

	String	latLonToMaidenhead(String loc);
	String	MaidenheadTolatLon(String qth);


	// int		getDayligth();
	String	getSunRise()	{ return jsonSun["results"]["sunrise"] | "6:00"; }
	String	getSunSet()		{ return jsonSun["results"]["sunset"]  | "18:00"; }

	// String	getIPAddress()	{ return jsonQTH["ip"]; }
	// String	getHostname()	{ return jsonQTH["hostname"]; }
	String	getCity()		{ return jsonQTH["city"]; }
	// String	getRegion()		{ return jsonQTH["region"]; }
	// String	getCountry()	{ return jsonQTH["country"]; }
	String	getLoc()		{ return jsonQTH["loc"]; }
	// String	getOrg()		{ return jsonQTH["org"]; }
	// String	getPostal()		{ return jsonQTH["postal"]; }
	// String	getTimezone()	{ return jsonQTH["timezone"]; }

	int		fetchWebPage(String& data, String URL);			// Read webpage to String data

private:
	JsonDocument	jsonQTH;
	JsonDocument	jsonSun;
};

#endif
