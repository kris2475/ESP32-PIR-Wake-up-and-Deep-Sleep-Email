# 📧 ESP32 Internal Temperature Email Alert System

A robust IoT project using the **ESP32 Dev Module** to read the chip's internal temperature and send an email alert via Gmail whenever the temperature exceeds a predefined threshold. This solution uses the stable `ESP_Mail_Client` library and Google App Passwords for secure SMTP authentication.

## 🌟 Features

* **Internal Temperature Monitoring:** Reads the ESP32 silicon die temperature.
* **Threshold Alerting:** Only connects to Wi-Fi and sends an email if the temperature is above a specified limit.
* **Non-Blocking Logic:** Uses `millis()` for time management, allowing the ESP32 to continuously monitor the sensor without delays.
* **Secure Communication:** Uses SSL (Port 465) and Gmail App Passwords for robust SMTP communication.

## 🛠️ Prerequisites

### Hardware

* ESP32 Dev Module (Any variant)
* Micro USB cable for power and programming.

### Software and Setup

1. **Arduino IDE:** Installed and configured with the ESP32 Board package.
2. **ESP32 Core:** Ensure you are using a compatible version (older versions like 3.3.2 are supported by reading the internal temperature).
3. **Library Installation:** Install the `ESP_Mail_Client` library. It is highly recommended to use a stable version (e.g., v3.5.0) to avoid compilation issues.
   * **Sketch** > **Include Library** > **Manage Libraries...**
   * Search for "ESP Mail Client" and install the recommended version.
4. **Gmail App Password:** You **must** generate a 16-character App Password from your Google Account settings to use as your `SENDER_PASSWORD`. Using your regular Gmail password will fail.

## 💻 Project Code (`ESP32_Email_Sensor_Readings.ino`)

The core logic has been refactored to use a continuous monitoring loop and non-blocking timing.

```cpp
#include <WiFi.h>
#include <ESP_Mail_Client.h>

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
#define SENDER_NAME       "ESP32 Temp Monitor"

// Recipient Configuration
#define RECIPIENT_EMAIL   "data.monitor.bot@gmail.com"
#define RECIPIENT_NAME    "Data Receiver"

// Alert Thresholds
#define TEMP_THRESHOLD_C  55.0  // Trigger alert if temperature exceeds 55°C
#define SEND_INTERVAL_MS  30000 // Only send a new email every 30 seconds

// ===================================
// 2. GLOBAL VARIABLES & FUNCTION PROTOTYPES
// ===================================

SMTPSession smtp;
float currentTempC = 0.0;
unsigned long lastSendTime = 0;
bool wifiConnected = false;

// Function Prototypes
void smtpCallback(SMTP_Status status);
void sendAlertEmail(const char* subject, const char* message);
float readInternalTemperature(); 
void connectWiFi();

// ===================================
// 3. SETUP (Initial Configuration)
// ===================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- ESP32 Temperature Alert System Initialized ---");
  connectWiFi(); 
}

// ===================================
// 4. MAIN LOOP (Continuous Monitoring)
// ===================================

void loop() {
  // 1. Read the Sensor
  currentTempC = readInternalTemperature();
  Serial.printf("Current Temp: %.2f °C\n", currentTempC);

  // 2. Check Alert Condition and Send Interval
  if (currentTempC > TEMP_THRESHOLD_C) {

    // Check if enough time has passed since the last email alert
    if (millis() - lastSendTime >= SEND_INTERVAL_MS) {

      Serial.println(">>> ALERT: Temperature Exceeded Threshold! <<<");

      // Ensure WiFi is connected before attempting to send
      if (WiFi.status() != WL_CONNECTED) {
          connectWiFi();
      }

      // --- Configure and Send Email ---
      ESP_Mail_Session session;
      session.server.host_name = SMTP_HOST; 
      session.server.port = SMTP_PORT;
      session.login.email = SENDER_EMAIL;
      session.login.password = SENDER_PASSWORD;

      smtp.callback(smtpCallback);

      if (smtp.connect(&session)) { 
          String subject = "CRITICAL ALERT: Overheating @ ";
          subject += String(currentTempC, 2);
          subject += " C";

          String message = "The ESP32 chip has reached a critical temperature of **";
          message += String(currentTempC, 2);
          message += " °C**. Threshold is ";
          message += String(TEMP_THRESHOLD_C, 2);
          message += " °C. Immediate attention required.";

          sendAlertEmail(subject.c_str(), message.c_str());

          lastSendTime = millis(); 
      } else {
          Serial.print("Error connecting to SMTP server: ");
          Serial.println(smtp.errorReason());
      }

    } else {
        Serial.println("Alert Condition Met, but waiting for send interval.");
    }
  }

  delay(1000); 
}

// ===================================
// 5. HELPER FUNCTIONS
// ===================================

void connectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
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
      wifiConnected = true;
    } else {
      Serial.println("\nFailed to connect to Wi-Fi.");
      wifiConnected = false;
    }
  }
}

float readInternalTemperature() {
    float tempF = temperatureRead(); 
    float tempC = (tempF - 32.0) / 1.8;
    return tempC;
}

void sendAlertEmail(const char* subject, const char* message) {
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
```

## ▶️ Getting Started

1. Paste the code into your Arduino IDE.
2. Update the `#define` configuration values with your Wi-Fi credentials and Gmail App Password.
3. Adjust `TEMP_THRESHOLD_C` and `SEND_INTERVAL_MS` as needed.
4. Select **ESP32 Dev Module** and upload the sketch.
5. Open the Serial Monitor at **115200 baud**.

When the internal temperature exceeds **55.0°C**, the alert email will be sent.

## ⚠️ Important Note on Internal Temperature

The `temperatureRead()` function measures the temperature of the ESP32 silicon die, not ambient air temperature. This value will be higher than room temperature, especially during Wi-Fi usage and high CPU load.

Typical idle readings may range from **40–50°C** depending on operating conditions. This metric is best suited for monitoring ESP32 operational health rather than environmental temperature.
