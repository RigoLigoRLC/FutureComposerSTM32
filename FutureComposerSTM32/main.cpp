#include "RTE_Components.h"
#include <arm_acle.h>
#include CMSIS_device_header
#include "fc14synthesizer.hpp"
#include "music_data.h"

// Sigma embedded devs never needed a heap
__asm(".global __use_no_heap\n");

#define ARRAY_SIZE(X) (sizeof(X)/sizeof((X)[0]))

uint8_t audio_buffer[4096] = {0};
uint8_t bufferHalfToFill = 1;
uint8_t bufferNeedsFill = 0;

alignas(4) constexpr auto MusicData = Fc14ByteorderInversion<KEIL_KEYGEN_MUSIC_DATA>();
FC14Synthesizer synthesizer(MusicData);

void init_hardware();

int main() {
    init_hardware();

    // Initial synthesis
    synthesizer.synthesize(audio_buffer, sizeof(audio_buffer));

    // Start DMA and Timer
    DMA1_Channel5->CCR |= DMA_CCR5_EN;
    TIM1->CR1 |= TIM_CR1_CEN;

    while (1) {
        if (bufferNeedsFill) {
            synthesizer.synthesize(audio_buffer + bufferHalfToFill * sizeof(audio_buffer) / 2,
                                   sizeof(audio_buffer) / 2);
            bufferNeedsFill = 0;
        }
    }
}

void init_hardware() {
    // We use TIM1 @ 72MHz, sample rate @ 28125 Hz
    // ARR = 0xFF (Uses full 8-bit values)
    // PSC = 0
    // Compare cycle = 72M / (PSC)1 / (ARR)256 / (RCR)10 = 28125 Hz, Repetition = 10, RCR = 9
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    TIM1->CR1 = TIM_CR1_URS; // DIR = 0: Up counter; URS: update event on overflow only
    TIM1->CR2 = 0; // Features unused
    TIM1->SMCR = 0; // Slave mode unused
    TIM1->DIER = TIM_DIER_UDE; // DMARQ on update event
    TIM1->CCMR1 = TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // OC1M=111 PWM Mode 2
    TIM1->CCER = TIM_CCER_CC1E; // Enable Capture/Compare Channel 1
    TIM1->CNT = 0; // Reset value
    TIM1->PSC = 0;
    TIM1->ARR = 0xFF;
    TIM1->RCR = 9;
    TIM1->DCR = TIM_DCR_DBL_0 | TIM_DCR_DBA_4; // Offset 16 = CCR1, Burst length = 1
    TIM1->BDTR = TIM_BDTR_MOE; // Enable master output switch

    // GPIO
    // TIM1 CH1 PA8
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN; // Enable GPIOA and AFIO
    GPIOA->CRH = GPIO_CRH_CNF8_1 | GPIO_CRH_MODE8; // PA8 AF-PP, 50MHz

    // DMA
    // Use DMA1 CH5
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop();
    DMA1_Channel5->CCR = DMA_CCR5_CIRC | DMA_CCR5_MINC | DMA_CCR5_PSIZE_0 |
                         DMA_CCR5_DIR | DMA_CCR5_HTIE | DMA_CCR5_TCIE;
                         // CIRC: circular buffer;
                         // MINC: increment on memory side;
                         // PSIZE=1 MSIZE=0: 16bit on periph, 8 bits on mem
                         // DIR: Memory to Peripheral
                         // HTIE: Half complete interrupt enabled
                         // TCIE: Transfer complete interrupt enabled
    DMA1_Channel5->CNDTR = 4096;
    DMA1_Channel5->CPAR = uint32_t(&TIM1->CCR1);
    DMA1_Channel5->CMAR = uint32_t(&audio_buffer);

    NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

extern "C" void DMA1_Channel5_IRQHandler() {
    if (DMA1->ISR & DMA_ISR_HTIF5) {
        // DMA1 CH5 Half complete
        bufferHalfToFill = bufferHalfToFill ? 0 : 1; // Flip to another half of buffer
        bufferNeedsFill = 1;
        DMA1->IFCR = DMA_IFCR_CHTIF5;
        NVIC_ClearPendingIRQ(DMA1_Channel5_IRQn);
    } else if (DMA1->ISR & DMA_ISR_TCIF5) {
        // DMA1 CH5 Transfer complete
        bufferHalfToFill = bufferHalfToFill ? 0 : 1; // Flip to another half of buffer
        bufferNeedsFill = 1;
        DMA1->IFCR = DMA_IFCR_CTCIF5;
        NVIC_ClearPendingIRQ(DMA1_Channel5_IRQn);
    }
}

// extern "C" void *malloc(size_t s) { return 0;}
// extern "C" void free(void *) { }

// Below is used to solve the C++ standard library malloc/free usage
// Found on: https://developer.arm.com/documentation/ka005021/latest/

// override the delete operator to avoid references to free.
void operator delete(void *pP) noexcept { }

// re-implement atexit calls due to static constructors
// and avoid allocation for the static destructors.
// This disables the static destructor calls
extern "C" {
    void __aeabi_atexit(void *) { }
    // disable static destructure call collection and avoid malloc
    // use only if application never terminates
    void __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle) { }
}
