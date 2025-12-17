#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <time.h> 

// ===================================
// 1. CONFIGURATION: YOUR CUSTOM DETAILS
// ===================================

// Wi-Fi Credentials
#define WIFI_SSID       "SKYYRMR7" 
#define WIFI_PASSWORD   "K2xWvDFZkuCh" 

// Gmail Sender Configuration (Using App Password)
#define SMTP_HOST         "smtp.gmail.com"
#define SMTP_PORT         465                 // SSL (recommended)
#define SENDER_EMAIL      "data.monitor.bot@gmail.com"
#define SENDER_PASSWORD   "qznrhhizewhfzrud" // YOUR 16-CHAR APP PASSWORD
#define SENDER_NAME       "15-Min Temp Log"

// Recipient Configuration
#define RECIPIENT_EMAIL   "data.monitor.bot@gmail.com"
#define RECIPIENT_NAME    "Data Receiver"

// NTP Configuration
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;      // UTC/GMT time offset 
const int   daylightOffset_sec = 3600; // 3600 for DST 

// Scheduling Constants
#define LOG_INTERVAL_MINUTES 15 
#define LOG_INTERVAL_SECONDS (LOG_INTERVAL_MINUTES * 60) // 900 seconds

// ===================================
// 2. GLOBAL VARIABLES & FUNCTION PROTOTYPES
// ===================================

SMTPSession smtp;
float currentTempC = 0.0;

// Variables stored in RTC memory survive Deep Sleep
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR time_t nextCaptureTime = 0; 

// Conversion factor for micro seconds to seconds
#define uS_TO_S_FACTOR 1000000 

// Function Prototypes
void smtpCallback(SMTP_Status status);
void sendDataEmail(const char* subject, const char* message);
float readInternalTemperature();
void initTime();
void goToDeepSleep();
void connectWiFi();
time_t calculateNextCaptureTime(time_t currentTime); // New helper function

// ===================================
// 3. SETUP (Connect and Send Once)
// ===================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("\n--- Woke up from Deep Sleep (Timer) ---");
  } else {
    ++bootCount;
    Serial.printf("\n--- System Initial Boot Up #%d ---\n", bootCount);
  }

  // 1. Connect and Sync Time
  connectWiFi();
  
  if (WiFi.status() == WL_CONNECTED) {
    initTime(); // Sync time via NTP
  } else {
    Serial.println("\nFailed to connect to Wi-Fi. Cannot sync time or send email.");
    esp_sleep_enable_timer_wakeup(300 * uS_TO_S_FACTOR); 
    esp_deep_sleep_start();
  }

  // 2. Check Schedule
  time_t currentTime;
  time(&currentTime); 

  if (currentTime >= nextCaptureTime) {
      Serial.println("\n--- STARTING SCHEDULED LOG CYCLE (15 MIN) ---");
      
      // 3. Read Sensor
      currentTempC = readInternalTemperature();
      Serial.printf("Captured Temperature: %.2f °C\n", currentTempC);

      // 4. Send Email
      String subject = "15-Minute Temp Log: ";
      subject += String(currentTempC, 2);
      subject += " C";

      char timeStr[64];
      struct tm timeinfo;
      getLocalTime(&timeinfo);
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
      
      String message = "Log captured at ";
      message += String(timeStr);
      message += ".<br>Internal Chip Temperature: **";
      message += String(currentTempC, 2);
      message += " °C**.";
      
      sendDataEmail(subject.c_str(), message.c_str());
      
      // Give time for email transmission
      delay(8000); 

      // 5. Calculate Next Sleep Cycle
      nextCaptureTime = calculateNextCaptureTime(currentTime);

      struct tm *lt = localtime(&currentTime);
      Serial.printf("Current Time: %s", asctime(lt));
      Serial.printf("Next Log Scheduled for: %s", ctime(&nextCaptureTime));
      
  } else {
    Serial.printf("Waking up only to check time. Next log in %lu seconds.\n", nextCaptureTime - currentTime);
  }
  
  // 6. Go to Sleep
  goToDeepSleep();
}

// ===================================
// 4. LOOP (Empty for Deep Sleep)
// ===================================

void loop() {
  // Empty - the chip sleeps after setup()
}

// ===================================
// 5. FUNCTION IMPLEMENTATIONS
// ===================================

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

