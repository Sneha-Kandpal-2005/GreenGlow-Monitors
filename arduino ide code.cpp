#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> // Use Secure client for HTTPS

// Pin Definitions 
#define TRIG_PIN D1   // GPIO5
#define ECHO_PIN D2   // GPIO4
#define GREEN_LED D5  // GPIO14
#define RED_LED D6    // GPIO12

// Dustbin Configuration
const int BIN_HEIGHT = 26;  // Total height of bin in cm
const int THRESHOLD = 80;   // Threshold percentage for alert
const int MIN_DISTANCE = 2; // Minimum valid distance in cm (changed from 5 to 2)

// WiFi Credentials 
const char* ssid = "Galaxy M33 5G DFEE";
const char* password = "sbzv2570";

// CircuitDigest SMS API Configuration 
const char* apiKey = "6TiDDMtsj6Vw";
const char* templateID = "101"; // This is the "101 format"
const char* mobileNumber = "+917701883107";
const char* var1 = "Smart Bin"; // var1 for the template
String var2 = "0"; // var2 (fill %) - will be updated in loop

// Variables
bool smsSent = false; // Flag to prevent multiple SMS
unsigned long lastCheck = 0;
const unsigned long checkInterval = 5000; // Check every 5 seconds

void setup() {
  Serial.begin(115200); // Using the faster baud rate
  delay(10);
  
  // Initialize pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  // Initial LED state
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  
  Serial.println("\n\nSmart Dustbin System Starting...");
  
  // Connect to WiFi
  connectWiFi();
}

void loop() {
  // Check dustbin level every 5 seconds (non-blocking)
  if (millis() - lastCheck >= checkInterval) {
    lastCheck = millis();
    
    // Get distance from ultrasonic sensor
    long distance = getDistance();
    
    // Skip if invalid reading
    if (distance == -1) {
      Serial.println("Invalid sensor reading, skipping...");
      return;
    }
    
    // Calculate fill percentage
    int fillLevel = calculateFillLevel(distance);
    
    // *** THIS IS THE FIX ***
    // Update the global var2 variable for the SMS function
    var2 = String(fillLevel);
    
    // Display status
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm | Fill Level: ");
    Serial.print(fillLevel);
    Serial.println("%");
    
    // DEBUG: Print condition check
    Serial.print("Checking: fillLevel(");
    Serial.print(fillLevel);
    Serial.print(") >= THRESHOLD(");
    Serial.print(THRESHOLD);
    Serial.print(") = ");
    Serial.println(fillLevel >= THRESHOLD ? "TRUE" : "FALSE");
    
    // Control LEDs and send SMS based on fill level
    if (fillLevel >= THRESHOLD) {
      // Bin is 80% or more full
      Serial.println(">>> Entering FULL condition - Setting RED LED HIGH");
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      
      // DEBUG: Check actual pin states
      Serial.print("Green LED pin D5 state after digitalWrite: ");
      Serial.println(digitalRead(GREEN_LED));
      Serial.print("Red LED pin D6 state after digitalWrite: ");
      Serial.println(digitalRead(RED_LED));
      
      // Send SMS only once when threshold is crossed
      if (!smsSent) {
        Serial.println("Threshold reached! Sending SMS...");
        sendSMS(); // Call the original, working SMS function
        smsSent = true;
      }
    } else {
      // Bin is less than 80% full
      Serial.println(">>> Entering NORMAL condition - Setting GREEN LED HIGH");
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      smsSent = false; // Reset flag when bin is emptied
    }
    
    Serial.println(); // Blank line for readability
  }
}

// Function to connect to WiFi
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed!");
  }
}

// Function to get distance from ultrasonic sensor
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(15);
  digitalWrite(TRIG_PIN, LOW);
  
  // Use a reasonable timeout
  long duration = pulseIn(ECHO_PIN, HIGH, 50000); // 50ms timeout
  
  if (duration == 0) {
    return -1; // Timeout is an invalid reading
    
  }
  
  long distance = duration * 0.034 / 2;
  
  if (distance > MIN_DISTANCE && distance <= 400) {
    return distance;
  } else {
    return -1; // Invalid reading
  }
}

// Function to calculate fill level percentage
int calculateFillLevel(long distance) {
  if (distance >= BIN_HEIGHT) {
    return 0; // Empty bin
  }
  
  int fillLevel = 100 - ((distance * 100) / BIN_HEIGHT);
  
  if (fillLevel < 0) fillLevel = 0;
  if (fillLevel > 100) fillLevel = 100;
  
  return fillLevel;
}

void sendSMS() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Skip cert validation
    HTTPClient http;
    
    String apiUrl = "https://www.circuitdigest.cloud/send_sms?ID=" + String(templateID);
    
    http.begin(client, apiUrl);
    http.addHeader("Authorization", apiKey);
    http.addHeader("Content-Type", "application/json");
    
    // This is the correct JSON payload for your template
    String payload = "{\"mobiles\":\"" + String(mobileNumber) + "\",\"var1\":\"" + String(var1) + "\",\"var2\":\"" + var2 + "\"}";
    
    Serial.println("Sending SMS request to circuitdigest.cloud...");
    Serial.println("Payload: " + payload);

    // This is a POST request, not GET
    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode == 200) {
      Serial.println("SMS sent successfully!");
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.print("SMS failed. Code: ");
      Serial.println(httpResponseCode);
      Serial.println("Response: " + http.getString());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected—retrying...");
    connectWiFi(); // Try to reconnect
  }
}