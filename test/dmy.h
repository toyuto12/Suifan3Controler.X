#ifndef DMY_H
#define DMY_H

#include <stdint.h>
#include <stdbool.h>

typedef union {
    struct {
        unsigned RA0                    :1;
        unsigned RA1                    :1;
        unsigned RA2                    :1;
        unsigned RA3                    :1;
        unsigned RA4                    :1;
        unsigned RA5                    :1;
        unsigned RA6                    :1;
        unsigned RA7                    :1;
    };
} PORTAbits_t;
PORTAbits_t PORTAbits;

typedef union {
    struct {
        unsigned ON                     :1;
        unsigned RD16                   :1;
        unsigned nSYNC                  :1;
        unsigned                        :1;
        unsigned CKPS                   :2;
    };
    struct {
        unsigned TMR1ON                 :1;
        unsigned T1RD16                 :1;
        unsigned nT1SYNC                :1;
        unsigned                        :1;
        unsigned T1CKPS0                :1;
        unsigned T1CKPS1                :1;
    };
    struct {
        unsigned                        :4;
        unsigned CKPS0                  :1;
        unsigned CKPS1                  :1;
    };
    struct {
        unsigned                        :1;
        unsigned RD161                  :1;
    };
} T1CONbits_t;
T1CONbits_t T1CONbits;

typedef union {
    struct {
        unsigned LATB0                  :1;
        unsigned LATB1                  :1;
        unsigned LATB2                  :1;
        unsigned LATB3                  :1;
        unsigned LATB4                  :1;
        unsigned LATB5                  :1;
        unsigned LATB6                  :1;
        unsigned LATB7                  :1;
    };
} LATBbits_t;
LATBbits_t LATBbits;

typedef union {
    struct {
        unsigned LATC0                  :1;
        unsigned LATC1                  :1;
        unsigned LATC2                  :1;
        unsigned LATC3                  :1;
        unsigned LATC4                  :1;
        unsigned LATC5                  :1;
        unsigned LATC6                  :1;
        unsigned LATC7                  :1;
    };
} LATCbits_t;
LATCbits_t LATCbits;

typedef union {
    struct {
        unsigned TX9D                   :1;
        unsigned TRMT                   :1;
        unsigned BRGH                   :1;
        unsigned SENDB                  :1;
        unsigned SYNC                   :1;
        unsigned TXEN                   :1;
        unsigned TX9                    :1;
        unsigned CSRC                   :1;
    };
} TX1STAbits_t;
TX1STAbits_t TX1STAbits;

typedef union {
    struct {
        unsigned SSP1IF                 :1;
        unsigned BCL1IF                 :1;
        unsigned SSP2IF                 :1;
        unsigned BCL2IF                 :1;
        unsigned TXIF                   :1;
        unsigned RCIF                   :1;
    };
} PIR3bits_t;
PIR3bits_t PIR3bits;

uint8_t TX1REG,TMR1L,TMR1H;
uint16_t TMR1;

#endif
