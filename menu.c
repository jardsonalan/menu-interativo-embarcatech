#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

// Pinos e módulos controlador i2c selecionado
#define I2C_PORT i2c1
#define PINO_SCL 14
#define PINO_SDA 15

#define BUZZER_PIN 21

const int VRX = 26;
const int VRY = 27;
const int ADC_CHANNEL_0 = 0;
const int ADC_CHANNEL_1 = 1;

// Botão do Joystick
const int SW = 22;

// Definição dos LEDs RGB
const uint BLUE_LED_PIN= 12; // LED azul no GPIO 12
const uint RED_LED_PIN  = 13; // LED vermelho no GPIO 13
const uint GREEN_LED_PIN = 11; // LED verde no GPIO 11
const float DIVIDER_PWM = 16.0;
const uint16_t PERIOD = 2000;
const uint16_t LED_STEP = 100;
uint16_t led_b_level, led_r_level = 100;
uint slice_led_b, slice_led_r;

// Variável para controlar se o buzzer está ativo ou desligado
volatile bool stop_star_wars = false;

// Variável para armazenar a posição do seletor do display
uint pos_y=12;

ssd1306_t disp;

// Notas musicais para a música tema de Star Wars - Buzzer
const uint star_wars_notes[] = {
    330, 330, 330, 262, 392, 523, 330, 262,
    392, 523, 330, 659, 659, 659, 698, 523,
    415, 349, 330, 262, 392, 523, 330, 262,
    392, 523, 330, 659, 659, 659, 698, 523,
    415, 349, 330, 523, 494, 440, 392, 330,
    659, 784, 659, 523, 494, 440, 392, 330,
    659, 659, 330, 784, 880, 698, 784, 659,
    523, 494, 440, 392, 659, 784, 659, 523,
    494, 440, 392, 330, 659, 523, 659, 262,
    330, 294, 247, 262, 220, 262, 330, 262,
    330, 294, 247, 262, 330, 392, 523, 440,
    349, 330, 659, 784, 659, 523, 494, 440,
    392, 659, 784, 659, 523, 494, 440, 392
};

// Duração das notas em milissegundos - Buzzer
const uint note_duration[] = {
    500, 500, 500, 350, 150, 300, 500, 350,
    150, 300, 500, 500, 500, 500, 350, 150,
    300, 500, 500, 350, 150, 300, 500, 350,
    150, 300, 650, 500, 150, 300, 500, 350,
    150, 300, 500, 150, 300, 500, 350, 150,
    300, 650, 500, 350, 150, 300, 500, 350,
    150, 300, 500, 500, 500, 500, 350, 150,
    300, 500, 500, 350, 150, 300, 500, 350,
    150, 300, 500, 350, 150, 300, 500, 500,
    350, 150, 300, 500, 500, 350, 150, 300,
};

// Função para inicialização de todos os recursos do sistema
void inicializa(){
    stdio_init_all();
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    i2c_init(I2C_PORT, 400*1000);// I2C Inicialização. Usando 400Khz.
    gpio_set_function(PINO_SCL, GPIO_FUNC_I2C);
    gpio_set_function(PINO_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(PINO_SCL);
    gpio_pull_up(PINO_SDA);
    disp.external_vcc=false;
    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT);

// Inicialização dos LEDs
    gpio_init(RED_LED_PIN);
    gpio_init(GREEN_LED_PIN);
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);

    // Inicialmente, desligar o LED RGB
    gpio_put(RED_LED_PIN, 0);
    gpio_put(GREEN_LED_PIN, 0);
    gpio_put(BLUE_LED_PIN, 0);

    // Botão do Joystick
    gpio_init(SW); // Inicializa o pino do botão
    gpio_set_dir(SW, GPIO_IN); // Configura o pino do botão como entrada
    gpio_pull_up(SW);
    
    adc_init();         // Inicializa o módulo ADC
    adc_gpio_init(VRX); // Configura o pino VRX (eixo X) para entrada ADC
    adc_gpio_init(VRY); // Configura o pino VRY (eixo Y) para entrada ADC

    // Inicializa o pino do botão do joystick
    gpio_init(SW); // Inicializa o pino do botão
    gpio_set_dir(SW, GPIO_IN); // Configura o pino do botão como entrada
    gpio_pull_up(SW);
}

