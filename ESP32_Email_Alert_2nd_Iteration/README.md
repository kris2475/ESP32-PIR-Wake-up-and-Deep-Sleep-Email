# 📝 Summary of Current Sensor Work (Internal Temperature Email Alert)

The primary goal of the **current sensor work** was to successfully read internal data from the ESP32 Dev Module and transmit that data via email using the established Wi-Fi and SMTP communication channels.

---

## 1. Sensor Selection and Challenge

### Initial Sensor Goal
We initially attempted to use the ESP32’s built-in **Hall Effect Sensor** via the `hallRead()` function.

### Problem Encountered
The compiler consistently failed with errors indicating that the required functions (`hallRead()` and even the low-level `hall_sensor_read()`) were **not declared in the current scope**.

This issue was caused by an incompatibility between the sketch and the older **ESP32 Arduino Core version (3.3.2)**, which lacks the necessary header declarations for these functions.

### Resolution
To resolve this, we pivoted to a more robust and consistently supported internal sensor:
- **Internal Temperature Sensor** accessed via the `temperatureRead()` function

This sensor is more reliably implemented across multiple ESP32 core versions.

---

## 2. Sensor Implementation

### Function Used
The core function for data acquisition is `readInternalTemperature()`, which utilizes the ESP32 SDK’s `temperatureRead()` function.

```cpp
float readInternalTemperature() {
    // Reads value in Fahrenheit from the silicon die
    float tempF = temperatureRead(); 
    // Converts to Celsius
    float tempC = (tempF - 32.0) / 1.8;
    return tempC;
}
Data Type
The sensor reports the temperature of the silicon die (the ESP32 chip itself) in degrees Celsius (°C).

3. Email Alert Logic Transition
The system evolved from a basic send-on-startup (single email) approach to a continuous monitoring and threshold-based alert system using non-blocking logic.

Key Improvements
Continuous Monitoring
The temperature sensor is read continuously within the loop() function.

Threshold Check
An email alert is triggered only when currentTempC exceeds a defined TEMP_THRESHOLD_C
(set to 55.0 °C in the final implementation).

Non-Blocking Timing
A rate-limiting mechanism using millis() prevents email spamming.
Even if the temperature remains above the threshold, a new email is sent only after
SEND_INTERVAL_MS has elapsed (e.g., 30 seconds).

4. Final System Architecture
The resulting code establishes a complete and functional IoT alert system with the following workflow:

Check Interval
Non-blocking check to determine whether the alert interval has expired.

Read Sensor
Acquire the current ESP32 chip temperature.

Check Condition
Determine whether the temperature exceeds the defined threshold.

Connect and Authenticate
Connect to Wi-Fi and authenticate the secure SMTP session (only when required).

Send Email
Assemble and transmit the email, embedding the current temperature in both the subject and body.

Update Timer
Reset the alert timer using lastSendTime = millis().

✅ Outcome
This system successfully integrates:

Reliable internal hardware data acquisition

Secure cloud-based email communication

Non-blocking logic suitable for continuous operation

The final implementation overcomes the ESP32 core version compatibility issues encountered earlier and provides a stable foundation for further sensor expansion or power-optimization enhancements.