// Helper function to calculate the time for the next 15-minute mark
time_t calculateNextCaptureTime(time_t currentTime) {
    struct tm *lt = localtime(&currentTime);
    
    // Total seconds since the last hour (e.g., 20:10:30 -> 630 seconds)
    long currentTotalSeconds = lt->tm_min * 60 + lt->tm_sec; 

    // Time to next 15-minute interval (00, 15, 30, 45)
    // Example: If min=10, 10 / 15 = 0. Next multiple is 15.
    // Example: If min=17, 17 / 15 = 1. Next multiple is 30.
    
    // Calculate the next multiple of the interval (in minutes)
    int nextMinuteInterval = ((lt->tm_min / LOG_INTERVAL_MINUTES) + 1) * LOG_INTERVAL_MINUTES;
    
    // Total seconds until the next 15-minute mark in the hour (e.g., 20:15:00 -> 900 seconds)
    long nextIntervalTotalSeconds = nextMinuteInterval * 60;

    // Seconds to sleep:
    // This handles wrapping around the hour (if current time is 20:50, next is 21:00)
    long secondsToSleep;
    if (nextMinuteInterval >= 60) {
        // Sleep until next hour (3600 seconds)
        secondsToSleep = 3600 - currentTotalSeconds; 
    } else {
        secondsToSleep = nextIntervalTotalSeconds - currentTotalSeconds;
    }
    
    // Safety check for negative sleep time (should not happen if math is perfect)
    if (secondsToSleep <= 0) {
        secondsToSleep = LOG_INTERVAL_SECONDS; // Default to 15 minutes
    }
    
    return currentTime + secondsToSleep;
}

void initTime() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time from NTP. Cannot schedule log.");
    } else {
        Serial.println("Time synchronized successfully.");
        
        // Only set the initial next capture time on the very first boot
        if (bootCount == 1 || nextCaptureTime == 0) {
          time_t now;
          time(&now);
          
          nextCaptureTime = calculateNextCaptureTime(now);
          Serial.printf("Initial next log scheduled for: %s", ctime(&nextCaptureTime));
        }
    }
}

void goToDeepSleep() {
    time_t currentTime;
    time(&currentTime);

    long sleepTimeSec = nextCaptureTime - currentTime;

    if (sleepTimeSec <= 0) { 
        Serial.println("WARNING: Missed log time! Sleeping 60s to retry.");
        sleepTimeSec = 60; 
    }
    
    Serial.printf("Going into deep sleep for %ld seconds...\n", sleepTimeSec);
    esp_sleep_enable_timer_wakeup(sleepTimeSec * uS_TO_S_FACTOR);
    
    // Clean up before sleep
    WiFi.disconnect(true);
    Serial.flush(); 
    
    // 
    esp_deep_sleep_start();
}

float readInternalTemperature() {
    float tempF = temperatureRead(); 
    float tempC = (tempF - 32.0) / 1.8;
    return tempC;
}

void sendDataEmail(const char* subject, const char* message) {
  // --- Configure and Connect for Email ---
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST; 
  session.server.port = SMTP_PORT;
  session.login.email = SENDER_EMAIL;
  session.login.password = SENDER_PASSWORD;

  smtp.callback(smtpCallback);

  if (!smtp.connect(&session)) {
    Serial.print("Error connecting to SMTP server before send: ");
    Serial.println(smtp.errorReason());
    return; // Exit if connection fails
  }

  // --- Prepare and Send Email ---
  SMTP_Message email;
  email.sender.name = SENDER_NAME;
  email.sender.email = SENDER_EMAIL;
  email.subject = subject;
  email.addRecipient(RECIPIENT_NAME, RECIPIENT_EMAIL);
  email.text.content = message;
  email.text.charSet = "UTF-8";
  email.text.transfer_encoding = Content_Transfer_Encoding::enc_base64; 

  if (!MailClient.sendMail(&smtp, &email, true)) {
    Serial.print("Error sending Email: ");
    Serial.println(smtp.errorReason());
  }
}

void smtpCallback(SMTP_Status status) {
  Serial.println("------------------------------------");
  Serial.println("SMTP Status Update:");
  Serial.println(status.info()); 

  if (status.success()) {
    Serial.println("Email Sent Successfully!");
  } else {
    Serial.print("Error Details: ");
    Serial.println(status.info()); 
  }
  Serial.println("------------------------------------");
}
