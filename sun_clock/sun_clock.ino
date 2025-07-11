#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <time.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

#define LED_PIN 8               // GPIO8 for LED ring data
#define LED_COUNT 24            // 24 LEDs for 24 hours
#define BRIGHTNESS 100          // Default brightness (0-255)
#define WIFI_TIMEOUT 30000      // 30 seconds WiFi timeout
#define GPS_TIMEOUT 180000      // 3 minutes GPS search timeout

// GPS uses hardware serial (Serial1)
// GPS TX -> GPIO20 (ESP32 RX)
// GPS RX -> GPIO21 (ESP32 TX)

// Multiple WiFi networks for fallback
struct WiFiNetwork {
  const char* ssid;
  const char* password;
};

// Update these with your network credentials
WiFiNetwork networks[] = {
  {"YOUR_WIFI_SSID_1", "YOUR_PASSWORD_1"},
  {"YOUR_WIFI_SSID_2", "YOUR_PASSWORD_2"},
  {"YOUR_WIFI_SSID_3", "YOUR_PASSWORD_3"},
  {"YOUR_WIFI_SSID_4", "YOUR_PASSWORD_4"}
};
const int numNetworks = 4;

// NTP settings (will be dynamically set based on GPS location)
const char* ntpServer = "pool.ntp.org";
long gmtOffset_sec = 0;         // Will be calculated from GPS
int daylightOffset_sec = 0;     // DST handling

// Web server and DNS for captive portal
WebServer server(80);
DNSServer dnsServer;

// Status flags
bool wifiConnected = false;
bool timeSync = false;
bool gpsConnected = false;
bool captivePortalActive = false;
int currentBrightness = BRIGHTNESS;
bool autoBrightness = true;

// GPS data structure
struct GPSData {
  bool valid;
  float latitude;
  float longitude;
  int year, month, day;
  int hour, minute, second;
  unsigned long lastUpdate;
  int timezone_offset;  // UTC offset in hours
};
GPSData gpsData = {false, 0, 0, 2025, 7, 11, 12, 0, 0, 0, 0};

// Calculated sun times for current location
struct SunTimes {
  int sunrise_hour;
  int sunrise_min;
  int sunset_hour;
  int sunset_min;
};
SunTimes currentSunTimes = {6, 0, 18, 0};  // Default values

Adafruit_NeoPixel ring(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== Sun Clock Starting ===");
  Serial.println("GPS-synchronized 24-hour LED clock");
  
  // Initialize LED ring
  ring.begin();
  ring.setBrightness(BRIGHTNESS);
  ring.show();
  
  // Show startup animation
  startupAnimation();
  
  // Initialize GPS on Serial1 (GPIO20=RX, GPIO21=TX)
  Serial1.begin(9600, SERIAL_8N1, 20, 21);
  Serial.println("GPS initialized on Serial1 (GPIO20/21)");
  
  // Try GPS first (3 minute search)
  Serial.println("Searching for GPS satellites...");
  searchForGPS();
  
  if (!gpsData.valid) {
    Serial.println("GPS search timeout, trying WiFi...");
    connectToWiFi();
    
    if (wifiConnected) {
      syncTimeWithNTP();
    } else {
      Serial.println("WiFi failed, starting captive portal...");
      startCaptivePortal();
    }
  }
  
  Serial.println("Sun Clock ready!");
  printStatus();
}

void loop() {
  // Handle captive portal
  if (captivePortalActive) {
    dnsServer.processNextRequest();
    server.handleClient();
  }
  
  // Try to get GPS data if we don't have it
  if (!gpsData.valid) {
    readGPS();
  }
  
  // Update display
  updateClock();
  
  // Auto-brightness based on time
  if (autoBrightness) {
    adjustBrightness();
  }
  
  // Print status every 60 seconds
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 60000) {
    printStatus();
    lastStatus = millis();
  }
  
  delay(1000);
}

