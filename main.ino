#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <time.h>

#define WIFI_SSID     "Binik8"
#define WIFI_PASSWORD "cucumber2pear"
#define BOT_TOKEN     "TOKEN"
#define RELAY_PIN     26

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

unsigned long bot_lasttime = 0;
const unsigned long BOT_MTBS = 1000;
unsigned long last_status_time = 0;
const unsigned long STATUS_INTERVAL = 12UL * 60UL * 60UL * 1000UL; // 12 hours in ms

String my_chat_id = ""; // Will be set from the first command received

String getPublicIP() {
  HTTPClient http;
  http.begin("https://api.ipify.org");
  int httpCode = http.GET();
  String payload = "Unavailable";
  if (httpCode > 0) {
    payload = http.getString();
  }
  http.end();
  return payload;
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (my_chat_id == "") {
      my_chat_id = chat_id; // Save chat ID on first interaction
    }

    if (text == "/click") {
      bot.sendChatAction(chat_id, "typing");
      digitalWrite(RELAY_PIN, HIGH);
      delay(500);
      digitalWrite(RELAY_PIN, LOW);
      bot.sendMessage(chat_id, "✅ Clicked (short press)");
    }

    if (text == "/clicklong") {
      bot.sendChatAction(chat_id, "typing");
      digitalWrite(RELAY_PIN, HIGH);
      delay(20000);
      digitalWrite(RELAY_PIN, LOW);
      bot.sendMessage(chat_id, "✅ Clicked (long press)");
    }

    if (text == "/ip") {
      String publicIP = getPublicIP();
      bot.sendMessage(chat_id, "📡 Local IP: " + WiFi.localIP().toString() + "\n🌍 Public IP: " + publicIP);
    }

    if (text == "/uptime") {
      long uptime = millis() / 1000;
      String upstr = String(uptime / 3600) + "h " + String((uptime % 3600) / 60) + "m";
      bot.sendMessage(chat_id, "⏱ Uptime: " + upstr);
    }
  }
}

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());
}

void sendStatus() {
  if (my_chat_id != "") {
    String publicIP = getPublicIP();
    bot.sendMessage(my_chat_id, "I am here (" + WiFi.localIP().toString() + ")\n🌍 Public IP: " + publicIP);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  connectToWiFi();

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    delay(100);
    now = time(nullptr);
  }

  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  delay(2000); // Wait a moment to ensure connection
  my_chat_id = "<YOUR_CHAT_ID_HERE>"; // Replace with your chat ID or leave empty to set dynamically
  sendStatus();
  last_status_time = millis();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  if (millis() - last_status_time > STATUS_INTERVAL) {
    sendStatus();
    last_status_time = millis();
  }
}
