#include <Arduino.h>
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Definindo pinos utilizados
#define pinSensorVazao 2   // Pino de leitura para Vazão
#define pinSensorTemp 14   // Pino de leitura da temperatura
#define PWM_Cooler 15      // Pino de saída do sinal PWM do Cooler
#define PWM_Resistor 19    // Pino de saída do sinal PWM da Resistência

// PWM setup
#define PWM_Cooler_Ch 0
#define PWM_Resistor_Ch 1
#define PWM_Freq 5000
#define PWM_Res 8

volatile uint32_t pulsos = 0;
volatile bool flagTimer = false;
hw_timer_t *timer = NULL;

// Variáveis para o sensor de temperatura DS18B20
OneWire oneWire(pinSensorTemp);          
DallasTemperature sensors(&oneWire);
float temperatura = 0.0;

// Variáveis Controlador Temperatura  
float d_t = 0.0;
int duty_cycle_t = 0;
float u_t = 0.0, ek_t = 0.0, uk_1t = 0.0, ek_1t = 0.0;

// Variáveis Controlador Vazão  
float d_f = 0.0;
int duty_cycle_f = 0;
float u_f = 0.0, ek_f = 0.0, uk_1f = 0.0, ek_1f = 0.0; 

// Vetores de setpoints ao longo do tempo
float setpoints_T[] = {40.0, 30.0};  // Temperatura
float setpoints_V[] = {0.0, 28.0};  // Vazão
int i = 0;
float referencia_T = setpoints_T[i];
float referencia_V = setpoints_V[i];

// Constantes do Controlador PI
float b0_t = 4.197;
float b1_t = -0.995*b0_t;
float b0_f = 1.72;
float b1_f = -0.792*b0_f;

// Tolerância e temporização
const float tolerancia_T = 0.5;
const unsigned long tempoEsperaT = 60000;  // 1 minutos em ms
// Controle de estados de espera
static bool esperandoTemp = false;
static unsigned long tempoInicioTemp = 0;

static bool esperandoVazao = false;
static unsigned long tempoInicioVazao = 0;

void IRAM_ATTR contarPulsos() {
  pulsos++;
}

void IRAM_ATTR timerInterrupt() {
  flagTimer = true;
}

// Controlador PI para Temperatura
float controlador_temperatura(float r, float y) {
  ek_t = r - y;
  u_t = uk_1t + b0_t * ek_t + b1_t * ek_1t;

  if (u_t > 255) u_t = 255;
  else if (u_t < 0) u_t = 0;

  ek_1t = ek_t;
  uk_1t = u_t;

  return u_t;
}

// Controlador PI para Vazão
float controlador_vazao(float r, float y) {
  ek_f = r - y;
  u_f = uk_1f + b0_f * ek_f + b1_f * ek_1f;

  if (u_f > 255) u_f = 255;
  else if (u_f < 0) u_f = 0;

  ek_1f = ek_f;
  uk_1f = u_f;

  return u_f;
}

void setup() {
  Serial.begin(230400);
  
  pinMode(pinSensorVazao, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinSensorVazao), contarPulsos, FALLING);

  pinMode(pinSensorTemp, INPUT);

  ledcSetup(PWM_Cooler_Ch, PWM_Freq, PWM_Res);
  ledcSetup(PWM_Resistor_Ch, PWM_Freq, PWM_Res);
  ledcAttachPin(PWM_Cooler, PWM_Cooler_Ch);
  ledcAttachPin(PWM_Resistor, PWM_Resistor_Ch);

  timer = timerBegin(0, 80, true); // 80 MHz / 80 = 1 us
  timerAttachInterrupt(timer, &timerInterrupt, true);
  timerAlarmWrite(timer, 500000, true); // 500 ms
  timerAlarmEnable(timer);

  ledcWrite(PWM_Cooler_Ch, 0);
  ledcWrite(PWM_Resistor_Ch, 0);

  sensors.begin();
}

void loop() {
  static uint8_t contadorTemperatura = 0;

  if (flagTimer) {
    flagTimer = false;

    // --- Controle de Vazão ---
    noInterrupts();
    uint32_t pulsosAtuais = pulsos;
    pulsos = 0;
    interrupts();

    float leituraVazao = pulsosAtuais; // Pode aplicar fator de conversão se necessário

    d_f = controlador_vazao(referencia_V, leituraVazao); 
    duty_cycle_f = int(d_f);
    ledcWrite(PWM_Cooler_Ch, duty_cycle_f);

    Serial.printf("Vazao | Tempo: %lu ms, Pulsos: %lu, PWM Cooler: %d\n", millis(), pulsosAtuais, duty_cycle_f);

    // --- Controle de Temperatura ---
    contadorTemperatura++;
    if (contadorTemperatura >= 3) {
      contadorTemperatura = 0;

      sensors.requestTemperatures();    
      temperatura = sensors.getTempCByIndex(0);

      d_t = controlador_temperatura(referencia_T, temperatura); 
      duty_cycle_t = int(d_t);
      ledcWrite(PWM_Resistor_Ch, duty_cycle_t);

      Serial.printf("Temperatura | Tempo: %lu ms, Temp: %.2f °C, PWM Resistor: %d\n", millis(), temperatura, duty_cycle_t);

      if (!esperandoTemp && fabs(temperatura - referencia_T) <= tolerancia_T) {
        esperandoTemp = true;
        tempoInicioTemp = millis();
      } else if (esperandoTemp && (millis() - tempoInicioTemp >= tempoEsperaT)) {
        esperandoTemp = false;
        if (i < 1) {
          i++;
          referencia_T = setpoints_T[i];
          referencia_V = setpoints_V[i];
        }
      }
    }
  }
}
