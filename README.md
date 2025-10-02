# 🗺💾 Sensor de Distância VL53L0X + Servo Motor + SD Card com Pico W (BitDogLab)

![Linguagem](https://img.shields.io/badge/Linguagem-C-blue.svg)
![Plataforma](https://img.shields.io/badge/Plataforma-Raspberry%20Pi%20Pico-purple.svg)
![Sensor](https://img.shields.io/badge/Sensor-VL53L0X-red.svg)
![Sensor](https://img.shields.io/badge/Sensor-AHT10-yellow.svg)
![Atuador](https://img.shields.io/badge/Servo-Motor-green.svg)
![Armazenamento](https://img.shields.io/badge/SD-Card-blue.svg)

## 📌 Sobre o Projeto

Este projeto tem como objetivo desenvolver um firmware em linguagem C para o Raspberry Pi Pico W (BitDogLab), capaz de:

- Monitorar a distância com o sensor **VL53L0X**
- Medir temperatura e umidade com o sensor **AHT10**
- Controlar um **servo motor** com base na presença detectada
- Exibir os dados em um **display OLED SSD1306**
- Registrar as informações em um **cartão SD**
- Indicar o estado com **LEDs RGB**

---
## 🛠️ Estrutura do Projeto

- `Dist_servo_tempUmi_card.c` – Programa principal que realiza a leitura de presença, Se a distância for menor que 10 cm, o servo gira para a direita (porta aberta); caso contrário, gira para a esquerda (porta fechada)
- As informações são exibidas no terminal serial, no visor OLED e gravadas no SD Card.
- `Sensor_distancia/vl53l0x.c` – Implementação do sensor de distância VL53L0X.
- `Atuador_servo/servo.c` – Controle do atuador servo motor.
- `Sensor_temp_umi/aht10.c` – Leitura de temperatura e umidade com o sensor AHT10.
- `inc/` – Arquivos de cabeçalho para o display OLED.
- `lib/` – Biblioteca FatFs para manipulação do cartão SD.
- `CMakeLists.txt` – Configuração do build com o Pico SDK.

---

## 🔌 Requisitos de Hardware

- Raspberry Pi Pico W (BitDogLab)
- Sensor de Distância VL53L0X
- Sensor de Temperatura e Umidade AHT10
- Servo Motor 9g + Adaptador
- Cartão microSD
- Display OLED SSD1306
- LEDs RGB
- Extensor I2C

---

## ⚙️ Como Usar

1. **Clone o repositório:**
   ```bash
   git clone https://github.com/tacinunesc/Dist_TempUmi_Servo_Card_Pico.git
2. **Compile o projeto com o Pico SDK:**
```bash
mkdir build
cd build
cmake ..
make
````
3. **Deploy no Pico W:**
- Pressione o botão BOOTSEL e conecte o Pico ao PC via USB.
- Copie o arquivo .uf2 gerado para o disco removível que aparecer.
- O Pico irá reiniciar e executar o firmware automaticamente.

**Funcionamento do Código**
1. Inicialização dos periféricos:
- Configura o barramento I2C0 (GPIO 0 e 1) para o VL53L0X e AHT10.
- Configura SPI para comunicação com o SD Card.
- Inicializa o display OLED SSD1306.
- Configura os pinos dos LEDs RGB.
- Inicializa PWM para controle do servo motor.
2. Leitura da distância:
- A cada ciclo, o sensor VL53L0X mede a distância.
- Se for menor que 10 cm → porta aberta → servo gira para a direita → LED verde acende.
- Se for maior ou igual a 10 cm → porta fechada → servo gira para a esquerda → LED vermelho acende.
3. Exibição e registro:
-Os dados são exibidos no terminal serial e no display OLED.
A cada leitura válida, os dados são gravados no SD Card:
  `Distância`
  `Tempertura e umidade`
  `Estado do servo (ABERTO/FECHADO)`
  `Tempo em minutos e segundos`

**📦 Dependências**
vl53l0x.h – Sensor de distância

aht10.h – Sensor de temperatura e umidade

servo.h – Controle do servo motor

ssd1306.h e ssd1306_fonts.h – Display OLED

ff.h – Biblioteca FatFs para SD Card
