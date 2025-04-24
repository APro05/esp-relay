#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <time.h>
#include <ESP32Ping.h> // Include the Ping library

#define WIFI_SSID     "SSID"
#define WIFI_PASSWORD "PASSWD"
#define BOT_TOKEN     "TOKEN"
#define RELAY_PIN     13
#define LED_PIN       2   // Onboard LED pin (usually pin 2 on ESP32)

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

unsigned long bot_lasttime = 0;
const unsigned long BOT_MTBS = 1000;
unsigned long last_status_time = 0;
const unsigned long STATUS_INTERVAL = 12UL * 60UL * 60UL * 1000UL; // 12 hours in ms
bool commandsLocked = false;
bool pinging = true;
bool awaitingPassword = false;
String passwordRequestFrom = "";



// change this to your desired password -----------------------------
const String unlockPassword = "Password"; 
// change this to your desired password -----------------------------


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
    if (i >= bot.messages.size()) continue; // Avoid OOB access
    String chat_id = bot.messages[i].chat_id;
    if (chat_id == "") continue; // Skip empty messages

    String text = bot.messages[i].text;

    if (my_chat_id == "") {
      my_chat_id = chat_id; // Save chat ID on first interaction
    }

if (text == "/lockcmds") {
  commandsLocked = false;
  awaitingPassword = false; // cancel any pending unlock attempt
  bot.sendMessage(chat_id, "🔒 Commands have been *locked*.", "Markdown");
  return;
}


if (text == "/unlockcmds") {
  awaitingPassword = true;
  passwordRequestFrom = chat_id;
  bot.sendMessage(chat_id, "🔑 Send your password now to unlock commands.");
  return;
}

if (awaitingPassword && chat_id == passwordRequestFrom) {
  awaitingPassword = false;
  if (text == unlockPassword) {
    commandsLocked = true;
    bot.sendMessage(chat_id, "✅ Commands *unlocked* successfully.", "Markdown");
  } else {
    bot.sendMessage(chat_id, "❌ Incorrect password. Commands remain locked.");
  }
  return;
}

    
    if (commandsLocked && text == "/click") {
      bot.sendChatAction(chat_id, "typing");
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      digitalWrite(RELAY_PIN, LOW);
      bot.sendMessage(chat_id, "✅ Clicked (short press)");
    }

    if (commandsLocked && text == "/clicklong") {
      bot.sendChatAction(chat_id, "typing");
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      delay(10000);
      digitalWrite(LED_PIN, LOW);
      digitalWrite(RELAY_PIN, LOW);
      bot.sendMessage(chat_id, "✅ Clicked (long press)");
    }

    if (commandsLocked && text == "/ip") {
      String publicIP = getPublicIP();
      bot.sendMessage(chat_id, "📡 Local IP: " + WiFi.localIP().toString() + "\n🌍 Public IP: " + publicIP);
    }

    if (pinging && commandsLocked && text == "/pingpc") {
      pinging = false;
      // Ping the default PC IP (192.168.0.14)
      String pc_ip = "192.168.0.14";
      int pingResult = Ping.ping(pc_ip.c_str());
      if (pingResult >= 0) {
        bot.sendMessage(chat_id, "✅ Ping to " + pc_ip + " successful: " + String(pingResult) + " ms");
        pinging = true;
      } else {
        bot.sendMessage(chat_id, "❌ Ping to " + pc_ip + " failed.");
        pinging = true;
      }
    }

    if (pinging && commandsLocked && text.startsWith("/pingip ")) {
      pinging = false;
      // Ping a user-defined IP address
      String ipAddress = text.substring(8); // Get the IP address from the command
      int pingResult = Ping.ping(ipAddress.c_str());
      if (pingResult >= 0) {
        bot.sendMessage(chat_id, "✅ Ping to " + ipAddress + " successful: " + String(pingResult) + " ms");
        pinging = true;
      } else {
        bot.sendMessage(chat_id, "❌ Ping to " + ipAddress + " failed.");
        pinging = true;
      }
    }

    if (commandsLocked && text.startsWith("/clicklength ")) {
  String lengthStr = text.substring(13);
  int length = lengthStr.toInt();
  if (length > 0) {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(length);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
    bot.sendMessage(chat_id, "✅ Clicked for " + String(length) + " ms");
  } else {
    bot.sendMessage(chat_id, "❌ Invalid duration. Use like this: /clicklength 1500");
  }
}


    if (commandsLocked && text == "/uptime") {
      long uptime = millis() / 1000;
      String upstr = String(uptime / 3600) + "h " + String((uptime % 3600) / 60) + "m";
      bot.sendMessage(chat_id, "⏱ Uptime: " + upstr);
    }

    if (commandsLocked && text == "/flash") {
      // Flash the onboard LED
      for (int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
      }
      bot.sendMessage(chat_id, "⚡ Onboard LED flashed!");
    }
        if (text == "/help") {
      String helpText = 
        "🛠 Available Commands:\n"
        "/click - Short relay press\n"
        "/clicklong - Long relay press\n"
        "/clicklength <ms>\n"
        "/ip - Show local & public IP\n"
        "/uptime - Show ESP32 uptime\n"
        "/pingpc - Ping 192.168.0.14\n"
        "/pingip <ip> - Ping user IP\n"
        "/flash - Flash onboard LED\n"
        "/help - Show this message\n"
        "/unlockcmds\n"
        "/lockcmds";
      bot.sendMessage(chat_id, helpText);
    }

  }
}

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, HIGH); // Keep LED on during connection
    delay(500);
    Serial.print(".");
  }

  bot.sendMessage(chat_id, "started wifi");
  digitalWrite(LED_PIN, LOW); // Turn off LED once connected
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());
}

void sendStatus() {
  if (my_chat_id != "") {
    String publicIP = getPublicIP();
    bot.sendMessage(my_chat_id, "I am here (" + WiFi.localIP().toString() + ")\n🌍 Public IP: " + publicIP);
  }
}

void handleLED() {
  // If not connected to WiFi, turn the LED on at max brightness
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, HIGH);  // LED on
  } else {
    digitalWrite(LED_PIN, LOW);   // LED off
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT); // Initialize onboard LED pin
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, HIGH); // Start with LED off

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

  // Check and handle LED behavior based on WiFi status
  handleLED();

int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
while (numNewMessages > 0) {
  handleNewMessages(numNewMessages);
  delay(100); // avoid hammering
  numNewMessages = bot.getUpdates(bot.last_message_received + 1);
}


  if (millis() - last_status_time > STATUS_INTERVAL) {
    sendStatus();
    last_status_time = millis();
  }
}
