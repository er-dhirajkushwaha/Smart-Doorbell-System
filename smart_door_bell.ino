#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ---------- Pin Definitions ----------
#define BUTTON_PIN 14        // D5 Push Button
#define LOCK_PIN 5           // D1 Relay controlling solenoid
#define LED_PIN 2            // D4 Built-in LED for indication

// ---------- WiFi Credentials ----------
const char* ssid = "";
const char* password = "";

// ---------- Telegram Bot ----------
#define BOT_TOKEN ""
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ---------- Authorized Users ----------
String chatIds[] = {
  "", // Dhiraj
  ""  // Karan
};
int chatCount = sizeof(chatIds)/sizeof(chatIds[0]);

// ---------- Variables ----------
bool buttonPressed = false;

// ---------- Setup ----------
void setup() {
  Serial.begin(9600);

  // Pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LOCK_PIN, LOW); // Relay OFF
  digitalWrite(LED_PIN, HIGH);  // LED OFF

  // Connect WiFi
  WiFi.begin(ssid, password);
  client.setInsecure(); // Telegram HTTPS

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
}

// ---------- Loop ----------
void loop() {
  checkButton();
  handleTelegram();
}

// ---------- Functions ----------

// Button pressed → send alert
void checkButton() {
  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
    buttonPressed = true;
    Serial.println("Doorbell Pressed!");

    for (int i = 0; i < chatCount; i++) {
      bot.sendMessage(
        chatIds[i],
        "🔔 Doorbell Alert!\nSomeone is at the door.\nSend /open to unlock.",
        ""
      );
    }
  }

  if (digitalRead(BUTTON_PIN) == HIGH) {
    buttonPressed = false;
  }
}

// Handle incoming Telegram commands
void handleTelegram() {
  int newMessages = bot.getUpdates(bot.last_message_received + 1);

  while (newMessages) {
    for (int i = 0; i < newMessages; i++) {
      String text = bot.messages[i].text;
      String chat_id = bot.messages[i].chat_id;

      if (text == "/open" && isAuthorized(chat_id)) {
        bot.sendMessage(chat_id, "🔓 Door Unlocked", "");
        openDoor();
      }
    }
    newMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// Open the solenoid lock
void openDoor() {
  digitalWrite(LOCK_PIN, HIGH); // Relay ON → Unlock
  blinkLED(3, 300);             // Blink built-in LED 3 times
  delay(3000);                  // Keep door unlocked
  digitalWrite(LOCK_PIN, LOW);  // Relay OFF → Lock door
}

// Blink built-in LED (active LOW) for indication
void blinkLED(int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LOW);   // LED ON
    delay(delayTime);
    digitalWrite(LED_PIN, HIGH);  // LED OFF
    delay(delayTime);
  }
}

// Check if user is authorized
bool isAuthorized(String chat_id) {
  for (int i = 0; i < chatCount; i++) {
    if (chat_id == chatIds[i]) return true;
  }
  return false;
}
