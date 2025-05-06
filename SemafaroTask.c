#include <stdio.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "FreeRTOS.h"
#include "task.h"

#include "lib/matrixws.h"
#include "lib/rgb.h"
#include "lib/display_init.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/buzzer.h"

// Button A definitions
#define BOTAO_A             5
#define DEBOUNCE_DELAY_MS   200

// State durations (ms)
#define GREEN_DURATION_MS   5000
#define YELLOW_DURATION_MS  2000
#define RED_DURATION_MS     5000

// Beep pattern timings (ms)
#define BEEP_GREEN_MS       1000
#define BEEP_YELLOW_ON_MS   100
#define BEEP_YELLOW_OFF_MS  100
#define BEEP_RED_ON_MS      500
#define BEEP_RED_OFF_MS     1500

// Night mode timings
#define NIGHT_BLINK_MS          1000
#define NIGHT_BEEP_MS           100
#define NIGHT_BEEP_PERIOD_MS    2000

// Delay slice for responsive TaskMatrix
#define MATRIX_SLICE_MS         100

// FreeRTOS priorities
#define PRIORITY_MATRIX     (tskIDLE_PRIORITY + 2)
#define PRIORITY_RGB        (tskIDLE_PRIORITY + 2)
#define PRIORITY_BUZZER     (tskIDLE_PRIORITY + 2)
#define PRIORITY_DISPLAY    (tskIDLE_PRIORITY + 1)

// Modes and states
typedef enum { MODE_NORMAL = 0, MODE_NOTURNO } Mode_t;
typedef enum { STATE_GREEN = 0, STATE_YELLOW, STATE_RED } State_t;

// Global shared variables
volatile Mode_t currentMode = MODE_NORMAL;
volatile State_t currentState = STATE_GREEN;
volatile uint32_t lastButtonATime = 0;

// Button A interrupt callback (debounced)
void buttonA_callback(uint gpio, uint32_t events) {
    if (gpio == BOTAO_A && (events & GPIO_IRQ_EDGE_FALL)) {
        uint32_t now = time_us_32() / 1000;
        if ((now - lastButtonATime) > DEBOUNCE_DELAY_MS) {
            lastButtonATime = now;
            currentMode = (currentMode == MODE_NORMAL) ? MODE_NOTURNO : MODE_NORMAL;
        }
    }
}

// Task prototypes
void TaskMatrix(void *pvParameters);
void TaskRGB(void *pvParameters);
void TaskDisplay(void *pvParameters);
void TaskBuzzer(void *pvParameters);

int main() {
    stdio_init_all();  // Inicializa comunicação serial

    // Inicialização dos periféricos
    controle(PINO_MATRIZ);       // Configura matriz WS2812
    set_brilho(BRILHO_PADRAO);
    iniciar_rgb();               // Configura LED RGB
    display();                   // Inicializa display OLED
    buzzer_init(buzzer, 1000);   // Inicializa buzzer

    // Configuração do botão A para alternância de modo
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, buttonA_callback);

    // Criação das tarefas FreeRTOS
    xTaskCreate(TaskMatrix,  "MatrixTask",   256, NULL, PRIORITY_MATRIX,  NULL);
    xTaskCreate(TaskRGB,     "RGBTask",      256, NULL, PRIORITY_RGB,     NULL);
    xTaskCreate(TaskDisplay, "DisplayTask",  512, NULL, PRIORITY_DISPLAY, NULL);
    xTaskCreate(TaskBuzzer,  "BuzzerTask",   512, NULL, PRIORITY_BUZZER,  NULL);

    vTaskStartScheduler();  // Inicia escalonador

    while (1) asm("wfi"); // Nunca deve chegar aqui
}

// TaskMatrix atualizada: verifica currentMode a cada pedaço de 100ms para mudança imediata
void TaskMatrix(void *pvParameters) {
    (void) pvParameters;
    while (1) {
        if (currentMode == MODE_NORMAL) {
            // Fase Verde
            currentState = STATE_GREEN;
            for (int i = 0; i < NUM_LEDS; ++i) cores(i, 0, BRILHO_PADRAO, 0);
            bf();
            for (int elapsed = 0; elapsed < GREEN_DURATION_MS; elapsed += MATRIX_SLICE_MS) {
                vTaskDelay(pdMS_TO_TICKS(MATRIX_SLICE_MS));
                if (currentMode != MODE_NORMAL) break;
            }
            if (currentMode != MODE_NORMAL) continue;

            // Fase Amarela
            currentState = STATE_YELLOW;
            for (int i = 0; i < NUM_LEDS; ++i) cores(i, BRILHO_PADRAO, BRILHO_PADRAO, 0);
            bf();
            for (int elapsed = 0; elapsed < YELLOW_DURATION_MS; elapsed += MATRIX_SLICE_MS) {
                vTaskDelay(pdMS_TO_TICKS(MATRIX_SLICE_MS));
                if (currentMode != MODE_NORMAL) break;
            }
            if (currentMode != MODE_NORMAL) continue;

            // Fase Vermelha
            currentState = STATE_RED;
            for (int i = 0; i < NUM_LEDS; ++i) cores(i, BRILHO_PADRAO, 0, 0);
            bf();
            for (int elapsed = 0; elapsed < RED_DURATION_MS; elapsed += MATRIX_SLICE_MS) {
                vTaskDelay(pdMS_TO_TICKS(MATRIX_SLICE_MS));
                if (currentMode != MODE_NORMAL) break;
            }
            if (currentMode != MODE_NORMAL) continue;

        } else {
            // Modo Noturno: pisca amarelo lentamente
            for (int i = 0; i < NUM_LEDS; ++i) cores(i, BRILHO_PADRAO, BRILHO_PADRAO, 0);
            bf();
            for (int elapsed = 0; elapsed < NIGHT_BLINK_MS; elapsed += MATRIX_SLICE_MS) {
                vTaskDelay(pdMS_TO_TICKS(MATRIX_SLICE_MS));
                if (currentMode != MODE_NOTURNO) break;
            }
            desliga();
            for (int elapsed = 0; elapsed < NIGHT_BLINK_MS; elapsed += MATRIX_SLICE_MS) {
                vTaskDelay(pdMS_TO_TICKS(MATRIX_SLICE_MS));
                if (currentMode != MODE_NOTURNO) break;
            }
        }
    }
}

