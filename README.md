# Projeto de Semáforo Inteligente com FreeRTOS

## 📌 Sumário  
- [📹 Demonstração](#-Demonstração)
- [🎯 Objetivo](#-objetivo)
- [🛠️ Funcionalidades Implementadas](#️-funcionalidades-implementadas)
- [✨ Destaques](#-destaques)
- [📦 Componentes Utilizados](#-componentes-utilizados)
- [⚙️ Configuração e Compilação](#️-configuração-e-compilação)
- [📂 Estrutura do Código](#-estrutura-do-código)
- [🔧 Diagrama de Estados](#-diagrama-de-estados)
- [👨‍💻 Autor](#-autor)

## 📹 Demonstração  
[clique aqui para acessar o vídeo](https://youtu.be/7DPF8P_o5AM)

## 🎯 Objetivo
Desenvolver um sistema de semáforo inteligente utilizando a plataforma Raspberry Pi Pico W com FreeRTOS, implementando controle de estados, modo noturno e interface com periféricos.

## 🛠️ Funcionalidades Implementadas

### 🚦 Modo Normal
| Estado | Duração | LED Matrix | Buzzer | Display |
|--------|---------|------------|--------|---------|
| Verde | 5000ms | Verde | 1 beep (1000ms) | "VERDE" |
| Amarelo | 2000ms | Amarelo | Bips rápidos (100ms on/off) | "AMARELO" |
| Vermelho | 5000ms | Vermelho | Bips lentos (500ms on/1500ms off) | "VERMELHO" |

### 🌙 Modo Noturno
- Matriz LED: Pisca amarelo (1000ms on/off)
- Buzzer: Bip curto (100ms) a cada 2000ms
- Display: Mostra "MODO NOTURNO"

## ✨ Destaques
- **Arquitetura RTOS**: 4 tarefas independentes com prioridades definidas
- **Sincronização**: Variáveis globais protegidas com `volatile`
- **Responsividade**: Tempo de reação <10ms para mudanças de estado
- **Eficiência**: Baixo consumo no modo noturno

## 📦 Componentes Utilizados  
- Microcontrolador: RP2040 (BitDogLab)  
- Display: OLED SSD1306 (128x64, I2C)  
- Matriz de LEDs: WS2812B 5x5 (controlada por PIO)  
- Buzzer: Ativo com controle de frequência  
- LED RGB: Comum anódico  
- Botão: Para alternância de modos  
## ⚙️ Compilação e Gravação  
```bash
git clone https://github.com/Ronaldo8617/semaforo-freertos.git

**Gravação:**  
Pelo ambiente do VScode compile e execute na placa de desnvovimento BitDogLab
Ou
Conecte o RP2040 no modo BOOTSEL e copie o `.uf2` gerado na pasta `build` para a unidade montada.
```

## 👨‍💻 Autor  
**Nome:** Ronaldo César Santos Rocha  
**GitHub:** [Ronaldo8617]