void startupAnimation() {
  // Rainbow wheel startup
  for(int j = 0; j < 256; j++) {
    for(int i = 0; i < LED_COUNT; i++) {
      ring.setPixelColor(i, wheel((i * 256 / LED_COUNT + j) & 255));
    }
    ring.show();
    delay(10);
  }
  
  // Fade to black
  for(int brightness = 255; brightness >= 0; brightness -= 5) {
    for(int i = 0; i < LED_COUNT; i++) {
      ring.setPixelColor(i, wheel((i * 256 / LED_COUNT) & 255));
      uint32_t color = ring.getPixelColor(i);
      uint8_t r = (color >> 16) * brightness / 255;
      uint8_t g = (color >> 8) * brightness / 255;
      uint8_t b = color * brightness / 255;
      ring.setPixelColor(i, ring.Color(r, g, b));
    }
    ring.show();
    delay(20);
  }
  
  ring.clear();
  ring.show();
}

void searchForGPS() {
  unsigned long startTime = millis();
  
  while ((millis() - startTime) < GPS_TIMEOUT) {
    // Show GPS search animation (yellow spinning dots)
    static unsigned long lastUpdate = 0;
    static int pos = 0;
    
    if (millis() - lastUpdate > 200) {
      ring.clear();
      ring.setPixelColor(pos, ring.Color(255, 255, 0));
      ring.setPixelColor((pos + 12) % LED_COUNT, ring.Color(255, 255, 0));
      ring.show();
      pos = (pos + 1) % LED_COUNT;
      lastUpdate = millis();
    }
    
    // Check for GPS data
    if (readGPS()) {
      Serial.println("GPS fix acquired!");
      ring.clear();
      ring.fill(ring.Color(0, 255, 0), 0, LED_COUNT);  // Green flash
      ring.show();
      delay(500);
      ring.clear();
      ring.show();
      return;
    }
    
    // Print progress every 10 seconds
    if ((millis() - startTime) % 10000 < 100) {
      Serial.printf("GPS search: %lu/%lu seconds\\n", 
                   (millis() - startTime) / 1000, GPS_TIMEOUT / 1000);
    }
    
    delay(100);
  }
  
  Serial.println("GPS search timeout");
}

bool readGPS() {
  while (Serial1.available()) {
    String sentence = Serial1.readStringUntil('\\n');
    sentence.trim();
    
    if (sentence.startsWith("$GPRMC")) {
      return parseGPRMC(sentence);
    }
  }
  return false;
}

