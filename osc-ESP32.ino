#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// ======================================================
// Pantalla OLED SH1106 128x64 por I2C
// ======================================================
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0);

// ======================================================
// Definición de pines
// ======================================================
#define ANALOG_PIN 34
#define POT_PIN    15
#define BUTTON_PIN 5

// ======================================================
// Configuración general
// ======================================================
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define SAMPLE_POINTS  128

enum Mode { MEASURE, CURSOR };
Mode currentMode = MEASURE;

// Buffer de muestras
int samples[SAMPLE_POINTS];

// Variables de adquisición
int sampleDelay = 50;
float sensitivity = 1.0;
int offset = 0;

// Cursores verticales
int cursorA = 30;
int cursorB = 90;

// Selección de cursor activo
bool selectA = true;

// Variables para manejo del botón
bool buttonPressed = false;
unsigned long lastPressTime = 0;
int pressCount = 0;

// Tiempo máximo entre pulsaciones
const unsigned long pressGap = 400;

// Modo amplitud
bool ampMode = false;

// Modo automático
bool autoMode = false;
float autoFreq = 0;

// Último valor del potenciómetro
int lastPotValue = 0;

// ======================================================
// CONFIGURACIÓN INICIAL
// ======================================================
void setup() {

  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ANALOG_PIN, INPUT);
  pinMode(POT_PIN, INPUT);

  // Resolución ADC de 12 bits
  analogReadResolution(12);

  // Inicialización de pantalla
  display.begin();

  display.clearBuffer();

  display.setFont(u8g2_font_6x10_tr);
  display.drawStr(15, 30, "ESP32 MiniScope");

  display.sendBuffer();

  delay(1500);
}

// ======================================================
// BUCLE PRINCIPAL
// ======================================================
void loop() {

  // Lectura del botón
  handleButton();

  // Cambio de modo
  if (currentMode == MEASURE)
    runMeasure();
  else
    runCursor();
}

// ======================================================
// MODO MEDICIÓN
// ======================================================
void runMeasure() {

  int pot = analogRead(POT_PIN);

  // Salir del modo automático
  // si se mueve el potenciómetro
  if (autoMode && abs(pot - lastPotValue) > 50) {

    autoMode = false;

    flashText("AUTO OFF");
  }

  lastPotValue = pot;

  // ==================================================
  // MODO AUTOMÁTICO
  // ==================================================
  if (autoMode) {

    // Adquisición de muestras
    for (int i = 0; i < SAMPLE_POINTS; i++) {

      samples[i] = analogRead(ANALOG_PIN);

      delayMicroseconds(sampleDelay);
    }

    // Estimación de frecuencia
    autoFreq = estimateFrequency(
                 samples,
                 SAMPLE_POINTS,
                 sampleDelay
               );

    // Ajuste automático de base de tiempo
    if (autoFreq > 0) {

      float period_us =
        (1.0 / autoFreq) * 1000000.0;

      sampleDelay = constrain(
                      period_us / 40.0,
                      2,
                      1000
                    );
    }

    drawWaveform(true);

    return;
  }

  // ==================================================
  // MODO MANUAL
  // ==================================================
  if (!ampMode) {

    // Control de base de tiempo
    sampleDelay = map(
                    pot,
                    0,
                    4095,
                    2,
                    1000
                  );
  }
  else {

    // Control de sensibilidad vertical
    sensitivity =
      map(pot, 0, 4095, 5, 50)
      / 10.0;
  }

  // Captura de muestras
  for (int i = 0; i < SAMPLE_POINTS; i++) {

    samples[i] = analogRead(ANALOG_PIN);

    delayMicroseconds(sampleDelay);
  }

  drawWaveform(true);
}

// ======================================================
// MODO CURSORES
// ======================================================
void runCursor() {

  int pot = analogRead(POT_PIN);

  // Posición del cursor
  int pos = map(
              pot,
              0,
              4095,
              0,
              SCREEN_WIDTH - 1
            );

  // Movimiento del cursor seleccionado
  if (selectA)
    cursorA = pos;
  else
    cursorB = pos;

  drawWaveform(false);

  // Dibujar cursores
  display.drawLine(
    cursorA,
    0,
    cursorA,
    SCREEN_HEIGHT - 1
  );

  display.drawLine(
    cursorB,
    0,
    cursorB,
    SCREEN_HEIGHT - 1
  );

  // Diferencia temporal
  int deltaPix = abs(cursorB - cursorA);

  float deltaT =
    deltaPix * sampleDelay / 1e6;

  display.setFont(u8g2_font_5x7_tr);

  char buf[32];

  sprintf(
    buf,
    "%s dT:%.2fms",
    selectA ? "A" : "B",
    deltaT * 1000
  );

  display.drawStr(0, 10, buf);

  display.sendBuffer();
}

