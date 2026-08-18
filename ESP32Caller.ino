#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <Melopero_RV3028.h>

// Global objects
WebServer server(80);  // Web Server
Melopero_RV3028 rtc;   // Real Time Clock
Preferences prefs;     // Set up Preferences (for storing information in flash storage to survive power cycles)

// Global Settings
String startTime01;
uint16_t duration01;
bool speakerOn = false;

// Set up WiFi
const char* ssid = "Caller01";
const char* password = "password";

// Set up pins
const int buttonPin = 3;

// Create struct for the current time
struct CurrentTime {
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint16_t minutesSinceMidnight;
  String formatted;
};

void setup() {

  // Start Serial
  Serial.begin(115200);
  delay(1000);

  // Initialise RTC clock
  Wire.begin(0, 1);
  rtc.initI2C();
  rtc.set24HourMode();

  // Start LittleFS so web files can be read
  if (!LittleFS.begin()) {
    Serial.println("LittleFS failed to mount");
    return;
  }

  // Load stored time and duration settings
  loadSettings();

  // Configure WiFi, server, load web files
  prepareWeb();

  // Tell server what to do when Save button is pressed
  enableSaving();

  // Get latest time and duration settings
  enableSettingsEndpoint();
  // Create endpoint for RTC time
  enableStatusEndpoint();
  enableRTCSettings();

  // Start web server
  server.begin();
  printTimestamp();
  Serial.println("Web server started.");
  Serial.println();

  // Prepare pins
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {

  server.handleClient();

  checkSchedule();

  // Button logic
  if (digitalRead(buttonPin) == LOW) {
    Serial.println("Button pressed!");
    startConfigurationMode();

    // Wait until button is released
    while (digitalRead(buttonPin) == LOW) {
      delay(50);
    }
  }

  delay(10);
}

void startConfigurationMode() {
  Serial.println("Configuration Mode activated.");
}

void loadSettings() {

  // Open the flash storage for saving time and duration settings, in read/write mode
  prefs.begin("settings", false);

  // Load default settings
  startTime01 = prefs.getString("time01", "09:00");
  duration01 = prefs.getUShort("duration01", 60);

  // Convert duration into hours and minutes
  uint8_t hours = duration01 / 60;
  uint8_t minutes = duration01 % 60;

  Serial.println();
  printTimestamp();
  Serial.println("Current Settings:");
  Serial.print("Start Time 01: ");
  Serial.println(startTime01);

  Serial.print("Hours: ");
  Serial.println(hours);

  Serial.print("Minutes: ");
  Serial.println(minutes);
  Serial.println();
}

void prepareWeb() {

  // Start WiFi access point
  printTimestamp();
  Serial.println("Starting WiFi...");
  WiFi.softAP(ssid, password);
  Serial.println("WiFi started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Define what happens when someone visits /
  server.serveStatic("/", LittleFS, "/index.html");
  server.serveStatic("/style.css", LittleFS, "/style.css");
  server.serveStatic("/script.js", LittleFS, "/script.js");

  // Check website files have loaded:
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.print(file.name());
    Serial.println(" loaded successfully.");
    file = root.openNextFile();
  }
  Serial.println();
}

void enableSaving() {
  // Tell server what to do when Save button is pressed

  server.on("/save", []() {
    // Create a variable for each setting
    startTime01 = server.arg("time01");
    duration01 = server.arg("duration01").toInt();

    // Ensure the submitted duration can't be over 24 hours
    if (duration01 > 1440) {
      duration01 = 1440;
    }

    // Save settings in flash storage
    prefs.putString("time01", startTime01);
    prefs.putUShort("duration01", duration01);

    // Print arguments received and their values
    Serial.println("Settings received:");
    for (int i = 0; i < server.args(); i++) {
      Serial.print(server.argName(i));
      Serial.print(" = ");
      Serial.println(server.arg(i));
    }
    Serial.println();

    Serial.println("Saved! Checking flash...");
    Serial.print("Stored time: ");
    Serial.println(prefs.getString("time01", "missing"));
    Serial.print("Stored duration: ");
    Serial.println(prefs.getUShort("duration01", 0));
    Serial.println();
    server.send(200, "text/plain", "Saved");
  });
}

void enableSettingsEndpoint() {
  // When visiting https://192.168.4.1/settings, load the latest time and duration settings

  server.on("/settings", []() {
    String time01 = prefs.getString("time01", "09:00");
    uint16_t duration01 = prefs.getUShort("duration01", 60);

    StaticJsonDocument<200> doc;

    doc["time01"] = time01;
    doc["duration01"] = duration01;

    String response;

    serializeJson(doc, response);

    server.send(200, "application/json", response);
  });
}

void enableStatusEndpoint() {

  server.on("/status", []() {
    CurrentTime now = getCurrentTime();

    StaticJsonDocument<200> doc;

    doc["time"] = now.formatted;
    doc["minutes"] = now.minutesSinceMidnight;

    String response;

    serializeJson(doc, response);

    server.send(200, "application/json", response);
  });
}

void enableRTCSettings() {

  server.on("/setRTC", []() {
    int year = server.arg("year").toInt();
    int month = server.arg("month").toInt();
    int weekday = server.arg("weekday").toInt();
    int day = server.arg("day").toInt();
    int hour = server.arg("hour").toInt();
    int minute = server.arg("minute").toInt();
    int second = server.arg("second").toInt();

    rtc.setTime(year, month, weekday, day, hour, minute, second);
    CurrentTime now = getCurrentTime();

    Serial.print("RTC updated: ");
    Serial.println(now.formatted);

    server.send(200, "text/plain", "RTC update");
  });
}

CurrentTime getCurrentTime() {
  // Get the current time from the RTC

  CurrentTime t;

  t.hour = rtc.getHour();
  t.minute = rtc.getMinute();
  t.second = rtc.getSecond();

  t.minutesSinceMidnight = t.hour * 60 + t.minute;

  t.formatted = "";

  if (t.hour < 10) t.formatted += "0";
  t.formatted += String(t.hour);
  t.formatted += ":";

  if (t.minute < 10) t.formatted += "0";
  t.formatted += String(t.minute);
  t.formatted += ":";

  if (t.second < 10) t.formatted += "0";
  t.formatted += String(t.second);

  return t;
}

void printTimestamp() {
  // Format a timestamp for debugging purposes

  CurrentTime now = getCurrentTime();
  Serial.print("==========(");
  Serial.print(now.formatted);
  Serial.println(")==========");
}

uint16_t timeStringToMinutes(const String& time) {
// Used to convert start times from strings (e.g. "09:00")
// into minutes since midnight as an interger (e.g. 540)

  int colon = time.indexOf(':');

  uint8_t hours = time.substring(0, colon).toInt();
  uint8_t minutes = time.substring(colon + 1).toInt();

  return (hours * 60) + minutes;
}

void checkSchedule() {
  // Logic which determines if the speaker should be playing or silent

  CurrentTime now = getCurrentTime();

  uint16_t start = timeStringToMinutes(startTime01);
  uint16_t end = start + duration01;

  bool shouldBeOn;

  // For situations where caller is running 24 hours a day
  if (duration01 >= 1440) {

    shouldBeOn = true;

  }
  // For schedules which don't cross midnight
  else if (end <= 1440) {

    shouldBeOn = 
      now.minutesSinceMidnight >= start &&
      now.minutesSinceMidnight < end;

  }
  // For schedules which cross midnight
  else {

    uint16_t endNextDay = end - 1440;

    shouldBeOn = 
      now.minutesSinceMidnight >= start ||
      now.minutesSinceMidnight < endNextDay;

  }

  // Only react when the state changes
  if (shouldBeOn && !speakerOn) {

    speakerOn = true;
    printTimestamp();
    Serial.println("Speaker ON");

  }
  else if (!shouldBeOn && speakerOn) {

    speakerOn = false;
    printTimestamp();
    Serial.println("Speaker OFF");
  }

}