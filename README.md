# Sun Clock 🌅

A GPS-synchronized 24-hour LED clock that automatically displays sunrise and sunset times for your location. Built with ESP32 and WS2812B addressable LEDs.

![Sun Clock Demo](images/sun_clock_demo.jpg)
*Photo coming soon - upload your own Sun Clock photo here!*

## ✨ Features

- **🛰️ GPS Time Sync**: Automatically sets time and timezone based on your GPS location
- **🌅 Sunrise/Sunset Display**: Shows daily sunrise and sunset times as colored arcs
- **📍 Location Aware**: Works anywhere in the world - no hardcoded locations
- **📶 WiFi Fallback**: Falls back to internet time sync when GPS unavailable
- **💡 24-Hour LED Display**: Each LED represents one hour of the day
- **🔆 Auto Brightness**: Adjusts brightness based on time of day
- **🌐 Captive Portal**: Easy WiFi setup through mobile device
- **🔄 OTA Updates**: Update firmware wirelessly

## 🛒 Shopping List

You'll need these components (total cost ~$25-35):

| Component | Quantity | Estimated Price | Notes |
|-----------|----------|----------------|-------|
| [ESP32-C3 SuperMini](https://www.aliexpress.com/item/1005005877531694.html) | 1 | $3-5 | Main microcontroller |
| [WS2812B LED Ring 24-pixel](https://www.amazon.com/dp/B07D1F4VP1) | 1 | $8-12 | Addressable RGB LEDs |
| [NEO-6M GPS Module](https://www.amazon.com/dp/B07P8YMVNT) | 1 | $8-15 | GPS receiver |
| [220Ω Resistor](https://www.amazon.com/dp/B072BL2VX1) | 1 | $1-2 | LED data line protection |
| [Breadboard Jumper Wires](https://www.amazon.com/dp/B07GD2BWPY) | 1 set | $3-5 | For connections |
| [Mini Breadboard](https://www.amazon.com/dp/B07DL13RZH) | 1 (optional) | $2-3 | For cleaner wiring |

## 🔌 Hardware Setup

### ESP32-C3 SuperMini Pinout
```
    USB-C
    ┌─────┐
 1  │ 5V  │ GND  2
 3  │ G3  │ G10  4
 5  │ G2  │ G1   6
 7  │ G20 │ G21  8
 9  │ G6  │ G7   10
11  │ G5  │ G4   12
13  │ G9  │ G8   14
15  │ G19 │ G18  16
    └─────┘
```

### Wiring Diagram

**LED Ring → ESP32:**
- VCC (5V) → Pin 1 (5V)
- GND → Pin 2 (GND)
- DIN → 220Ω resistor → Pin 14 (GPIO8)

**GPS Module → ESP32:**
- VCC → Pin 1 (5V)
- GND → Pin 2 (GND)
- TX → Pin 7 (GPIO20) - ESP32 RX
- RX → Pin 8 (GPIO21) - ESP32 TX

### 🔧 Assembly Steps

1. **Mount the LED ring** on your project surface
2. **Connect power**: Wire 5V and GND from ESP32 to both LED ring and GPS
3. **Connect LED data**: LED ring DIN → 220Ω resistor → GPIO8
4. **Connect GPS serial**: GPS TX→GPIO20, GPS RX→GPIO21
5. **Double-check connections** before powering on

⚠️ **Important**: The 220Ω resistor protects the LED data line. Don't skip it!

## 💾 Software Setup

### Prerequisites
- [Arduino IDE](https://www.arduino.cc/en/software) or [Arduino CLI](https://arduino.github.io/arduino-cli/)
- ESP32 board support package

### 1. Install Arduino IDE and ESP32 Support

1. Download and install [Arduino IDE](https://www.arduino.cc/en/software)
2. Open Arduino IDE
3. Go to **File → Preferences**
4. Add this URL to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
5. Go to **Tools → Board → Boards Manager**
6. Search for "ESP32" and install "ESP32 by Espressif Systems"

### 2. Install Required Libraries

Go to **Tools → Manage Libraries** and install:
- `Adafruit NeoPixel` by Adafruit

### 3. Download and Configure

1. **Download this project** (Clone or download ZIP)
2. **Copy `.env.example` to `.env`**
3. **Edit `.env`** with your WiFi credentials:
   ```env
   WIFI_SSID_1="YourWiFiName"
   WIFI_PASSWORD_1="YourPassword"
   ```
4. **Update the Arduino sketch** with your WiFi details (see Configuration section)

## ⚙️ Configuration

### WiFi Setup
Edit the WiFi credentials in `sun_clock.ino`:

```cpp
// Update these with your network credentials
WiFiNetwork networks[] = {
  {"YOUR_WIFI_SSID_1", "YOUR_PASSWORD_1"},
  {"YOUR_WIFI_SSID_2", "YOUR_PASSWORD_2"},  // Optional backup networks
  {"YOUR_WIFI_SSID_3", "YOUR_PASSWORD_3"},
  {"YOUR_WIFI_SSID_4", "YOUR_PASSWORD_4"}
};
```

### Hardware Configuration
If you need to change pins, edit these constants:
```cpp
#define LED_PIN 8        // GPIO pin for LED ring
#define BRIGHTNESS 100   // LED brightness (0-255)
```

## 📤 Upload Instructions

1. **Connect ESP32** to your computer via USB-C
2. **Select board**: Tools → Board → ESP32C3 Dev Module
3. **Select port**: Tools → Port → (your ESP32 port)
4. **Open** `sun_clock.ino` in Arduino IDE
5. **Upload** (Ctrl+U or Upload button)

### Upload Settings
- Board: "ESP32C3 Dev Module"
- Flash Size: "4MB"
- Partition Scheme: "Default 4MB with spiffs"

## 🚀 Usage

### First Boot
1. **Power on** - ESP32 will show a startup animation
2. **GPS search** - Yellow spinning dots indicate GPS satellite search
3. **WiFi connection** - Blue pattern during WiFi connection
4. **Time sync** - Green flash when time is synchronized

### Normal Operation
- **Current hour**: Bright white LED
- **Sunrise arc**: Orange/yellow LEDs before and during sunrise
- **Daylight**: Blue background LEDs during day
- **Sunset arc**: Orange/red LEDs before and during sunset
- **Night**: Dim blue/purple LEDs

### LED Status Indicators
| Pattern | Meaning |
|---------|---------|
| Yellow spinning dots | GPS searching for satellites |
| Blue chase | Connecting to WiFi |
| Green flash | Time synchronized successfully |
| Red flash | Error (check serial monitor) |
| White LED | Current hour |
| Colored arcs | Sunrise/sunset times |

## 🔧 Troubleshooting

### GPS Issues
- **Red flashing on GPS**: Normal - searching for satellites
- **No GPS fix**: Try outdoors or near window, GPS needs clear sky view
- **GPS takes long**: First fix can take 5-15 minutes, subsequent fixes are faster

### WiFi Issues
- **Can't connect**: Check SSID and password in code
- **No internet**: Check router connection
- **Captive portal**: If WiFi fails, ESP32 creates "SunClock-Setup" network

### LED Issues
- **No LEDs**: Check 5V power connection and 220Ω resistor
- **Wrong colors**: Check LED ring wiring (DIN pin)
- **Some LEDs work**: Power supply may be insufficient

### Programming Issues
- **Upload failed**: Check USB cable and ESP32 connection
- **Board not found**: Install ESP32 board package
- **Compilation errors**: Install Adafruit NeoPixel library

### Serial Monitor Debug
Connect at 115200 baud to see debug output:
```
=== Sun Clock Starting ===
GPS initialized on Serial1 (GPIO20/21)
Searching for WiFi networks...
GPS: Searching for satellites...
```

## 🌍 How It Works

1. **GPS Location**: Gets your exact latitude/longitude from satellites
2. **Sunrise Calculation**: Calculates sunrise/sunset times for your location and date
3. **Time Zones**: Automatically determines timezone from GPS coordinates
4. **LED Mapping**: Maps 24 hours to 24 LEDs, with current time as bright white
5. **Solar Display**: Shows sunrise/sunset as colored arcs on the LED ring

## 🎨 Customization Ideas

- **Change colors**: Modify sunrise/sunset colors in the code
- **Add seasons**: Different color schemes for different times of year
- **Weather integration**: Add weather API for cloud cover
- **Multiple timezones**: Show time in different zones
- **Alarm clock**: Add buzzer for sunrise alarm

## 📡 Technical Details

- **Microcontroller**: ESP32-C3 (WiFi + Bluetooth)
- **GPS Protocol**: NMEA (GPRMC sentences)
- **LED Protocol**: WS2812B addressable RGB
- **Time Sync**: GPS primary, NTP fallback
- **Astronomy**: Real sunrise/sunset calculations
- **Memory**: ~30% flash, ~15% RAM usage

## 🤝 Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Adafruit for the excellent NeoPixel library
- ESP32 community for hardware support
- All the makers who will improve this project!

## 📞 Support

- **Issues**: Use GitHub Issues for bugs and feature requests
- **Discussions**: Use GitHub Discussions for questions
- **Documentation**: Check this README first

---

**Made with ❤️ for the maker community**

*Don't forget to star ⭐ this repo if you found it useful!*