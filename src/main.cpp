#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h" //Provide the token generation process info.
#include "addons/RTDBHelper.h"  //Provide the real-time database payload printing info and other helper functions.

#define WIFI_SSID "DCS"
#define WIFI_PASSWORD "son12345"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCCTjKWELtwx1XzAe0dM_1F-E-wlXYR_Lk"

// ----Insert real-time database URL
#define DATABASE_URL "https://ledrgb-aba5c-default-rtdb.firebaseio.com"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Define DHT sensor pin
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define LED 2
#define FAN 5

// Define the time interval for sending data to Firebase (milliseconds)
unsigned long time1 = 0;
unsigned long time2 = 0;
bool signupSuccess = false;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }
  Serial.println();
  Serial.print("Connected to... ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Assigning the project API key
  config.api_key = API_KEY;

  // Assign the project URL
  config.database_url = DATABASE_URL;

  /// check signup statue
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("sign up ok");
    signupSuccess = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  // Assign the callback function for token generation task
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  dht.begin();
  pinMode(LED, OUTPUT);
}

void loop()
{
  if (Firebase.ready() && signupSuccess && (millis() - time1 > 3000 || time1 == 0))
  {
    time1 = millis();

    // Read data from DHT sensor
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    if (isnan(temperature) || isnan(humidity))
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // update data to Firebase
    if (Firebase.RTDB.setFloat(&fbdo, "/home/temp", temperature))
    {
      Serial.println("SET TEMP PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("SET TEMPFAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/home/hum", humidity))
    {
      Serial.println("SET HUM PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("SET HUM FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }

  if (Firebase.ready() && signupSuccess && (millis() - time2 > 1000 || time2 == 0))
  {
    time2 = millis();
    // get data from Firebase
    if (Firebase.RTDB.getBool(&fbdo, "/home/led"))
    {
      Serial.println("GET LED PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.println("VALUE: " + fbdo.boolData() ? "true" : "false");
      digitalWrite(LED, fbdo.boolData());
    }
    else
    {
      Serial.println("GET LED FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.getBool(&fbdo, "/home/fan"))
    {
      Serial.println("GET FAN PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.println("VALUE: " + fbdo.boolData() ? "true" : "false");
      digitalWrite(FAN, fbdo.boolData());
    }
    else
    {
      Serial.println("GET FAN FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}