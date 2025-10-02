// Bibliotecas padrão e específicas do Raspberry Pi Pico
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// Bibliotecas dos sensores e atuadores utilizados
#include "Sensor_distancia/vl53l0x.h"     // Sensor de distância VL53L0X
#include "Sensor_temp_umi/aht10.h"        // Sensor de temperatura e umidade AHT10
#include "Atuador_servo/servo.h"          // Controle de servo motor
#include "inc/ssd1306.h"                  // Display OLED SSD1306
#include "inc/ssd1306_fonts.h"            // Fontes para o display OLED
#include "ff.h"                           // Sistema de arquivos FatFs para cartão SD

// Definições de pinos e periféricos
#define aht_I2C    i2c0
#define aht_SDA    0
#define aht_SCL    1

#define vlx_I2C    i2c0
#define vlx_SDA    0
#define vlx_SCL    1

#define LED_VERDE     11
#define LED_VERMELHO  13

#define SPI_PORT   spi0
#define PIN_MISO   16
#define PIN_CS     17
#define PIN_SCK    18
#define PIN_MOSI   19

#define DISTANCIA_INVALIDA   2001         // Valor que representa leitura inválida
#define DISTANCIA_MAXIMA_CM  999          // Distância máxima considerada válida

FATFS fs; // Estrutura para o sistema de arquivos FatFs

// Função para registrar dados em arquivo no cartão SD
void registrar_dados(uint16_t distancia_cm, const char* estado, float temperatura, float umidade, uint64_t tempo_ms) {
    FIL arquivo;
    char linha[128], valor_str[16], unidade[4];

    // Formata a distância para cm ou metros
    if (distancia_cm < DISTANCIA_INVALIDA) {
        if (distancia_cm > 99) {
            snprintf(valor_str, sizeof(valor_str), "%.2f", distancia_cm / 100.0f);
            strcpy(unidade, "m");
        } else {
            snprintf(valor_str, sizeof(valor_str), "%d", distancia_cm);
            strcpy(unidade, "cm");
        }
    } else {
        strcpy(valor_str, "ERRO");
        strcpy(unidade, "");
    }

    // Converte tempo em milissegundos para minutos e segundos
    unsigned long minutos = tempo_ms / 60000;
    unsigned long segundos = (tempo_ms / 1000) % 60;

    // Abre ou cria arquivo para adicionar dados
    FRESULT fr = f_open(&arquivo, "tarefa_unidade_5.txt", FA_OPEN_APPEND | FA_WRITE);
    if (fr == FR_OK) {
        snprintf(linha, sizeof(linha),
            "[%02lu:%02lu] Dist: %s %s | Estado: %s | Temp: %.2f C | Umid: %.2f %%\n",
            minutos, segundos, valor_str, unidade, estado, temperatura, umidade);
        UINT bytes_escritos;
        f_write(&arquivo, linha, strlen(linha), &bytes_escritos);
        f_close(&arquivo);
    } else {
        printf("Erro ao abrir arquivo: %d\n", fr);
    }
}

// Inicializa comunicação SPI e monta o cartão SD
void inicializar_sd() {
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    FRESULT fr = f_mount(&fs, "", 1);
    if (fr != FR_OK) {
        printf("Erro ao montar SD: %d\n", fr);
    } else {
        printf("Cartão SD montado com sucesso.\n");
    }
}

// Exibe dados no display OLED
void exibir_oled(uint16_t distancia_cm, const char* estado_porta, float temperatura, float umidade) {
    char buffer[32];
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("MONITOR UNID 5", Font_6x8, White);

    // Exibe distância formatada
    if (distancia_cm < DISTANCIA_INVALIDA) {
        if (distancia_cm > 99) {
            snprintf(buffer, sizeof(buffer), "DISTANCIA: %.2f m", distancia_cm / 100.0f);
        } else {
            snprintf(buffer, sizeof(buffer), "DISTANCIA: %d cm", distancia_cm);
        }
    } else {
        snprintf(buffer, sizeof(buffer), "ERRO NA LEITURA DA DISTANCIA");
    }

    ssd1306_SetCursor(0, 16);
    ssd1306_WriteString(buffer, Font_6x8, White);

    // Exibe temperatura e umidade
    snprintf(buffer, sizeof(buffer), "TEMP: %.1f C | UMID: %.1f %%", temperatura, umidade);
    ssd1306_SetCursor(0, 32);
    ssd1306_WriteString(buffer, Font_6x8, White);

    ssd1306_UpdateScreen();
}

