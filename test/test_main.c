#include <stdbool.h>
#include <stdint.h>

#define EQUAL cut_assert_equal_int

typedef struct{
	uint8_t FanLevel	:2 ;
	uint8_t LRFanOn		:1 ;
	uint8_t UDFanOn		:1 ;
	uint8_t Check1		:4 ;
}stCommData;

typedef struct{
	uint8_t FanOn	:1;
	uint8_t UDOn	:1;
	uint8_t LROn	:1;
	uint8_t Timer	:1;
	uint8_t FanMove	:1;
	uint8_t Check1	:3;
}stInput;

uint8_t CheckTimerLED(){
    uint8_t val = 0;
    val |= (LATBbits.LATB4) ?0x01 :0 ;
    val |= (LATBbits.LATB5) ?0x02 :0 ;
    val |= (LATBbits.LATB2) ?0x04 :0 ;
    val |= (LATBbits.LATB3) ?0x08 :0 ;
    return val;
}

void DispOffTimerLevel( uint16_t offTimerValue );
void test_DispOffTimerLevel(){
    DispOffTimerLevel(0);
    EQUAL( 0, CheckTimerLED() );
    DispOffTimerLevel(1);
    EQUAL( 1, CheckTimerLED() );
    DispOffTimerLevel(7200);
    EQUAL( 1, CheckTimerLED() );
    DispOffTimerLevel(7201);
    EQUAL( 2, CheckTimerLED() );
    DispOffTimerLevel(14400);
    EQUAL( 2, CheckTimerLED() );
    DispOffTimerLevel(14401);
    EQUAL( 4, CheckTimerLED() );
    DispOffTimerLevel(21600);
    EQUAL( 4, CheckTimerLED() );
    DispOffTimerLevel(21601);
    EQUAL( 8, CheckTimerLED() );
    DispOffTimerLevel(28800);
    EQUAL( 8, CheckTimerLED() );
}

// Code
// 0:None 1:MainFanOn 2:MainFanMove 3:Timer 4:LeftRightFan 5:UpDownFan

extern uint16_t sOffCount;
void ContFanValue( uint8_t code, stInput *in, stCommData *out );
void test_ContFanValue(){
    stCommData o;   stInput i;
    uint8_t *po = &o,*pi = &i, n;

    // stInputによる操�?
    i.FanMove = 1;
    ContFanValue( 0, &i, &o );
    EQUAL( 0, *po );                // FAN_OFF
    *pi = 0; i.FanOn = 1;
    ContFanValue( 0, &i, &o );
    EQUAL( 1, *po );                // FAN_ON LV1
    *pi = 0; i.FanMove = 1;
    ContFanValue( 0, &i, &o );
    EQUAL( 2, *po );                //FAN_ON LV2
    ContFanValue( 0, &i, &o );
    EQUAL( 3, *po );                //FAN_ON LV3
    ContFanValue( 0, &i, &o );
    EQUAL( 1, *po );                //FAN_ON LV1

    *pi = 0; i.LROn = 1;
    ContFanValue( 0, &i, &o );
    EQUAL( 0x05, *po );             // LRFAN_ON
    *pi = 0; i.UDOn = 1;
    ContFanValue( 0, &i, &o );
    EQUAL( 0x0d, *po );             // UDFAN_ON(押しっぱな�?) / LRFAN_ON(継�?)
    *pi = 0;
    ContFanValue( 0, &i, &o );
    EQUAL( 0x05, *po );             // UDFAN_OFF
    i.FanOn = 1;
    ContFanValue( 0, &i, &o );
    EQUAL( 0, *po );                // FAN_OFF

    *pi = 0; i.Timer = 1; sOffCount = 100;
    ContFanValue( 0, &i, &o );
    EQUAL( 7200, sOffCount );       // OFFTIMER_COUNTUP

    // コードによる操�?
    *pi = 0;
    ContFanValue( 2, &i, &o );
    EQUAL( 0, *po );                // FAN_OFF
    ContFanValue( 1, &i, &o );
    EQUAL( 1, *po );                // FAN_ON LV1
    ContFanValue( 2, &i, &o );
    EQUAL( 2, *po );                // FAN_ON LV2
    ContFanValue( 2, &i, &o );
    EQUAL( 2, *po );                // FAN_ON LV2(連続データ弾�?)
    ContFanValue( 0, &i, &o );
    ContFanValue( 2, &i, &o );
    EQUAL( 3, *po );                // FAN_ON LV3
    ContFanValue( 3, &i, &o );
    EQUAL( 14400, sOffCount );      // OFFTIMER_COUNTUP
    ContFanValue( 4, &i, &o );
    EQUAL( 7, *po );                //  LRFAN_ON
    ContFanValue( 5, &i, &o );
    EQUAL( 15, *po );               // UPFAN_ON(押しっぱな�?) / LRFAN_ON?��継続�?
    ContFanValue( 5, &i, &o );
    EQUAL( 15, *po );               // UPFAN_ON
    ContFanValue( 4, &i, &o );
    EQUAL( 3, *po );                // UPFAN_OFF / LRFAN_OFF

}

void TaskOfftimer(void);
uint8_t IsOffTime(void);
void test_OffTimer(){
    uint8_t i;
    sOffCount = 100;

    cut_assert_false( IsOffTime() );
    for(i=0; i<100; i++ ){
        TMR1 = 0xFFFF;
        TaskOfftimer();
    }
    cut_assert_true( IsOffTime(),  cut_message("%d",sOffCount));
    cut_assert_false( IsOffTime() );
    TMR1 = 0xFFFF;
    TaskOfftimer();
    EQUAL( 0, sOffCount );
    
}

extern uint16_t HighWidth,LowWidth;
extern uint8_t IsIrReceived,IrPos,IrTmp;
void TaskIrReceive(void);
void ReadIrData( uint8_t *rev );

void SetIrPulse( uint16_t lowUs, uint16_t highUs ){
    HighWidth = highUs*4;
    LowWidth = lowUs*4;
    TaskIrReceive();
}

void test_TaskIrReceive(){
    uint8_t val,i;
    uint32_t testData = 0x55aa33cc;

    // HeaderPulse False
    SetIrPulse( 562*16, 562*9 );
    EQUAL( 0xFF, IrPos);
    SetIrPulse( 562*16, 562*11 );
    EQUAL( 0xFF, IrPos);
    SetIrPulse( 562*18, 562*8 );
    EQUAL( 0xFF, IrPos);
    SetIrPulse( 562*14, 562*8 );
    EQUAL( 0xFF, IrPos);

    // HeaderPulse True
    SetIrPulse( 562*16, 562*8 );
    EQUAL( 0, IrPos);

    for( i=0; i<32; i++ ){
        SetIrPulse( 562, (testData&(1<<i) ?562*3 :562 ));
    }
    val = 0;
    ReadIrData( &val );
    EQUAL( 0xaa, val );
//    EQUAL( 0xFF, IrPos );
    EQUAL( 0xdF, IrPos );

    uint16_t cnt = 0;
    while( val && (cnt<10000) ){
        cnt ++;
        ReadIrData( &val );
    }
    cut_assert_true( cnt>200 && cnt<300 );

}



