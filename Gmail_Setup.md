# 📧 ESP32 Gmail Alert System (First Iteration)

This project demonstrates the simplest way to send an email alert using an ESP32 microcontroller, a standard Wi-Fi connection, and a Gmail account. It uses Google's secure **App Passwords** feature, which is required because older devices (such as microcontrollers) and basic applications do not support modern Google login methods.

---

## Prerequisites

### Hardware
- ESP32 Development Board (e.g., ESP32 DevKitC)
- Micro-USB cable (for power and programming)

### Software & Libraries
- Arduino IDE
- ESP32 Board Support Package installed in the Arduino IDE
- **ESP_Mail_Client** library (Author: Mobizt), installed via the Arduino Library Manager

---

## 🔑 Setup Guide: Creating the Gmail App Password

Because the ESP32 cannot handle modern web-based authentication, you must generate a special **16-character App Password**. This requires enabling **2-Step Verification (2FA)** on your Google Account first.

---

### Step 1: Enable 2-Step Verification (2FA)

This security feature is mandatory before App Passwords become available.

1. Go to your **Google Account → Security** page.
2. Under **How you sign in to Google**, locate **2-Step Verification** and click it.
3. Click **GET STARTED** and follow the prompts to enable 2FA using:
   - SMS text messages, or
   - Google Prompt on your phone.
4. Once complete, confirm that the status shows **On**.

---

### Step 2: Generate the 16-Character App Password

1. Return to **Google Account → Security**.
2. Under **How you sign in to Google**, click **2-Step Verification**.
3. Re-enter your Google password if prompted.
4. Scroll down to find **App passwords** and click it.
5. On the App Passwords page:
   - **Select app**: Mail  
   - **Select device**: Other (Custom name…)  
   - **Name**: `ESP32 Email Alert`
6. Click **Generate**.
7. A yellow box will display a **16-character password**.
   - Copy this password immediately.
   - Store it securely.
   - You will not be able to view it again.
8. Click **Done**.

This App Password will be used in your Arduino sketch instead of your normal Gmail password.

---

## ⚙️ Sketch Configuration

You must replace all placeholder values in the configuration section of the Arduino sketch with your own credentials and App Password.

---

### 1. Update Credentials

Edit the following section in your `.ino` file:

```cpp
// ===================================
// 1. CONFIGURATION: YOUR CUSTOM DETAILS
// ===================================

// Wi-Fi Credentials
#define WIFI_SSID       "YOUR_WIFI_NAME"              // <-- Your Wi-Fi network name
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"          // <-- Your Wi-Fi password

// Gmail Sender Configuration (Using App Password)
// CRITICAL: This MUST be the Gmail account used to generate the App Password.
#define SENDER_EMAIL      "YOUR_SENDER_GMAIL@gmail.com"
#define SENDER_PASSWORD   "YOUR_16_CHAR_APP_PASSWORD" // <-- 16-character App Password (NO SPACES)

// Recipient Configuration
#define RECIPIENT_EMAIL   "YOUR_RECIPIENT_EMAIL@example.com"
