# Osciloscopio-DIY-ESP32

El proyecto implementa un osciloscopio básico utilizando un ESP32 y una pantalla OLED SH1106 de 128x64 píxeles. El sistema adquiere señales analógicas mediante el ADC interno del ESP32, las almacena en un buffer y posteriormente las grafica en tiempo real.

🖼️ESQUEMATICO

<img width="1298" height="886" alt="ESQUEMATICO1" src="https://github.com/user-attachments/assets/29ffebc6-27e6-41a8-9b6c-4d96d27e7b41" />


<h3>La implementacion incorpora las siguientes caracteristicas:</h3>

- Visualización de formas de onda en tiempo real en un OLED de 128x64
- Control del potenciómetro para base de tiempo y amplitud
- Botón multifunción
  - 1 Pulso: Cambiar entre control de tiempo y amplitud
  - 2 Puslos: Modo automático ajusta la base de tiempo automáticamente
  - 3 Pulsos: Enter Modo cursor medir la frecuencia manualmente


<h3>Hardware utilizado<h3>
- Microcontrolador
- ESP32
- ADC de 12 bits
- Comunicación I2C para display OLED
- Pantalla OLED SH1106

| Componentes         | ESP32 Pin                    | Notas                                         |
|------------------|------------------------------|-----------------------------------------------|
| Entrada analógica | GPIO 12                      | Entrada analógica(max 3.3 V, usar divisor resistivo) |
| Potenciómetro    | GPIO 15                      | Ajuste de base de tiempo / amplitud                  |
| Botón      | GPIO 5                       | Botón de control multifunción                |
| Pantalla OLED SH1106 | SDA → GPIO 21, SCL → GPIO 22 | 3.3 V power                                   |

