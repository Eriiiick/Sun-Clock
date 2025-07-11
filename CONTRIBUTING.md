# Contributing to Sun Clock

Thank you for your interest in contributing to Sun Clock! 🌅

## How to Contribute

### Reporting Bugs
- Use GitHub Issues to report bugs
- Include detailed steps to reproduce
- Mention your hardware setup (ESP32 model, LED ring, GPS module)
- Include serial monitor output if relevant

### Suggesting Features
- Open a GitHub Issue with the "enhancement" label
- Describe the feature and why it would be useful
- Consider if it fits the project's scope (GPS + LED clock)

### Code Contributions

1. **Fork the repository**
2. **Create a feature branch** (`git checkout -b feature/amazing-feature`)
3. **Test your changes** thoroughly on real hardware
4. **Follow the coding style** (see below)
5. **Commit your changes** (`git commit -m 'Add amazing feature'`)
6. **Push to your branch** (`git push origin feature/amazing-feature`)
7. **Open a Pull Request**

## Coding Guidelines

### Arduino Code Style
- Use 2-space indentation
- Descriptive variable names (`currentHour` not `h`)
- Comment complex calculations
- Keep functions focused and small
- Use `Serial.println()` for debug output

### Hardware Considerations
- Test on ESP32-C3 SuperMini (primary target)
- Ensure compatibility with standard WS2812B LED rings
- Consider power consumption in battery-powered setups
- Verify GPS module compatibility (NEO-6M standard)

### Documentation
- Update README.md for new features
- Add comments for complex algorithms
- Include wiring diagrams for new hardware
- Test all installation instructions

## Testing Checklist

Before submitting a PR, please verify:

- [ ] Code compiles without warnings
- [ ] GPS acquisition works (test outdoors)
- [ ] WiFi fallback functions properly
- [ ] LED animations display correctly
- [ ] Serial output is helpful for debugging
- [ ] Power consumption is reasonable
- [ ] Works with 3.3V and 5V power sources

## Hardware Testing

Test your changes with:
- Fresh GPS cold start (no previous satellite data)
- Multiple WiFi networks
- Different LED ring sizes (if supporting new counts)
- Various brightness levels
- Day/night operation

## Code Review Process

1. Maintainers will review your PR within a few days
2. We may request changes or ask questions
3. Once approved, your PR will be merged
4. Your contribution will be recognized in releases

## Development Setup

### Required Hardware
- ESP32-C3 SuperMini development board
- 24-pixel WS2812B LED ring
- NEO-6M GPS module
- Breadboard and jumper wires

### Software Tools
- Arduino IDE or Arduino CLI
- Git for version control
- Serial monitor for debugging

## Feature Ideas

Looking for contribution ideas? Consider:

- **Weather integration**: Add weather API for cloud cover
- **Multiple timezones**: Display multiple locations
- **Alarm functionality**: Sunrise wake-up alarm
- **Color themes**: Seasonal color schemes
- **Configuration web interface**: Better setup UI
- **Power management**: Battery operation support
- **Enclosure designs**: 3D printable cases

## Questions?

- Open a GitHub Discussion for general questions
- Use Issues for specific bugs or feature requests
- Check existing Issues before creating new ones

## Recognition

Contributors will be acknowledged in:
- README.md acknowledgments section
- Release notes for major contributions
- GitHub contributor statistics

Thank you for helping make Sun Clock better for everyone! 🚀