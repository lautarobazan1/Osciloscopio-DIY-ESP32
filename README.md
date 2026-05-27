# Osciloscopio-DIY-ESP32

El proyecto implementa un osciloscopio básico utilizando un ESP32 y una pantalla OLED SH1106 de 128x64 píxeles. El sistema adquiere señales analógicas mediante el ADC interno del ESP32, las almacena en un buffer y posteriormente las grafica en tiempo real.

🖼️ESQUEMATICO

<img width="1298" height="886" alt="ESQUEMATICO1" src="https://github.com/user-attachments/assets/29ffebc6-27e6-41a8-9b6c-4d96d27e7b41" />

La implementacion incorpora las siguientes caracteristicas:

- Control de base de tiempo.
- Ajuste de amplitud.
- Modo automático.
- Cursores de medición temporal.
- Interfaz mediante un único botón y un potenciómetro.

Hardware utilizado
- Microcontrolador
- ESP32
- ADC de 12 bits
- Comunicación I2C para display OLED
- Pantalla OLED SH1106

| Component         | ESP32 Pin                    | Notes                                         |
|------------------|------------------------------|-----------------------------------------------|
| Signal input     | GPIO 12                      | Analog input (max 3.3 V, use voltage divider) |
| Potentiometer    | GPIO 15                      | Adjusts timebase / amplitude                  |
| Push button      | GPIO 5                       | Multi-function control button                 |
| OLED (SSD1306, I²C) | SDA → GPIO 21, SCL → GPIO 22 | 3.3 V power                                   |

