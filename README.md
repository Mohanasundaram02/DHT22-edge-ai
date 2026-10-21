# ESP32 Edge AI Anomaly Detection

An embedded Edge AI project that monitors **temperature and humidity** using a **DHT22 sensor** and an **ESP32**, runs a **TensorFlow Lite Micro (LiteRT Micro)** model for local anomaly detection, and publishes readings and model output over **MQTT**.

## Overview

The system combines sensor acquisition, on-device machine-learning inference, and cloud-connected telemetry. The ESP32 reads the DHT22, prepares the model input, runs inference locally, and publishes temperature, humidity, and a `confidence` value to an MQTT broker.

> **Note:** The precise meaning of `confidence` (for example, anomaly probability versus normal-condition score) depends on the model's output definition. Model accuracy and inference performance have not yet been benchmarked.

## Features

- Temperature and relative humidity monitoring with DHT22
- Local ML inference on ESP32 using TensorFlow Lite Micro
- MQTT-based telemetry publishing
- JSON-formatted sensor and model output
- Serial-monitor logging for connection and publishing status

## Hardware and software

| Component | Purpose |
| --- | --- |
| ESP32 development board | Sensor reading, local inference, MQTT connectivity |
| DHT22 | Temperature and humidity measurements |
| TensorFlow Lite Micro / LiteRT Micro | On-device ML inference |
| MQTT broker | Receives telemetry messages |
| Wi-Fi | Network connection for MQTT |

## Architecture

```text
DHT22 (temperature + humidity)
            |
            v
          ESP32
            |
     Preprocessing
            |
            v
 TensorFlow Lite Micro model
            |
       Model output
            |
            v
   MQTT JSON publication
            |
            v
       MQTT broker
```

The ML inference runs on the ESP32; MQTT is used to transmit the results.

## Example MQTT payload

```json
{
  "temp": 41.7,
  "humidity": 49.1,
  "confidence": 0.02
}
```

Example serial output:

```text
Connecting to MQTT...Connected!
Published to Cloud: {"temp":41.7, "humidity":49.1, "confidence":0.00}
Published to Cloud: {"temp":41.8, "humidity":49.0, "confidence":0.08}
Published to Cloud: {"temp":41.4, "humidity":49.0, "confidence":0.00}
```

These are sample readings, **not** evidence of validated anomaly-detection accuracy.

## Getting started

1. Install an ESP32-compatible development environment (for example, Arduino IDE with the ESP32 board package).
2. Install the libraries required by your firmware, including the DHT22 driver, MQTT client, and TensorFlow Lite Micro integration used by your project.
3. Connect the DHT22 to the ESP32 according to the GPIO pin defined in your firmware. Check the sensor module's voltage requirements and use an appropriate data-line pull-up if needed.
4. Configure your Wi-Fi credentials, MQTT broker address, port, topic, and authentication in your local configuration.
5. Include your trained model as required by your TensorFlow Lite Micro implementation.
6. Build and flash the firmware, then open the serial monitor to inspect readings and MQTT publication.

> The exact GPIO assignment, library versions, MQTT topic, and build commands depend on your source code and should be documented here before publishing. **Never commit real Wi-Fi passwords or MQTT credentials.**

## Suggested repository structure

Organize your actual files as appropriate:

```text
ESP32-EdgeAI-Anomaly-Detection/
├── README.md
├── firmware/             # ESP32 source code
├── model/                # Trained/converted ML model
├── training/             # Training and preprocessing scripts (if available)
├── docs/                 # Wiring diagram and architecture notes
└── results/              # Benchmark measurements (when available)
```

## Benchmarking roadmap

To evaluate the system objectively, measure:

- **Detection quality:** precision, recall, F1 score, ROC-AUC (where applicable), and false alarms per day on labeled, held-out data
- **Inference latency:** mean and p95 runtime of `interpreter.Invoke()`
- **Memory usage:** tensor arena / RAM usage and firmware/model flash footprint
- **Power consumption:** energy per inference and average device power, if measurement equipment is available
- **Baseline comparison:** ML anomaly detector versus fixed temperature/humidity thresholds

Use multiple days and environmental conditions for testing; avoid treating a handful of MQTT messages as a validated benchmark.

## Applications

- Environmental condition monitoring
- Smart-building temperature and humidity alerts
- Edge AI and TinyML education and experimentation
- Prototype monitoring for storage environments (subject to application-specific validation)

## Future improvements

- Add timestamps and an explicit anomaly label to MQTT payloads
- Publish model version and inference latency
- Add robust sensor-failure handling and MQTT reconnection logic
- Benchmark quantized versus unquantized model behavior where supported
- Create a dashboard for trends and anomaly alerts

## License

Add a `LICENSE` file with the license you choose before describing this project as open source.
