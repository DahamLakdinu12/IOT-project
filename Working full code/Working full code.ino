#include <Servo.h>
#include <DHT.h>

// --- Pin Definitions ---
#define LDR_PIN D7               // Digital LDR pin (used like motion)
#define BUZZER1_PIN D3           // Buzzer 1
#define BUZZER2_PIN D4           // Buzzer 2
#define SERVO_PIN D6             // Servo motor
#define DHTPIN D5                // DHT22 signal pin
#define DHTTYPE DHT22            // Sensor type
#define MOISTURE_PIN A0          // Soil moisture analog pin
#define RELAY_PIN D8             // Relay module control pin (for water pump)

// --- Objects ---
Servo sg90;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  pinMode(LDR_PIN, INPUT);
  pinMode(BUZZER1_PIN, OUTPUT);
  pinMode(BUZZER2_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);     // Ensure pump is OFF at start
  digitalWrite(BUZZER1_PIN, LOW);   // Ensure buzzers are OFF at start
  digitalWrite(BUZZER2_PIN, LOW);

  sg90.attach(SERVO_PIN);
  dht.begin();

  Serial.println("System Initialized...");
}

void loop() {
  // --- LDR Sensor Logic ---
  int ldrState = digitalRead(LDR_PIN);
  Serial.print("LDR Reading (Motion Detected?): ");
  Serial.println(ldrState);

  if (ldrState == HIGH) {
    sg90.write(0); // Open door
    Serial.println("Motion Detected! Door Opened");

    // Beep both buzzers 3 times
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER1_PIN, HIGH);
      digitalWrite(BUZZER2_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER1_PIN, LOW);
      digitalWrite(BUZZER2_PIN, LOW);
      delay(200);
    }
  } else {
    sg90.write(180); // Close door
    Serial.println("No motion. Door Closed");

    digitalWrite(BUZZER1_PIN, LOW);
    digitalWrite(BUZZER2_PIN, LOW);
  }

  // --- DHT22 Sensor Reading ---
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
  }

  // --- Soil Moisture Sensor Logic ---
  int moistureVal = analogRead(MOISTURE_PIN);
  Serial.print("Soil Moisture Level: ");
  Serial.print(moistureVal);

  if (moistureVal < 500) {
    Serial.println(" → Soil is Wet");
    digitalWrite(RELAY_PIN, LOW);  // Turn OFF pump
  } else {
    Serial.println(" → Soil is Dry");
    digitalWrite(RELAY_PIN, HIGH); // Turn ON pump
    Serial.println("Water Pump Activated!");
  }

  Serial.println("------------------------------------------------");
  delay(3000);  // 3s delay between readings
}
