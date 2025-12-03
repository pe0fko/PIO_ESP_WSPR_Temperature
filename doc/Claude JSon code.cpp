/*
{
  "system": {
    "version": "1.0.0",
    "build": "2024-05-01 12:00:00",
      "chipid": 8065906,
      "hostname": "wsprtx",
      "display_off": 60
  },

  "user": {
    "call": "PE0FKO",
    "prefix": "",
    "suffix": "",
    "locator": "JO32CD",
    "locator_FR": "JN13IW",
    "power": "10",
  },

  "si5351": {
      "enable": true,
    "drive_strength": 8,
      "calibration": -900,
  },

  "timezones": [
    {
      "start": "0:0",
      "sunrise": "-1:0",
      "clk": 0,
      "list": "clk0",
      "name": "Nigth light zone"
    },
    {
      "sunrise": "-1:00",
      "sunset": "+1:00",
      "clk": 0,
      "list": "clk0",
      "name": "Day light zone"
    },
    {
      "sunset": "+1:0",
      "end": "24:00",
      "clk": 0,
      "list": "clk0",
      "name": "Night light zone"
    },
    {
      "start": "00:00",
      "end": "24:00",
      "clk": 1,
      "list": "clk1",
      "name": "Freq list for clk1"
    },
    {
      "start": "00:00",
      "end": "24:00",
      "clk": 2,
      "list": "clk2",
      "name": "Freq list for clk2"
    }
  ],
  "clk0": [
    7
  ],
  "clk0D": [
    7,
    28,
    14,
    7,
    21,
    14
  ],
  "clk0N": [
    7,
    3,
    14
  ],
  "clk1": [
    0
  ],
  "clk2": [
    0
  ],

  "wsprtype": [
    { "slot": 3, "type": 3 },
    { "slot": 4, "type": 3 },
    { "slot": 5, "type": 3 },
  ],
  "temperature": {
    "enable": true,
    "band": 10,
    "clk": 0,
      "temp_correction": -4,
    "timezones": [
      {
        "start": "00:00",
        "end": "24:00",
        "clk": 0,
        "band": 10
      }
    ]
  },
  "wspr": {
    "enabled": true,
    "band_freq": 0,
    "tone_mul": 1,
    "drive_strength": 8
  }
}
*/


#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// Structuren voor configuratie data
struct SystemConfig {
  String version;
  String build;
  uint32_t chipid;
  String hostname;
  int display_off;
};

struct UserConfig {
  String call;
  String prefix;
  String suffix;
  String locator;
  String locator_FR;
  String power;
};

struct Si5351Config {
  bool enable;
  int drive_strength;
  int calibration;
};

struct Timezone {
  String start;
  String end;
  String sunrise;
  String sunset;
  int clk;
  String list;
  String name;
};

struct WsprType {
  int slot;
  int type;
};

struct TemperatureConfig {
  bool enable;
  int band;
  int clk;
  int temp_correction;
  std::vector<Timezone> timezones;
};

struct WsprConfig {
  bool enabled;
  int band_freq;
  int tone_mul;
  int drive_strength;
};

// Globale configuratie variabelen
SystemConfig systemConfig;
UserConfig userConfig;
Si5351Config si5351Config;
TemperatureConfig tempConfig;
WsprConfig wsprConfig;
std::vector<Timezone> timezones;
std::vector<int> clk0, clk0D, clk0N, clk1, clk2;
std::vector<WsprType> wsprTypes;