// Função principal
int main() {
    stdio_init_all(); // Inicializa comunicação USB
    while (!stdio_usb_connected()) sleep_ms(100); // Aguarda conexão USB

    // Inicializa I2C para sensores
    i2c_init(aht_I2C, 100 * 1000);
    gpio_set_function(aht_SDA, GPIO_FUNC_I2C);
    gpio_set_function(aht_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(aht_SDA);
    gpio_pull_up(aht_SCL);

    // Inicializa display OLED
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    // Inicializa LEDs
    gpio_init(LED_VERDE);    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(LED_VERMELHO); gpio_set_dir(LED_VERMELHO, GPIO_OUT);

    // Inicializa servo motor e cartão SD
    inicializar_pwm_servo();
    inicializar_sd();

    // Inicializa sensor de distância
    vl53l0x_dispositivo sensor;
    if (!vl53l0x_inicializar(&sensor, vlx_I2C)) {
        printf("Erro ao iniciar VL53L0X.\n");
        while (1);
    }
    vl53l0x_iniciar_continuo(&sensor, 0);

    // Inicializa sensor de temperatura e umidade
    if (!aht10_init(aht_I2C)) {
        printf("Erro ao iniciar AHT10.\n");
        while (1);
    }

    uint8_t ultima_posicao = 255; // Armazena última posição do servo

    // Loop principal
    while (1) {
        // Lê distância e tempo atual
        uint16_t distancia_cm = vl53l0x_ler_distancia_continua_cm(&sensor);
        uint64_t tempo_ms = to_ms_since_boot(get_absolute_time());

        // Lê dados de temperatura e umidade
        aht10_dado_t dado;
        if (!aht10_ler_dado(aht_I2C, &dado)) {
            printf("Erro ao ler dados do AHT10.\n");
            dado.temperatura = 0.0f;
            dado.umidade = 0.0f;
        }

        // Determina estado da porta com base na distância
        const char* estado_porta = "FECHADO";
        uint8_t nova_posicao = 2;
        if (distancia_cm < 10) {
            nova_posicao = 1;
            estado_porta = "ABERTO";
        }

        // Formata distância para exibição
        char valor_str[16], unidade[4];
        if (distancia_cm < DISTANCIA_INVALIDA) {
            if (distancia_cm > 99) {
                snprintf(valor_str, sizeof(valor_str), "%.2f", distancia_cm / 100.0f);
                strcpy(unidade, "m");
            } else {
                snprintf(valor_str, sizeof(valor_str), "%d", distancia_cm);
                strcpy(unidade, "cm");
            }
        } else {
            strcpy(valor_str, "ERRO");
            strcpy(unidade, "");
        }

        // Exibe dados no terminal
        printf("Estado: %s | Dist: %s %s | Temp: %.1f C | Umid: %.1f %%\n",
               estado_porta, valor_str, unidade, dado.temperatura, dado.umidade);

        // Atualiza display OLED
        exibir_oled(distancia_cm, estado_porta, dado.temperatura, dado.umidade);

        // Se a distância for válida, registra dados e movimenta servo
        if (distancia_cm != DISTANCIA_INVALIDA && distancia_cm <= DISTANCIA_MAXIMA_CM) {
                        registrar_dados(distancia_cm, estado_porta, dado.temperatura, dado.umidade, tempo_ms);

            // Se a posição do servo mudou, atualiza o servo
            if (nova_posicao != ultima_posicao) {
                servo_posicao(nova_posicao); // Move para nova posição
                sleep_ms(500);               // Aguarda meio segundo
                servo_posicao(0);            // Retorna à posição inicial
                ultima_posicao = nova_posicao;
            }

            // Atualiza LEDs conforme estado da porta
            gpio_put(LED_VERDE, nova_posicao == 1);     // LED verde acende se porta aberta
            gpio_put(LED_VERMELHO, nova_posicao != 1);  // LED vermelho acende se porta fechada
        }

        sleep_ms(200); // Aguarda 200 ms antes da próxima leitura
    }
}
