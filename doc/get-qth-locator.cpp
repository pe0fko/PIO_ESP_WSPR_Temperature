#include <WiFi.h>
#include <HTTPClient.h>

// Vul hier je WiFi-gegevens in
const char* ssid = "JE_WIFI_NAAM";
const char* password = "JE_WIFI_WACHTWOORD";

String getIPLocation();
String latLonToMaidenhead(float lat, float lon);

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    
    Serial.print("Verbinden met WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nVerbonden!");
    
    String maidenhead = getIPLocation();
    Serial.print("Jouw QTH-locator is: ");
    Serial.println(maidenhead);
}

void loop() {
    // Niets in de loop nodig, maar kan worden uitgebreid
    delay(10000);
}

// Haalt de GPS-coördinaten op via ipinfo.io en converteert naar Maidenhead
String getIPLocation() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("https://ipinfo.io/json");
        int httpCode = http.GET();

        if (httpCode > 0) {
            String payload = http.getString();
            
            // Zoek de locatiegegevens (bijv. "loc": "52.3676,4.9041")
            int locIndex = payload.indexOf("\"loc\": \"");
            if (locIndex != -1) {
                int start = locIndex + 8;
                int end = payload.indexOf("\"", start);
                String loc = payload.substring(start, end);
                
                // Splits in latitude en longitude
                int commaIndex = loc.indexOf(",");
                float lat = loc.substring(0, commaIndex).toFloat();
                float lon = loc.substring(commaIndex + 1).toFloat();
                
                return latLonToMaidenhead(lat, lon);
            }
        }
        http.end();
    }
    return "FOUT";
}

// Zet breedte- en lengtegraad om naar Maidenhead Locator
String latLonToMaidenhead(float lat, float lon) {
    int A = 'A';
    
    lon += 180;
    lat += 90;

    int fieldLon = int(lon / 20);
    int fieldLat = int(lat / 10);
    int squareLon = int((lon - (fieldLon * 20)) / 2);
    int squareLat = int((lat - (fieldLat * 10)) / 1);
    int subsquareLon = int(((lon - (fieldLon * 20) - (squareLon * 2)) / 2) * 12);
    int subsquareLat = int(((lat - (fieldLat * 10) - (squareLat * 1)) / 1) * 24);

    String maidenhead = "";
    maidenhead += char(A + fieldLon);
    maidenhead += char(A + fieldLat);
    maidenhead += String(squareLon);
    maidenhead += String(squareLat);
    maidenhead += char(A + subsquareLon);
    maidenhead += char(A + subsquareLat);

    return maidenhead;
}