// Functie om JSON bestand te laden en parsen
bool loadConfiguration(const char* filename) {
  // Open bestand
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println("Failed to open config file");
    return false;
  }

  // Alloceer JSON document (pas grootte aan indien nodig)
  StaticJsonDocument<4096> doc;
  
  // Parse JSON
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.print("Failed to parse config file: ");
    Serial.println(error.c_str());
    return false;
  }

  // Parse System configuratie
  JsonObject system = doc["system"];
  systemConfig.version = system["version"].as<String>();
  systemConfig.build = system["build"].as<String>();
  systemConfig.chipid = system["chipid"];
  systemConfig.hostname = system["hostname"].as<String>();
  systemConfig.display_off = system["display_off"];

  // Parse User configuratie
  JsonObject user = doc["user"];
  userConfig.call = user["call"].as<String>();
  userConfig.prefix = user["prefix"].as<String>();
  userConfig.suffix = user["suffix"].as<String>();
  userConfig.locator = user["locator"].as<String>();
  userConfig.locator_FR = user["locator_FR"].as<String>();
  userConfig.power = user["power"].as<String>();

  // Parse SI5351 configuratie
  JsonObject si5351 = doc["si5351"];
  si5351Config.enable = si5351["enable"];
  si5351Config.drive_strength = si5351["drive_strength"];
  si5351Config.calibration = si5351["calibration"];

  // Parse Timezones
  JsonArray tzArray = doc["timezones"];
  timezones.clear();
  for (JsonObject tz : tzArray) {
    Timezone timezone;
    timezone.start = tz["start"].as<String>();
    timezone.end = tz["end"].as<String>();
    timezone.sunrise = tz["sunrise"].as<String>();
    timezone.sunset = tz["sunset"].as<String>();
    timezone.clk = tz["clk"];
    timezone.list = tz["list"].as<String>();
    timezone.name = tz["name"].as<String>();
    timezones.push_back(timezone);
  }

  // Parse frequentie lijsten
  clk0.clear();
  for (JsonVariant v : doc["clk0"].as<JsonArray>()) {
    clk0.push_back(v.as<int>());
  }
  
  clk0D.clear();
  for (JsonVariant v : doc["clk0D"].as<JsonArray>()) {
    clk0D.push_back(v.as<int>());
  }
  
  clk0N.clear();
  for (JsonVariant v : doc["clk0N"].as<JsonArray>()) {
    clk0N.push_back(v.as<int>());
  }
  
  clk1.clear();
  for (JsonVariant v : doc["clk1"].as<JsonArray>()) {
    clk1.push_back(v.as<int>());
  }
  
  clk2.clear();
  for (JsonVariant v : doc["clk2"].as<JsonArray>()) {
    clk2.push_back(v.as<int>());
  }

  // Parse WSPR types
  JsonArray wsprTypeArray = doc["wsprtype"];
  wsprTypes.clear();
  for (JsonObject wt : wsprTypeArray) {
    WsprType wsprType;
    wsprType.slot = wt["slot"];
    wsprType.type = wt["type"];
    wsprTypes.push_back(wsprType);
  }

  // Parse Temperature configuratie
  JsonObject temp = doc["temperature"];
  tempConfig.enable = temp["enable"];
  tempConfig.band = temp["band"];
  tempConfig.clk = temp["clk"];
  tempConfig.temp_correction = temp["temp_correction"];
  
  tempConfig.timezones.clear();
  for (JsonObject tz : temp["timezones"].as<JsonArray>()) {
    Timezone timezone;
    timezone.start = tz["start"].as<String>();
    timezone.end = tz["end"].as<String>();
    timezone.clk = tz["clk"];
    timezone.list = tz["band"].as<String>();
    tempConfig.timezones.push_back(timezone);
  }

  // Parse WSPR configuratie
  JsonObject wspr = doc["wspr"];
  wsprConfig.enabled = wspr["enabled"];
  wsprConfig.band_freq = wspr["band_freq"];
  wsprConfig.tone_mul = wspr["tone_mul"];
  wsprConfig.drive_strength = wspr["drive_strength"];

  Serial.println("Configuration loaded successfully");
  return true;
}