// Função para configurar o PWM de um LED (genérica para azul e vermelho) - Joystick LED
void setup_pwm_led(uint led, uint *slice, uint16_t level)
{
  gpio_set_function(led, GPIO_FUNC_PWM); // Configura o pino do LED como saída PWM
  *slice = pwm_gpio_to_slice_num(led); // Obtém o slice do PWM associado ao pino do LED
  pwm_set_clkdiv(*slice, DIVIDER_PWM); // Define o divisor de clock do PWM
  pwm_set_wrap(*slice, PERIOD); // Configura o valor máximo do contador (período do PWM)
  pwm_set_gpio_level(led, level); // Define o nível inicial do PWM para o LED
  pwm_set_enabled(*slice, true); // Habilita o PWM no slice correspondente ao LED
}

// Função de configuração geral - Joystick LED
void setup()
{
  setup_pwm_led(BLUE_LED_PIN, &slice_led_b, led_b_level); // Configura o PWM para o LED azul
  setup_pwm_led(RED_LED_PIN, &slice_led_r, led_r_level); // Configura o PWM para o LED vermelho
}

// Função para ler os valores dos eixos do joystick (X e Y) - Joystick LED
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value)
{
  // Leitura do valor do eixo X do joystick
  adc_select_input(ADC_CHANNEL_0); // Seleciona o canal ADC para o eixo X
  sleep_us(2);                     // Pequeno delay para estabilidade
  *vrx_value = adc_read();         // Lê o valor do eixo X (0-4095)

  // Leitura do valor do eixo Y do joystick
  adc_select_input(ADC_CHANNEL_1); // Seleciona o canal ADC para o eixo Y
  sleep_us(2);                     // Pequeno delay para estabilidade
  *vry_value = adc_read();         // Lê o valor do eixo Y (0-4095)
}

// Inicializa o PWM no pino do buzzer - Buzzer
void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f); // Ajusta divisor de clock
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0); // Desliga o PWM inicialmente
}

// Toca uma nota com a frequência e duração especificadas - Buzzer
void play_tone(uint pin, uint frequency, uint duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t top = clock_freq / frequency - 1;

    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(pin, top / 2); // 50% de duty cycle

    sleep_ms(duration_ms);

    pwm_set_gpio_level(pin, 0); // Desliga o som após a duração
    sleep_ms(50); // Pausa entre notas
}

// Função principal para tocar a música - Buzzer
void play_star_wars(uint pin) {
    for (int i = 0; i < sizeof(star_wars_notes) / sizeof(star_wars_notes[0]); i++) {
        if (stop_star_wars) {
            break; // Sai do loop se a variável stop_star_wars for verdadeira
        }
        if (star_wars_notes[i] == 0) {
            sleep_ms(note_duration[i]);
        } else {
            play_tone(pin, star_wars_notes[i], note_duration[i]);
        }
    }
}

// Led RGB
void setup_pwm()
{
    uint slice;
    gpio_set_function(RED_LED_PIN, GPIO_FUNC_PWM); // Configura o pino do LED para função PWM
    slice = pwm_gpio_to_slice_num(RED_LED_PIN); // Obtém o slice do PWM associado ao pino do LED
    pwm_set_clkdiv(slice, DIVIDER_PWM); // Define o divisor de clock do PWM
    pwm_set_wrap(slice, PERIOD); // Configura o valor máximo do contador (período do PWM)
    pwm_set_gpio_level(RED_LED_PIN, led_r_level); // Define o nível inicial do PWM para o pino do LED
    pwm_set_enabled(slice, true); // Habilita o PWM no slice correspondente
}

// Função escrita no display.
void print_texto(char *msg, uint pos_x, uint pos_y, uint scale){
    ssd1306_draw_string(&disp, pos_x, pos_y, scale, msg);// Desenha texto
    ssd1306_show(&disp);// Apresenta no Oled
}

// O desenho do retangulo fará o papel de seletor
void print_retangulo(int x1, int y1, int x2, int y2){
    ssd1306_draw_empty_square(&disp,x1,y1,x2,y2);
    ssd1306_show(&disp);
}

