#include <Arduino.h>
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Definindo pinos utilizados
#define pinSensorVazao 2   // Pino de leitura para Vazão
#define pinSensorTemp 14   // Pino de leitura da temperatura
#define PWM_Cooler 15      // Pino de saída do sinal PWM do Cooler
#define PWM_Resistor 19    // Pino de saída do sinal PWM da Resistência

// Definição dos canais PWM para ESP32
#define PWM_Cooler_Ch 0
#define PWM_Resistor_Ch 1
#define PWM_Freq 5000      // Frequência do PWM (Hz)
#define PWM_Res 8          // Resolução de 8 bits (0-255)

volatile bool flagTimer = false;
hw_timer_t *timer = NULL;

// Variáveis PWM
uint32_t dutyCycleCooler = 0;
uint32_t dutyCycleResistor = 0;

// Variáveis para o sensor de temperatura DS18B20
OneWire oneWire(pinSensorTemp);          
DallasTemperature sensors(&oneWire);
float temperatura = 0.0;
float temperaturaamb = 0.0;

// Variáveis para leitura do sensor de vazão
volatile uint32_t pulsos = 0;
float vazaoSistema = 0.0;
unsigned long tempoAnterior = 0;

// Estado do sistema
bool sistemaCritico = false;  

void IRAM_ATTR contarPulsos() {
  pulsos++;
}

void IRAM_ATTR timerInterrupt() {
  flagTimer = true;
}

void setup() {
  Serial.begin(230400);

  // Configuração dos pinos
  pinMode(pinSensorVazao, INPUT);
  pinMode(pinSensorTemp, INPUT);
  
  // Configuração dos canais PWM
  ledcSetup(PWM_Cooler_Ch, PWM_Freq, PWM_Res);
  ledcSetup(PWM_Resistor_Ch, PWM_Freq, PWM_Res);
  
  // Atribuição dos pinos aos canais PWM
  ledcAttachPin(PWM_Cooler, PWM_Cooler_Ch);
  ledcAttachPin(PWM_Resistor, PWM_Resistor_Ch);

  // Configuração da interrupção para o sensor de vazão
  attachInterrupt(digitalPinToInterrupt(pinSensorVazao), contarPulsos, RISING);

  // Inicializa o sensor de temperatura
  sensors.begin();

  // Primeira leitura da temperatura ambiente
  sensors.requestTemperatures();    
  temperaturaamb = sensors.getTempCByIndex(0);
  
  // Timer (0, 80MHz/80 = 1us tick)
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &timerInterrupt, true);
  timerAlarmWrite(timer, 500000, true); // 500ms
  timerAlarmEnable(timer);

  // Inicializa saídas
  ledcWrite(PWM_Cooler_Ch, 0);
  ledcWrite(PWM_Resistor_Ch, 0);  
}

void loop() {

  if (flagTimer) {
    flagTimer = false;

    // --- Controle de Vazão (500ms) ---
    noInterrupts();
    uint32_t pulsosAtuais = pulsos;
    pulsos = 0;
    interrupts();

    // Se for a primeira leitura, segurar 3 segundos
    if (tempoAnterior == 0) {
      delay(4000);
    }
    
    dutyCycleCooler = 255;
    dutyCycleResistor = 0;
    ledcWrite(PWM_Resistor_Ch, dutyCycleResistor);
    ledcWrite(PWM_Cooler_Ch, dutyCycleCooler);

    // Log de dados
    Serial.print (millis());
    Serial.print (",");
    Serial.println (pulsos);
    // Reseta contador de pulsos e atualiza tempo
    pulsos = 0;
   
    // Reativa a interrupção (caso tenha sido desativada)
    attachInterrupt(digitalPinToInterrupt(pinSensorVazao), contarPulsos, RISING);
  }
}