// ======================================================
// DIBUJO DE LA FORMA DE ONDA
// ======================================================
void drawWaveform(
  bool showInfo
) {

  display.clearBuffer();

  // Marco de pantalla
  display.drawFrame(0, 0, 128, 64);

  // Línea central
  display.drawLine(0, 32, 127, 32);

  // Dibujar señal
  for (int i = 0; i < SAMPLE_POINTS - 1; i++) {

    int y1 = map(
               (int)(samples[i] * sensitivity)
               + offset,
               0,
               4095,
               SCREEN_HEIGHT - 1,
               0
             );

    int y2 = map(
               (int)(samples[i + 1] * sensitivity)
               + offset,
               0,
               4095,
               SCREEN_HEIGHT - 1,
               0
             );

    // Limitar coordenadas
    y1 = constrain(
           y1,
           0,
           SCREEN_HEIGHT - 1
         );

    y2 = constrain(
           y2,
           0,
           SCREEN_HEIGHT - 1
         );

    // Trazar línea entre puntos
    display.drawLine(
      i,
      y1,
      i + 1,
      y2
    );
  }

  // Información en pantalla
  if (showInfo) {

    display.setFont(
      u8g2_font_5x7_tr
    );

    char buf[32];

    if (autoMode)
      sprintf(
        buf,
        "AUTO %dus",
        sampleDelay
      );
    else
      sprintf(
        buf,
        "%s %dus",
        ampMode
        ? "AMPL"
        : "TIME",
        sampleDelay
      );

    display.drawStr(2, 10, buf);
  }

  display.sendBuffer();
}

// ======================================================
// MANEJO DEL BOTÓN
// ======================================================
void handleButton() {

  bool state =
    !digitalRead(BUTTON_PIN);

  // Detección de pulsación
  if (state && !buttonPressed) {

    buttonPressed = true;

    pressCount++;

    lastPressTime = millis();
  }

  // Liberación del botón
  if (!state && buttonPressed)
    buttonPressed = false;

  // Evaluación de cantidad de pulsaciones
  if ((millis() - lastPressTime
       > pressGap)
      && pressCount > 0) {

    if (pressCount == 1)
      handleSinglePress();

    else if (pressCount == 2)
      handleDoublePress();

    else if (pressCount >= 3)
      handleTriplePress();

    pressCount = 0;
  }
}

// ======================================================
// PULSACIÓN SIMPLE
// ======================================================
void handleSinglePress() {

  if (currentMode == MEASURE) {

    ampMode = !ampMode;

    flashText(
      ampMode
      ? "AMPL"
      : "TIME"
    );
  }
  else if (currentMode == CURSOR) {

    selectA = !selectA;

    flashText(
      selectA
      ? "A"
      : "B"
    );
  }
}

// ======================================================
// DOBLE PULSACIÓN
// ======================================================
void handleDoublePress() {

  if (currentMode == MEASURE) {

    autoMode = true;

    flashText("AUTO");
  }
}

// ======================================================
// TRIPLE PULSACIÓN
// ======================================================
void handleTriplePress() {

  currentMode =
    (currentMode == MEASURE)
    ? CURSOR
    : MEASURE;

  autoMode = false;

  flashText(
    currentMode == MEASURE
    ? "MEASURE"
    : "CURSOR"
  );
}

// ======================================================
// MENSAJE TEMPORAL EN PANTALLA
// ======================================================
void flashText(
  const char *msg
) {

  display.clearBuffer();

  display.setFont(
    u8g2_font_logisoso20_tr
  );

  display.drawStr(
    10,
    40,
    msg
  );

  display.sendBuffer();

  delay(350);
}

// ======================================================
// ESTIMACIÓN DE FRECUENCIA
// ======================================================
float estimateFrequency(
  int *data,
  int len,
  int delay_us
) {

  // Nivel medio ADC
  const int mid = 2048;

  // Histéresis
  const int hysteresis = 80;

  int crossings[10];

  int crossingCount = 0;

  int last = data[0];

  // Detección de cruces ascendentes
  for (int i = 1; i < len; i++) {

    bool rising =
      (last < (mid - hysteresis))
      &&
      (data[i] >= (mid + hysteresis));

    if (rising) {

      if (crossingCount < 10) {

        crossings[crossingCount] = i;

        crossingCount++;
      }
    }

    last = data[i];
  }

  // Verificación mínima
  if (crossingCount < 3)
    return 0;

  float avgSamples = 0;

  int periods = 0;

  // Cálculo de período promedio
  for (int i = 1; i < crossingCount; i++) {

    int delta =
      crossings[i]
      - crossings[i - 1];

    if (delta > 2) {

      avgSamples += delta;

      periods++;
    }
  }

  if (periods == 0)
    return 0;

  avgSamples /= periods;

  float period_sec =
    (avgSamples * delay_us)
    / 1000000.0;

  if (period_sec <= 0)
    return 0;

  // Retorno de frecuencia
  return 1.0 / period_sec;
}
