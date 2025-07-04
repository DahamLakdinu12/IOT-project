#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Servo.h>
#include <DHT.h>

// ------------------- WiFi Credentials -------------------
#define WIFI_SSID "Jerome"
#define WIFI_PASSWORD "jerome2005"

// ------------------- Firebase Credentials -------------------
#define FIREBASE_HOST "https://iot-project-ed4c8-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyD6LYRpOKBZRjUCNEUlt7EgF0Ov9v4UdV8"

// ------------------- Sensor Pin Definitions -------------------
#define LDR_PIN D7
#define BUZZER1_PIN D3
#define BUZZER2_PIN D4
#define SERVO_PIN D6
#define DHTPIN D5
#define DHTTYPE DHT22
#define MOISTURE_PIN A0
#define RELAY_PIN D8

// ------------------- Objects -------------------
Servo sg90;
DHT dht(DHTPIN, DHTTYPE);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);

  // Pin Setup
  pinMode(LDR_PIN, INPUT);
  pinMode(BUZZER1_PIN, OUTPUT);
  pinMode(BUZZER2_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER1_PIN, LOW);
  digitalWrite(BUZZER2_PIN, LOW);

  sg90.attach(SERVO_PIN);
  dht.begin();

  // WiFi Connection
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to WiFi");

  // Firebase Setup
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("System Initialized...");
}

void loop() {
  // --- LDR Sensor ---
  int ldrState = digitalRead(LDR_PIN);
  Serial.print("LDR Reading (Motion Detected?): ");
  Serial.println(ldrState);

  // Update Firebase with LDR state
  Firebase.setInt(fbdo, "/ldr", ldrState);

  if (ldrState == HIGH) {
    sg90.write(0); // Open door
    Serial.println("Motion Detected! Door Opened");

    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER1_PIN, HIGH);
      digitalWrite(BUZZER2_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER1_PIN, LOW);
      digitalWrite(BUZZER2_PIN, LOW);
      delay(100);
    }
  } else {
    sg90.write(180); // Close door
    Serial.println("No motion. Door Closed");
    digitalWrite(BUZZER1_PIN, LOW);
    digitalWrite(BUZZER2_PIN, LOW);
  }

  // --- DHT Sensor ---
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();

  if (isnan(temp) || isnan(humid)) {
    Serial.println("Error: Failed to read from DHT22 sensor.");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print(" °C | Humidity: ");
    Serial.print(humid);
    Serial.println(" %");

    // Upload to Firebase
    Firebase.setFloat(fbdo, "/temperature", temp);
    Firebase.setFloat(fbdo, "/humidity", humid);
  }

  // --- Soil Moisture Sensor ---
  int moistureVal = analogRead(MOISTURE_PIN);
  Serial.print("Soil Moisture Level: ");
  Serial.print(moistureVal);

  Firebase.setInt(fbdo, "/soil_moisture", moistureVal);

  if (moistureVal < 500) {
    Serial.println(" → Soil is Wet");
    digitalWrite(RELAY_PIN, LOW);  // Turn OFF pump
    Firebase.setString(fbdo, "/pump_status", "OFF");
  } else {
    Serial.println(" → Soil is Dry");
    digitalWrite(RELAY_PIN, HIGH); // Turn ON pump
    Firebase.setString(fbdo, "/pump_status", "ON");
    Serial.println("Water Pump Activated!");
  }

  // --- Optional: Read Status Command from Firebase ---
  if (Firebase.getString(fbdo, "/status")) {
    String status = fbdo.stringData();
    Serial.println("Firebase Command: " + status);
  }

  Serial.println("------------------------------------------------");
  delay(3000);
}
