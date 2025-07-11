# Changelog

All notable changes to Sun Clock will be documented in this file.

## [1.0.0] - 2025-07-11

### Added
- Initial release of Sun Clock
- GPS-synchronized time from satellites
- Automatic timezone detection from GPS coordinates  
- Real-time sunrise/sunset calculation for any location
- 24-hour LED ring display with current time indicator
- WiFi fallback with NTP time synchronization
- Captive portal for manual time setting
- Auto-brightness adjustment based on time of day
- Multiple WiFi network support with failover
- OTA update capability
- Comprehensive error handling and status indicators
- Visual GPS search and connection status
- Hardware support for ESP32-C3 SuperMini
- WS2812B addressable LED compatibility
- NEO-6M GPS module integration

### Features
- 🛰️ GPS time sync with automatic location detection
- 🌅 Location-aware sunrise/sunset times  
- 📍 Works anywhere in the world (no hardcoded locations)
- 📶 WiFi fallback when GPS unavailable
- 💡 24-hour visual time display
- 🔆 Automatic brightness adjustment
- 🌐 Easy WiFi setup via captive portal
- 🔄 Over-the-air firmware updates

### Technical Details
- Supports ESP32-C3 SuperMini microcontroller
- Hardware Serial1 communication with GPS (GPIO20/21)
- Adafruit NeoPixel library for LED control
- NMEA GPRMC sentence parsing for GPS data
- Astronomical calculations for sunrise/sunset
- Timezone offset calculation from longitude
- Multiple fallback mechanisms for reliability

### Documentation
- Comprehensive README with setup instructions
- Complete hardware shopping list with links
- Detailed wiring diagrams and pinouts
- Troubleshooting guide for common issues
- Contributing guidelines for open source development
- MIT license for maximum compatibility

---

## Future Releases

### Planned Features
- Weather integration for cloud cover effects
- Multiple timezone display support
- Seasonal color theme variations
- Alarm clock functionality with sunrise wake-up
- Battery operation with power management
- 3D printable enclosure designs
- Mobile app for advanced configuration

### Community Requests
- Support for different LED ring sizes
- Custom color scheme configuration
- Integration with home automation systems
- Historical sunrise/sunset data logging

---

*For detailed changes and development history, see the project's Git commit log.*