// TaskRGB: controla acendimento do LED RGB segundo currentState e currentMode
void TaskRGB(void *pvParameters) {
    (void) pvParameters;
    bool ledOn = false;
    State_t lastState = -1;
    Mode_t lastMode = -1;
    while (1) {
        if (currentMode == MODE_NORMAL) {
            if (lastMode != MODE_NORMAL || lastState != currentState) {
                lastMode  = MODE_NORMAL;
                lastState = currentState;
                switch (currentState) {
                    case STATE_GREEN:  state(false, true,  false); break;
                    case STATE_YELLOW: state(true,  true,  false); break;
                    case STATE_RED:    state(true,  false, false); break;
                }
            }
        } else {
            // Pisca amarelo no modo noturno
            ledOn = !ledOn;
            state(ledOn, ledOn, false);
            vTaskDelay(pdMS_TO_TICKS(NIGHT_BLINK_MS));
            continue;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// TaskDisplay: atualiza display OLED com modo e estado atuais
void TaskDisplay(void *pvParameters) {
    (void) pvParameters;
    char buf[16];
    while (1) {
        ssd1306_fill(&ssd, false);  // Limpa tela
        sprintf(buf, "%s", currentMode == MODE_NORMAL ? "NORMAL" : "NOTURNO");
        ssd1306_draw_string(&ssd, buf, 0, 0);
        if (currentMode == MODE_NORMAL) {
            sprintf(buf, "%s", currentState == STATE_GREEN  ? "VERDE"  :
                               currentState == STATE_YELLOW ? "AMARELO": "VERMELHO");
            ssd1306_draw_string(&ssd, buf, 0, 10);
        }
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// TaskBuzzer: emite sons conforme estado do semáforo ou modo noturno (versão final)
void TaskBuzzer(void *pvParameters) {
    (void) pvParameters;
    TickType_t lastStateChangeTime = xTaskGetTickCount();
    State_t lastState = currentState;
    Mode_t lastMode = currentMode;
    bool firstIteration = true;

    while (1) {
        if (currentMode != lastMode || currentState != lastState) {
            // Estado ou modo mudou - resetar variáveis
            lastStateChangeTime = xTaskGetTickCount();
            lastState = currentState;
            lastMode = currentMode;
            firstIteration = true;
            buzzer_stop(buzzer); // Garante que o buzzer pare ao mudar de estado
        }

        if (currentMode == MODE_NORMAL) {
            switch (currentState) {
                case STATE_GREEN:
                    if (firstIteration) {
                        // Toca apenas uma vez no início do estado verde
                        buzzer_set_freq(buzzer, 1000);
                        vTaskDelay(pdMS_TO_TICKS(BEEP_GREEN_MS));
                        buzzer_stop(buzzer);
                        firstIteration = false;
                    }
                    vTaskDelay(pdMS_TO_TICKS(10)); // Pequeno delay para evitar uso excessivo da CPU
                    break;

                case STATE_YELLOW:
                    // Bip contínuo (100ms ligado, 100ms desligado)
                    buzzer_set_freq(buzzer, 1500);
                    vTaskDelay(pdMS_TO_TICKS(BEEP_YELLOW_ON_MS));
                    buzzer_stop(buzzer);
                    vTaskDelay(pdMS_TO_TICKS(BEEP_YELLOW_OFF_MS));
                    break;

                case STATE_RED:
                    // Bip lento (500ms ligado, 1500ms desligado)
                    buzzer_set_freq(buzzer, 500);
                    vTaskDelay(pdMS_TO_TICKS(BEEP_RED_ON_MS));
                    buzzer_stop(buzzer);
                    vTaskDelay(pdMS_TO_TICKS(BEEP_RED_OFF_MS));
                    break;
            }
        } else {
            // Modo noturno - bip a cada 2 segundos
            buzzer_set_freq(buzzer, 1000);
            vTaskDelay(pdMS_TO_TICKS(NIGHT_BEEP_MS));
            buzzer_stop(buzzer);
            vTaskDelay(pdMS_TO_TICKS(NIGHT_BEEP_PERIOD_MS - NIGHT_BEEP_MS));
        }
    }
}