# ESP32 Telegram Bot with Relay Control

## 🧰 Features

### 🔌 Relay Control
- `/click` – Short relay press (200ms)
- `/clicklong` – Long relay press (10s)
- `/clicklength <ms>` – Custom relay press duration

### 🌐 Network Info
- `/ip` – Show local + public IP
- `/uptime` – Show ESP32 uptime
- `/pingpc` – Ping preset IP (192.168.0.14)
- `/pingip <ip>` – Ping any custom IP

### 💡 Device Control
- `/flash` – Flash onboard LED

### 🔁 Bot Behavior
- Sends startup status + every 12 hours
- Auto WiFi reconnect with LED status feedback
- `/help` – Lists available commands

### 🔒 Security Features
- `/lockcmds` – Locks all bot commands
- `/unlockcmds` – Prepares for password unlock
- Password must be sent in the next message after `/unlockcmds`
- All commands (except `/help` and unlock flow) are disabled when locked
