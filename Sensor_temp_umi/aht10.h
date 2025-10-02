#ifndef AHT10_H
#define AHT10_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define AHT10_ADDR 0x38

// Estrutura para armazenar os dados finais do sensor
typedef struct {
    float temperatura; // Temperatura em Graus Celsius
    float umidade;    // Umidade Relativa em %
} aht10_dado_t;

// Funções públicas
bool aht10_init(i2c_inst_t* i2c_port);
bool aht10_ler_dado(i2c_inst_t* i2c_port, aht10_dado_t* dado);

#endif