// Functie om configuratie te printen
void printConfiguration() {
  Serial.println("\n=== SYSTEM CONFIG ===");
  Serial.printf("Version: %s\n", systemConfig.version.c_str());
  Serial.printf("Build: %s\n", systemConfig.build.c_str());
  Serial.printf("Chip ID: %u\n", systemConfig.chipid);
  Serial.printf("Hostname: %s\n", systemConfig.hostname.c_str());
  Serial.printf("Display Off: %d sec\n", systemConfig.display_off);

  Serial.println("\n=== USER CONFIG ===");
  Serial.printf("Call: %s\n", userConfig.call.c_str());
  Serial.printf("Locator: %s\n", userConfig.locator.c_str());
  Serial.printf("Locator FR: %s\n", userConfig.locator_FR.c_str());
  Serial.printf("Power: %s dBm\n", userConfig.power.c_str());

  Serial.println("\n=== SI5351 CONFIG ===");
  Serial.printf("Enabled: %s\n", si5351Config.enable ? "Yes" : "No");
  Serial.printf("Drive Strength: %d mA\n", si5351Config.drive_strength);
  Serial.printf("Calibration: %d\n", si5351Config.calibration);

  Serial.println("\n=== FREQUENCY LISTS ===");
  Serial.print("CLK0: ");
  for (int band : clk0) Serial.printf("%d ", band);
  Serial.println();
  
  Serial.print("CLK0D (Day): ");
  for (int band : clk0D) Serial.printf("%d ", band);
  Serial.println();
  
  Serial.print("CLK0N (Night): ");
  for (int band : clk0N) Serial.printf("%d ", band);
  Serial.println();

  Serial.println("\n=== TIMEZONES ===");
  for (size_t i = 0; i < timezones.size(); i++) {
    Serial.printf("Zone %d: %s (CLK%d, List: %s)\n", 
      i, timezones[i].name.c_str(), timezones[i].clk, timezones[i].list.c_str());
  }

  Serial.println("\n=== TEMPERATURE CONFIG ===");
  Serial.printf("Enabled: %s\n", tempConfig.enable ? "Yes" : "No");
  Serial.printf("Band: %d MHz\n", tempConfig.band);
  Serial.printf("Correction: %d°C\n", tempConfig.temp_correction);

  Serial.println("\n=== WSPR CONFIG ===");
  Serial.printf("Enabled: %s\n", wsprConfig.enabled ? "Yes" : "No");
  Serial.printf("Tone Multiplier: %d\n", wsprConfig.tone_mul);
  Serial.printf("Drive Strength: %d mA\n", wsprConfig.drive_strength);
}

// Helper functie: welke frequentie lijst gebruiken op dit moment?
std::vector<int>* getCurrentFrequencyList(int clk) {
  // Implementeer hier je logica op basis van tijd/zon positie
  // Dit is een simpel voorbeeld
  if (clk == 0) {
    // Check of het dag of nacht is (vereenvoudigd)
    int hour = 12; // Haal huidige uur op van RTC
    if (hour >= 6 && hour < 20) {
      return &clk0D; // Dag lijst
    } else {
      return &clk0N; // Nacht lijst
    }
  } else if (clk == 1) {
    return &clk1;
  } else if (clk == 2) {
    return &clk2;
  }
  return &clk0;
}

// Helper functie: converteer band nummer naar frequentie
unsigned long bandToFrequency(int band) {
  switch (band) {
    case 0: return 0; // Disabled
    case 3: return 136000; // 2200m (630m in Europa)
    case 7: return 7040000; // 40m
    case 10: return 10140200; // 30m
    case 14: return 14097100; // 20m
    case 21: return 21096100; // 15m
    case 28: return 28126100; // 10m
    default: return 0;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nESP32 WSPR Configuration Loader");
  Serial.println("================================");

  // Initialiseer SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // Laad configuratie
  if (loadConfiguration("/config.json")) {
    printConfiguration();
    
    // Voorbeeld gebruik
    Serial.println("\n=== EXAMPLE USAGE ===");
    std::vector<int>* currentList = getCurrentFrequencyList(0);
    Serial.println("Current frequency list for CLK0:");
    for (int band : *currentList) {
      Serial.printf("Band %d MHz -> %lu Hz\n", band, bandToFrequency(band));
    }
  }
}

void loop() {
  // Hier komt je hoofdcode
  delay(1000);
}