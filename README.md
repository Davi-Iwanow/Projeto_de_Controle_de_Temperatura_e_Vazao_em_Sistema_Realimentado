
# 🌡️💧 Projeto de Controle de Temperatura e Vazão em Sistema Realimentado

Este repositório documenta todas as etapas do desenvolvimento de um **sistema realimentado de controle de temperatura e vazão**, incluindo desde o projeto eletrônico até a implementação digital e análise de resultados.

---

## 📋 Descrição

Neste projeto, buscamos controlar com precisão dois processos físicos – **temperatura** e **vazão** – utilizando técnicas modernas de modelagem e controle digital. A proposta segue o ciclo completo de desenvolvimento de um sistema realimentado, unindo teoria e prática em um sistema físico real.

---

## 🚀 Etapas do Projeto

1. **🔌 Projeto da PCB**  
   Desenvolvimento da placa de circuito impresso para integração dos sensores, atuadores e microcontrolador.

2. **🧪 Ensaio das Plantas**  
   Levantamento dos dados experimentais para cada planta (temperatura e vazão) a partir de respostas a degrau.

3. **📊 Modelagem via MATLAB**  
   Obtenção das **funções de transferência** contínuas a partir dos dados coletados nos ensaios.

4. **🔁 Discretização (Método de Tustin)**  
   Conversão dos modelos contínuos para o domínio discreto, garantindo fidelidade ao sistema físico.

5. **🧠 Projeto dos Controladores**  
   Dimensionamento dos controladores digitais (PID ou outro tipo), considerando desempenho e estabilidade.

6. **🧪 Simulação**  
   Verificação dos controladores e modelos em ambiente simulado (MATLAB/Simulink), avaliando tempo de resposta, sobrelevação, erro em regime, etc.

7. **💻 Implementação Digital**  
   Programação do controlador em microcontrolador (ex: ESP32, Arduino), com amostragem adequada e controle em tempo real.

8. **📈 Resultados**  
   Coleta e análise dos dados reais com o sistema operando, comparando com as simulações para validar o projeto.

---

## 🛠️ Tecnologias Utilizadas

- MATLAB/Simulink
- C/C++ para microcontroladores
- KiCad (PCB Design)
- Sensor de temperatura e vazão
- ESP32 / Arduino

---
