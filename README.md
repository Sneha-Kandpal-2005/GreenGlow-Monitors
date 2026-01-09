## Description
**GreenGlow Monitors** is an IoT-based Smart Waste Management System designed to optimize campus hygiene through real-time monitoring. The system uses an HC-SR04 ultrasonic sensor interfaced with a NodeMCU ESP8266 microcontroller to continuously measure bin fill levels. When the waste reaches a predefined threshold of 80%, the system triggers a visual red LED alert and sends an automated SMS notification to maintenance staff via the CircuitDigest API, ensuring timely collection and preventing overflow. This solution integrates hardware and cloud-based communication to reduce manual labor, improve campus aesthetics, and provide a data-driven approach to waste management.

## Technical Specifications & Software Tools
1. **Microcontroller: NodeMCU ESP8266** (WiFi-enabled) for core logic and cloud connectivity.

2. **Sensor Technology: HC-SR04 Ultrasonic Sensor** for measuring fill distance.

3. **Programming Language: C++ (Arduino IDE)** for firmware development and hardware control.

4. **Connectivity & APIs: WiFi (ESP8266WiFi library)** and **CircuitDigest SMS API** for remote communication.

5. **Visual Indicators: Dual-LED** status system (Green for normal, Red for full).

6. **Infrastructure**: Cloud-based alert distribution to mobile devices for maintenance supervisors.