// Função para executar a opção do Joystick LED
void joystick_led(){
    uint16_t vrx_value, vry_value, sw_value; // Variáveis para armazenar os valores do joystick (eixos X e Y) e botão
    setup(); // Chama a função de configuração
    // Loop principal
    while (1)
    {
        joystick_read_axis(&vrx_value, &vry_value); // Lê os valores dos eixos do joystick
        // Ajusta os níveis PWM dos LEDs de acordo com os valores do joystick
        pwm_set_gpio_level(BLUE_LED_PIN, vrx_value); // Ajusta o brilho do LED azul com o valor do eixo X
        pwm_set_gpio_level(RED_LED_PIN, vry_value); // Ajusta o brilho do LED vermelho com o valor do eixo Y

        // Pequeno delay antes da próxima leitura
        sleep_ms(100); // Espera 100 ms antes de repetir o ciclo

        if (gpio_get(SW) == 0) {
            joystick_read_axis(&vrx_value, &vry_value); // Lê os valores dos eixos do joystick
            // Ajusta os níveis PWM dos LEDs de acordo com os valores do joystick
            pwm_set_gpio_level(BLUE_LED_PIN, 0); // Ajusta o brilho do LED azul com o valor do eixo X
            pwm_set_gpio_level(RED_LED_PIN, 0); // Ajusta o brilho do LED vermelho com o valor do eixo Y
            break;
        }
    }
}

// Função para executar a opção do Buzzer
void tocar_buzzer(){
    stop_star_wars = false; // Reseta a variável stop_star_wars
    for (int i = 0; i < sizeof(star_wars_notes) / sizeof(star_wars_notes[0]); i++) {
        if (stop_star_wars) {
            break; // Sai do loop se a variável stop_star_wars for verdadeira
        }
        if (star_wars_notes[i] == 0) {
            sleep_ms(note_duration[i]);
        } else {
            play_tone(BUZZER_PIN, star_wars_notes[i], note_duration[i]);
        }
        if (gpio_get(SW) == 0) {
            stop_star_wars = true; // Define a variável stop_star_wars como verdadeira quando o botão do joystick for pressionado
        }
    }
}

// Função para executar a opção do Led RGB
void led_rgb(){
    uint up_down = 1;
    setup_pwm();

    while (true)
    {
        pwm_set_gpio_level(RED_LED_PIN, led_r_level); // Define o nível atual do PWM (duty
        sleep_ms(100);
        if (up_down)
        {
            led_r_level += LED_STEP; // Incrementa o nível do LED
            if (gpio_get(SW) == 0) {
                pwm_set_gpio_level(RED_LED_PIN, 0);
                break;
            } else {
                if (led_r_level >= PERIOD)
                    up_down = 0; // Muda direção para diminuir quando atingir o período máximo
            }
        }
        else
        {
            led_r_level -= LED_STEP; // Decrementa o nível do LED
            if (gpio_get(SW) == 0) {
                pwm_set_gpio_level(RED_LED_PIN, 0);
                break;
            } else {
                if (led_r_level <= LED_STEP)
                    up_down = 1; // Muda direção para aumentar quando atingir o mínimo
            }
        }
    }
}

int main()
{
    inicializa();
    pwm_init_buzzer(BUZZER_PIN);
    char *text = ""; // Texto do menu
    uint countdown = 0; // Verificar seleções para baixo do joystick
    uint countup = 2; // Verificar seleções para cima do joystick
    uint adc_y_raw_old = 0;
   
    while(true){
        adc_select_input(0);
        uint adc_y_raw = adc_read();
        const uint bar_width = 40;
        const uint adc_max = (1 << 12) - 1;
        uint bar_y_pos = adc_y_raw * bar_width / adc_max + 1; // Posição do joystick

        // Só atualiza se houver mudança significativa no joystick
        if (abs((int)bar_y_pos - (int)adc_y_raw_old) > 1) { 

            // Limpa e redesenha o menu
            ssd1306_clear(&disp);
            print_texto(text="Menu", 52, 2, 1);
            print_retangulo(2, pos_y, 120, 18);
            print_texto(text="Joystick LED", 6, 18, 1.5);
            print_texto(text="Tocar Buzzer", 6, 30, 1.5);
            print_texto(text="LED RGB", 6, 42, 1.5);

            // Verifica movimento do joystick
            if (bar_y_pos < 20 && countdown < 2) {
                pos_y += 12;
                countdown += 1;
                countup -= 1;
            } else if (bar_y_pos > 20 && countup < 2) {
                pos_y -= 12;
                countup += 1;
                countdown -= 1;
            }

            adc_y_raw_old = bar_y_pos;
        }

        sleep_ms(100);

        // Verifica se botão foi pressionado. Se sim, entra no switch case para verificar posição do seletor e chama acionamento dos leds.
        if(gpio_get(SW) == 0){
            switch (pos_y){
                case 12:
                    joystick_led();
                    break;
                case 24:
                    tocar_buzzer();
                    break;
                case 36:
                    led_rgb();
                    break;
                default:
                    break;
            }
       }
    }
    return 0;
}
