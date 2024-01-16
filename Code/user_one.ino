// Importing libraries
#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Setting up your wifi connection
#define WIFI_SSID "enter ssid here"
#define WIFI_PASSWORD "enter password here"

// Setting up Firebase
#define API_KEY "enter API key here"
#define DATABASE_URL "enter API url here"

// RGB LED Pins
const int redPin = 13;
const int greenPin = 12;
const int bluePin = 14;

// RGB LED Color
int red = 0;
int green = 0;
int blue = 0;

// Color combination for RGB LED (feel free to exchange this list with your own colors)
const int colors[][3] = {
  {255, 0, 0},    // Red
  {0, 255, 0},    // Green
  {0, 0, 255},    // Blue
  {255, 150, 0},  // Yellow
  {255, 0, 255}   // Purple
};

int ledStatus = 1;
int ledStatusOld = 1;

// Button Pins
#define buttonPin 27
int buttonState = 0;
int buttonStateOld = 0;
int buttonSend = 0;
int buttonSendOld = 0;
int buttonLED = 26;

//Define Firebase Data object and other silly things
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // setting Baud rate
  Serial.begin(115200);

  // setting the LED pins as outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buttonLED, OUTPUT);
  // setting the button pin as an input
  pinMode(buttonPin, INPUT_PULLUP);

  // turn on the led inside the button
  analogWrite(buttonLED, 10);

  // calling connectToWiFi function
  connectToWiFi();

  // Assigning the API key, and URL
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up Firebase
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)) {}
  sendDataPrevMillis = millis();

  // reading the button state to see if it is pressed
  int buttonState = digitalRead(buttonPin);

  // if it is pressed, choose one random color from our list
  if (buttonState == 0 and buttonStateOld == 1) {
    // Select a random color from the list
    int randomColorIndex = random(5);
    red = colors[randomColorIndex][0];
    green = colors[randomColorIndex][1];
    blue = colors[randomColorIndex][2];
    buttonSend = randomColorIndex + 1;
  } else if (buttonState == 1) {
    buttonSend = 0;
  }

  // then light up the LED in our chosen color
  if (buttonState == 0) {
    // Turn on the LED
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
  }

  // sending the buttonState to the database
  if (Firebase.RTDB.setInt(&fbdo, "/user_one", buttonSend)) {
    Serial.println("");
    Serial.println(buttonSend);
    Serial.println("--- successfully saved to: " + fbdo.dataPath());
    Serial.println("(" + fbdo.dataType() + ")");
  } else {
    Serial.println("Data upload failed");
  }

  // downloading user two's data
  if (Firebase.RTDB.getInt(&fbdo, "/user_two")) {
    if (fbdo.dataType() == "int") {
      ledStatus = fbdo.intData();
      if (ledStatus != 0) {
        // light up our LED in the same color as the user two
        red = colors[ledStatus - 1][0];
        green = colors[ledStatus - 1][1];
        blue = colors[ledStatus - 1][2];
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
      }
      if (ledStatus == 0) {
        analogWrite(redPin, 0);
        analogWrite(greenPin, 0);
        analogWrite(bluePin, 0);
      }
      Serial.println("Successful READ from" + fbdo.dataPath() + ": " + ledStatus + "!");
    }
  } else {
    Serial.println("Data download failed");
  }

  // reset LED and button status
  ledStatusOld = ledStatus;
  buttonStateOld = buttonState;
}