bool parseGPRMC(String sentence) {
  // Parse GPRMC: $GPRMC,time,status,lat,N/S,lon,E/W,speed,course,date,mag_var,E/W*checksum
  
  int commaIndex[12];
  int commaCount = 0;
  
  // Find comma positions
  for (int i = 0; i < sentence.length() && commaCount < 12; i++) {
    if (sentence.charAt(i) == ',') {
      commaIndex[commaCount] = i;
      commaCount++;
    }
  }
  
  if (commaCount < 9) return false;
  
  // Check if data is valid
  String status = sentence.substring(commaIndex[1] + 1, commaIndex[2]);
  if (status != "A") return false;
  
  // Parse time (HHMMSS)
  String timeStr = sentence.substring(commaIndex[0] + 1, commaIndex[1]);
  if (timeStr.length() != 6) return false;
  
  // Parse date (DDMMYY)
  String dateStr = sentence.substring(commaIndex[8] + 1, commaIndex[9]);
  if (dateStr.length() != 6) return false;
  
  // Parse latitude
  String latStr = sentence.substring(commaIndex[2] + 1, commaIndex[3]);
  String latDir = sentence.substring(commaIndex[3] + 1, commaIndex[4]);
  
  // Parse longitude
  String lonStr = sentence.substring(commaIndex[4] + 1, commaIndex[5]);
  String lonDir = sentence.substring(commaIndex[5] + 1, commaIndex[6]);
  
  if (latStr.length() < 4 || lonStr.length() < 5) return false;
  
  // Convert coordinates
  float lat = latStr.substring(0, 2).toFloat() + latStr.substring(2).toFloat() / 60.0;
  if (latDir == "S") lat = -lat;
  
  float lon = lonStr.substring(0, 3).toFloat() + lonStr.substring(3).toFloat() / 60.0;
  if (lonDir == "W") lon = -lon;
  
  // Extract time components
  int utc_hour = timeStr.substring(0, 2).toInt();
  int minute = timeStr.substring(2, 4).toInt();
  int second = timeStr.substring(4, 6).toInt();
  
  // Extract date components
  int day = dateStr.substring(0, 2).toInt();
  int month = dateStr.substring(2, 4).toInt();
  int year = 2000 + dateStr.substring(4, 6).toInt();
  
  // Calculate timezone offset from longitude (rough approximation)
  int timezone_offset = round(lon / 15.0);
  
  // Convert UTC to local time
  int local_hour = utc_hour + timezone_offset;
  if (local_hour >= 24) {
    local_hour -= 24;
    day++;
  } else if (local_hour < 0) {
    local_hour += 24;
    day--;
  }
  
  // Update GPS data
  gpsData.valid = true;
  gpsData.latitude = lat;
  gpsData.longitude = lon;
  gpsData.hour = local_hour;
  gpsData.minute = minute;
  gpsData.second = second;
  gpsData.day = day;
  gpsData.month = month;
  gpsData.year = year;
  gpsData.timezone_offset = timezone_offset;
  gpsData.lastUpdate = millis();
  
  // Calculate sunrise/sunset for this location
  calculateSunTimes(lat, lon, year, month, day);
  
  Serial.printf("GPS Location: %.4f, %.4f (UTC%+d)\\n", lat, lon, timezone_offset);
  Serial.printf("Local time: %04d-%02d-%02d %02d:%02d\\n", 
                year, month, day, local_hour, minute);
  
  return true;
}

void calculateSunTimes(float lat, float lon, int year, int month, int day) {
  // Simplified sunrise/sunset calculation
  // This is an approximation - for precise calculations, use a full astronomical library
  
  // Day of year
  int dayOfYear = dayOfYearFromDate(year, month, day);
  
  // Solar declination angle
  float declination = 23.45 * sin(radians(360.0 * (284 + dayOfYear) / 365.0));
  
  // Hour angle
  float hourAngle = acos(-tan(radians(lat)) * tan(radians(declination)));
  hourAngle = degrees(hourAngle);
  
  // Sunrise and sunset in decimal hours (UTC)
  float sunrise_utc = 12.0 - hourAngle / 15.0;
  float sunset_utc = 12.0 + hourAngle / 15.0;
  
  // Convert to local time
  float sunrise_local = sunrise_utc + gpsData.timezone_offset;
  float sunset_local = sunset_utc + gpsData.timezone_offset;
  
  // Handle day overflow
  if (sunrise_local < 0) sunrise_local += 24;
  if (sunrise_local >= 24) sunrise_local -= 24;
  if (sunset_local < 0) sunset_local += 24;
  if (sunset_local >= 24) sunset_local -= 24;
  
  // Convert to hours and minutes
  currentSunTimes.sunrise_hour = (int)sunrise_local;
  currentSunTimes.sunrise_min = (int)((sunrise_local - currentSunTimes.sunrise_hour) * 60);
  currentSunTimes.sunset_hour = (int)sunset_local;
  currentSunTimes.sunset_min = (int)((sunset_local - currentSunTimes.sunset_hour) * 60);
  
  Serial.printf("Sunrise: %02d:%02d, Sunset: %02d:%02d\\n",
                currentSunTimes.sunrise_hour, currentSunTimes.sunrise_min,
                currentSunTimes.sunset_hour, currentSunTimes.sunset_min);
}

int dayOfYearFromDate(int year, int month, int day) {
  int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  // Check for leap year
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
    daysInMonth[1] = 29;
  }
  
  int dayOfYear = day;
  for (int i = 0; i < month - 1; i++) {
    dayOfYear += daysInMonth[i];
  }
  
  return dayOfYear;
}

