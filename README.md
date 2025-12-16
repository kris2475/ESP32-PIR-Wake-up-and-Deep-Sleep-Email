# 📸 ESP32-CAM Time-Triggered Photo Email System

This project transforms an ESP32-CAM module into a power-efficient, self-contained time-lapse camera that captures and emails a JPEG photo at precise one-hour intervals. It utilizes Deep Sleep for power saving and the Network Time Protocol (NTP) for accurate scheduling.

---

## 📌 Features

- **Time-Triggered Capture**  
  Uses NTP to accurately schedule photo captures near the top of every hour.

- **Deep Sleep**  
  Minimizes power consumption by keeping the device asleep between captures.

- **Email Attachment**  
  Connects to Gmail's SMTP server to send the captured JPEG as a file attachment.

---

## Prerequisites

### Hardware
- ESP32-CAM (AI-Thinker model)
- FTDI Programmer (for flashing the ESP32-CAM)
- Micro-USB Cable

### Software & Libraries
- Arduino IDE
- ESP32 Board Support Package (must be installed for ESP32-CAM support)
- **ESP_Mail_Client Library**  
  - Author: Mobizt  
  - Version: 3.4.24 or later  
  - Installed via the Arduino Library Manager

> **Note:** The ESP32-CAM requires specific configuration during flashing (GPIO 0 connected to GND).

---

## 🔑 Setup Guide: Google App Password

This system uses a secure **Google App Password** for Gmail authentication, as the ESP32 cannot use modern OAuth login methods.

---

### 1. Enable 2-Step Verification (2FA)

You must enable 2FA on your Google Account before generating an App Password.

1. Go to your **Google Account → Security** page.
2. Enable **2-Step Verification**.

---

### 2. Generate the App Password

1. Navigate back to the **Security** page.
2. Click **2-Step Verification → App passwords**.
3. Set the selections to:
   - **App**: Mail  
   - **Device**: Other (Custom name…)
4. Click **Generate**.
5. Copy the **16-character code** (e.g., `abcd efgh ijkl mnop`).

This code is your `SENDER_PASSWORD`.

---

## ⚙️ Sketch Configuration

---

### 1. Arduino IDE Board Settings

Selecting the correct board is critical for camera operation.

1. In the Arduino IDE, go to:  
   **Tools → Board → AI-Thinker ESP32-CAM**
2. Set **Upload Speed** to `921600`.
3. Ensure the correct **Port** is selected.

---

### 2. Code Credentials

Update the following constants in the Arduino sketch with your credentials and localization settings:

```cpp
// Wi-Fi Credentials
#define WIFI_SSID       "YOUR_WIFI_NETWORK_NAME"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// Gmail Sender Configuration
// The SENDER_EMAIL MUST match the account used to generate the App Password.
#define SENDER_EMAIL      "YOUR_SENDER_GMAIL@gmail.com"
#define SENDER_PASSWORD   "YOUR16CHARAPPPASSWORD"      // <-- App Password (NO SPACES!)

// Recipient Configuration
#define RECIPIENT_EMAIL   "YOUR_RECIPIENT_EMAIL@example.com"

// NTP Server Configuration (Timezone adjustment)
const long  gmtOffset_sec = 0;      // 3600 for GMT+1, -18000 for EST, etc.
const int   daylightOffset_sec = 0; // 3600 if Daylight Savings Time is active
```

---

## 📐 Camera Pinout (AI-Thinker)

The following pin definitions are hardcoded for the popular AI-Thinker ESP32-CAM module. Do **not** modify these unless you are using a different camera board.

| Function        | GPIO Pin | Note                                                                 |
|-----------------|----------|----------------------------------------------------------------------|
| PIR / Wake      | N/A      | Timer-based wake-up used in this iteration                            |
| Deep Sleep      | Internal Timer | Sleeps for a calculated duration                                |
| Flash LED       | GPIO 4   | Optional: Uncomment code in `capturePhoto()` to enable flash usage   |

---

## 💡 How the System Works (The Sleep Cycle)

1. **Sleep**  
   The ESP32-CAM enters Deep Sleep for a calculated duration (initially up to 1 hour).

2. **Wake**  
   The internal RTC timer wakes the device.

3. **Check Time**  
   The ESP32-CAM connects to Wi-Fi and retrieves the current time via NTP.

4. **Capture Decision**
   - If the current time is near the top of the hour (e.g., 1:00 PM–1:05 PM):
     - Capture photo
     - Email JPEG
     - Enter Deep Sleep for a full hour
   - If the time is not near the top of the hour (e.g., 1:30 PM):
     - Calculate remaining time
     - Sleep until the next top-of-the-hour window

5. **Repeat**  
   This cycle repeats continuously, maintaining accurate scheduling while maximizing power efficiency.
