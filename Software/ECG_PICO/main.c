#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "pico/time.h"
#include "ssd1306.h"

SSD1306 oled;

int vec[5];
int pos = 0;

int x = 0;

int bpm = 0;
int pico = 0;

absolute_time_t tiempoAnterior;

void iniciarAD()
{
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);
}

int leerAD()
{
    return adc_read();
}

int filtrarAD(int dato)
{
    int suma = 0;

    vec[pos] = dato;

    pos++;

    if(pos >= 5)
    {
        pos = 0;
    }

    for(int i = 0; i < 5; i++)
    {
        suma += vec[i];
    }

    return suma / 5;
}

void iniciarOLED()
{
    ssd1306_init(&oled, 128, 64, 0x3C, i2c0);
    ssd1306_clear(&oled);
}

void mostrarECG(int dato)
{
    int y;

    y = 63 - ((dato * 63) / 4095);

    ssd1306_draw_pixel(&oled, x, y);

    x++;

    if(x >= 128)
    {
        x = 0;
        ssd1306_clear(&oled);
    }
}

int calcularBPM(int dato)
{
    if(dato > 2500 && pico == 0)
    {
        absolute_time_t tiempoActual;

        tiempoActual = get_absolute_time();

        int64_t diferencia;

        diferencia =
        absolute_time_diff_us(
        tiempoAnterior,
        tiempoActual);

        if(diferencia > 300000)
        {
            bpm = 60000000 / diferencia;
            tiempoAnterior = tiempoActual;
        }

        pico = 1;
    }

    if(dato < 2500)
    {
        pico = 0;
    }

    return bpm;
}

void mostrarBPM(int ritmo)
{
    char texto[20];

    sprintf(texto, "BPM:%d", ritmo);

    ssd1306_draw_string(
        &oled,
        0,
        0,
        1,
        texto
    );

    ssd1306_show(&oled);
}

int main()
{
    int dato;
    int filtrado;
    int ritmo;

    stdio_init_all();

    i2c_init(i2c0, 400000);

    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);

    gpio_pull_up(4);
    gpio_pull_up(5);

    iniciarAD();

    iniciarOLED();

    tiempoAnterior = get_absolute_time();

    while(1)
    {
        dato = leerAD();

        filtrado = filtrarAD(dato);

        ritmo = calcularBPM(filtrado);

        mostrarECG(filtrado);

        mostrarBPM(ritmo);

        sleep_ms(5);
    }

    return 0;
}