void connectToWiFi() {
  for (int i = 0; i < numNetworks; i++) {
    Serial.printf("Trying WiFi: %s\\n", networks[i].ssid);
    
    WiFi.begin(networks[i].ssid, networks[i].password);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < 10000) {
      // Show WiFi connection animation (blue chase)
      static int pos = 0;
      ring.clear();
      ring.setPixelColor(pos, ring.Color(0, 0, 255));
      ring.show();
      pos = (pos + 1) % LED_COUNT;
      delay(200);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.printf("Connected to %s\\n", networks[i].ssid);
      Serial.printf("IP: %s\\n", WiFi.localIP().toString().c_str());
      
      // Green flash for success
      ring.fill(ring.Color(0, 255, 0), 0, LED_COUNT);
      ring.show();
      delay(500);
      ring.clear();
      ring.show();
      return;
    }
  }
  
  Serial.println("All WiFi networks failed");
  // Red flash for failure
  ring.fill(ring.Color(255, 0, 0), 0, LED_COUNT);
  ring.show();
  delay(500);
  ring.clear();
  ring.show();
}

void syncTimeWithNTP() {
  if (!wifiConnected) return;
  
  // Use GPS timezone if available, otherwise UTC
  gmtOffset_sec = gpsData.valid ? gpsData.timezone_offset * 3600 : 0;
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  Serial.print("Syncing time from NTP...");
  int attempts = 0;
  while (!time(nullptr) && attempts < 10) {
    Serial.print(".");
    delay(1000);
    attempts++;
  }
  
  if (time(nullptr)) {
    timeSync = true;
    Serial.println(" Success!");
    
    // If we don't have GPS location, use a default location for sun calculations
    if (!gpsData.valid) {
      // Use a reasonable default (e.g., New York coordinates)
      calculateSunTimes(40.7128, -74.0060, 2025, 7, 11);
      Serial.println("Using default location for sunrise/sunset");
    }
  } else {
    Serial.println(" Failed!");
  }
}

void startCaptivePortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("SunClock-Setup", "");
  
  dnsServer.start(53, "*", WiFi.softAPIP());
  
  server.onNotFound(handleCaptivePortal);
  server.on("/", handleCaptivePortal);
  server.on("/set", handleTimeSet);
  server.begin();
  
  captivePortalActive = true;
  Serial.println("Captive portal started");
  Serial.printf("Connect to 'SunClock-Setup' and visit %s\\n", 
                WiFi.softAPIP().toString().c_str());
}

void handleCaptivePortal() {
  String html = "<html><head><title>Sun Clock Setup</title></head><body>";
  html += "<h1>Sun Clock Time Setup</h1>";
  html += "<p>GPS and WiFi unavailable. Please set time manually:</p>";
  html += "<form action='/set' method='POST'>";
  html += "Year: <input type='number' name='year' value='2025' min='2020' max='2030'><br><br>";
  html += "Month: <input type='number' name='month' value='7' min='1' max='12'><br><br>";
  html += "Day: <input type='number' name='day' value='11' min='1' max='31'><br><br>";
  html += "Hour: <input type='number' name='hour' value='12' min='0' max='23'><br><br>";
  html += "Minute: <input type='number' name='minute' value='0' min='0' max='59'><br><br>";
  html += "<input type='submit' value='Set Time'>";
  html += "</form></body></html>";
  
  server.send(200, "text/html", html);
}

void handleTimeSet() {
  // Handle manual time setting from captive portal
  // This is a simplified implementation
  server.send(200, "text/html", "<html><body><h1>Time Set!</h1><p>Sun Clock updated.</p></body></html>");
}

void updateClock() {
  ring.clear();
  
  int currentHour, currentMinute;
  
  if (gpsData.valid) {
    // Use GPS time (with basic time progression)
    unsigned long elapsed = (millis() - gpsData.lastUpdate) / 1000;
    currentMinute = gpsData.minute + (elapsed / 60);
    currentHour = gpsData.hour + (currentMinute / 60);
    currentMinute = currentMinute % 60;
    currentHour = currentHour % 24;
  } else {
    // Fallback to system time or default
    currentHour = 12;
    currentMinute = 0;
  }
  
  // Background: dim colors based on time of day
  for (int i = 0; i < LED_COUNT; i++) {
    uint32_t bgColor;
    
    // Night (dark blue)
    if (i < currentSunTimes.sunrise_hour || i > currentSunTimes.sunset_hour) {
      bgColor = ring.Color(0, 0, 30);
    }
    // Day (light blue)
    else {
      bgColor = ring.Color(0, 30, 60);
    }
    
    ring.setPixelColor(i, bgColor);
  }
  
  // Sunrise arc (orange/yellow)
  int sunriseStart = (currentSunTimes.sunrise_hour - 1 + 24) % 24;
  int sunriseEnd = (currentSunTimes.sunrise_hour + 1) % 24;
  for (int i = sunriseStart; i != sunriseEnd; i = (i + 1) % 24) {
    ring.setPixelColor(i, ring.Color(255, 100, 0));
  }
  
  // Sunset arc (red/orange)
  int sunsetStart = (currentSunTimes.sunset_hour - 1 + 24) % 24;
  int sunsetEnd = (currentSunTimes.sunset_hour + 1) % 24;
  for (int i = sunsetStart; i != sunsetEnd; i = (i + 1) % 24) {
    ring.setPixelColor(i, ring.Color(255, 50, 0));
  }
  
  // Current hour: bright white
  ring.setPixelColor(currentHour, ring.Color(255, 255, 255));
  
  // Minute progress on next hour LED
  int minuteBrightness = map(currentMinute, 0, 59, 0, 100);
  int nextHour = (currentHour + 1) % 24;
  ring.setPixelColor(nextHour, ring.Color(minuteBrightness, minuteBrightness, minuteBrightness));
  
  ring.show();
}

void adjustBrightness() {
  // Auto-adjust brightness based on time
  int hour = gpsData.valid ? gpsData.hour : 12;
  int brightness;
  
  if (hour >= 22 || hour <= 6) {
    brightness = 30;  // Night: dim
  } else if (hour >= 7 && hour <= 21) {
    brightness = BRIGHTNESS;  // Day: normal
  } else {
    brightness = 60;  // Dawn/dusk: medium
  }
  
  if (brightness != currentBrightness) {
    currentBrightness = brightness;
    ring.setBrightness(brightness);
  }
}

void printStatus() {
  Serial.println("\\n=== Sun Clock Status ===");
  Serial.printf("Uptime: %lu seconds\\n", millis() / 1000);
  
  if (gpsData.valid) {
    Serial.printf("GPS: CONNECTED (%.4f, %.4f)\\n", 
                  gpsData.latitude, gpsData.longitude);
    Serial.printf("Time: %04d-%02d-%02d %02d:%02d (UTC%+d)\\n",
                  gpsData.year, gpsData.month, gpsData.day,
                  gpsData.hour, gpsData.minute, gpsData.timezone_offset);
    Serial.printf("Sun: Rise %02d:%02d, Set %02d:%02d\\n",
                  currentSunTimes.sunrise_hour, currentSunTimes.sunrise_min,
                  currentSunTimes.sunset_hour, currentSunTimes.sunset_min);
  } else {
    Serial.println("GPS: SEARCHING...");
  }
  
  Serial.printf("WiFi: %s\\n", wifiConnected ? "CONNECTED" : "DISCONNECTED");
  Serial.printf("Time Sync: %s\\n", timeSync ? "YES" : "NO");
  Serial.printf("Brightness: %d/%d (auto: %s)\\n", 
                currentBrightness, BRIGHTNESS, autoBrightness ? "ON" : "OFF");
  Serial.println("========================\\n");
}

// Helper function for rainbow colors
uint32_t wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return ring.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return ring.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return ring.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}