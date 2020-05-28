#ifndef TDD
#include "mcc_generated_files/mcc.h"
#endif

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

typedef union{
	uint32_t dat32;
	uint8_t dat8[4];
}stIrCommData;

stCommData gComm;
stInput gIn;
uint8_t gInterval;

#define iSW_TIMER	PORTAbits.RA4
#define iSW_FANUD	PORTAbits.RA5
#define iSW_FANLR	PORTAbits.RA3
#define iSW_FANMAIN	PORTAbits.RA2
#define iSW_FANON	PORTAbits.RA0

#define oLED_MAIN_HIGH	LATCbits.LATC0
#define oLED_MAIN_MID	LATCbits.LATC1
#define oLED_MAIN_LOW	LATCbits.LATC2
#define oLED_LR			LATCbits.LATC4
#define oLED_UD			LATCbits.LATC5

#define oLED_OFFTIMER2	LATBbits.LATB4
#define oLED_OFFTIMER4	LATBbits.LATB5
#define oLED_OFFTIMER6	LATBbits.LATB2
#define oLED_OFFTIMER8	LATBbits.LATB3

#define RANGE(target,val,range)	( (target > (val-range)) && (target < (val+range)) )


// Interrupt

// 1msec Interval
void isrTimer0(void){
	gInterval ++;
}

uint8_t sNowPoly;
uint16_t HighWidth,LowWidth;
uint16_t PulseWidth;
// IrPulseReceive
#ifndef TDD
void isrCcp1( uint16_t val ){
	TMR3_WriteTimer(0);
	
	if( PIR4bits.TMR3IF ){
		PIR4bits.TMR3IF = 0;
		val = 0;
	}
	
	PulseWidth = val;

#if 0
	if( sNowPoly == 1 ){		// Low -> High
		CCP1CONbits.MODE = 4;		// Set FallingEdge
		HighWidth = val;
		sNowPoly = 0;
		oLED_MAIN_HIGH = 1;
	}else{						// High -> Low
		CCP1CONbits.MODE = 5;		// Set RisingEdge
		LowWidth = val;
		sNowPoly = 1;
		oLED_MAIN_HIGH = 0;
	}
#endif
}
#endif

// Driver

stIrCommData sComm;
uint8_t IsIrReceived,IrPos=-1;
uint32_t IrTmp=0;
#define T_TIMEBASE	850				// T = 425us
void TaskIrReceive(void){
	if( RANGE(PulseWidth, T_TIMEBASE*24, T_TIMEBASE*2)){
			IrPos = 0;
			PulseWidth = 0;
	}else if( (IrPos != -1) && PulseWidth ){
		IrPos ++;
		if( RANGE( PulseWidth, T_TIMEBASE*4, T_TIMEBASE )){
			IrTmp = 0x80000000 | (IrTmp>>1);
		}else if( RANGE( PulseWidth, T_TIMEBASE*2, T_TIMEBASE )){
			IrTmp = IrTmp>>1;
		}else{
			IrPos = -1;
		}
		PulseWidth = 0;
		
		if( IrPos == 32 ){
			sComm.dat32 = IrTmp;
			IsIrReceived = 1;
			IrPos = -1;
		}
	}
	
#if 0
	if( HighWidth > (T_TIMEBASE/2) ){
		if( RANGE( LowWidth,T_TIMEBASE *16,(T_TIMEBASE *4)) ){
			if( RANGE(HighWidth,(T_TIMEBASE *8),(T_TIMEBASE *1)) ){		// Header?
				IrPos = 0;
			}
		}else if( RANGE(LowWidth, T_TIMEBASE, (T_TIMEBASE/2) )){
			if( IrPos != -1 ){
				if( HighWidth < (T_TIMEBASE*2) ){
					IrTmp = (IrTmp>>1);
				}else{
					IrTmp = 0x80000000 | (IrTmp>>1);
				}

				if( IrPos < 31 ) IrPos ++;
				else{
					sComm.dat32 = IrTmp;
					IsIrReceived = 1;
					IrPos = -1;
				}
			}
		}
		LowWidth = 0;
		HighWidth = 0;
	}
#endif
}

#define SW_TIM	100
void TaskInput( stInput *dat ){
	static uint8_t Tim[5];
	uint8_t *pTim = Tim;
	
	if( iSW_FANON ){
		if( *pTim <= SW_TIM) *pTim ++;
	}else *pTim = 0;
	dat->FanOn = (*pTim==SW_TIM) ?1 :0 ;

	pTim ++;
	if( iSW_FANMAIN ){
		if( *pTim <= SW_TIM) *pTim ++;
	}else *pTim = 0;
	dat->FanMove = (*pTim==SW_TIM) ?1 :0 ;
	
	pTim ++;
	if( iSW_FANLR ){
		if( *pTim <= SW_TIM) *pTim ++;
	}else *pTim = 0;
	dat->LROn = (*pTim==SW_TIM) ?1 :0 ;

	pTim ++;
	if( iSW_TIMER ){
		if( *pTim <= SW_TIM ) *pTim ++;
	}else *pTim = 0;
	dat->Timer = (*pTim==SW_TIM) ?1 :0 ;
	
	pTim ++;
	if( iSW_FANUD ){
		if( *pTim < SW_TIM ) *pTim ++;
		else dat->UDOn = 1;
	}else{
		if( *pTim!=0 ) *pTim --;
		else dat->UDOn = 0;
	}
}

// Timer1 1SecCounterInitial
uint16_t sOffCount;
uint8_t isOfftimerEnd;
#define TIMER1_1SEC		7750
#ifndef TDD
void InitTimeCount(void){
	T1CONbits.ON = false;
	
	T1CONbits.CKPS = 0b10;	// Prescale 1/4
	T1CONbits.RD16 = false;
	
	T1GCONbits.GE = false;	// GateOff
	
	T1CLKbits.CS = 0b0100;	// LFINTOSC(31kHz)
	
	TMR1 = 0;
}
#endif

uint8_t IsTime1Sec(void){
	uint16_t tmr = TMR1;
	
	if( tmr >= TIMER1_1SEC ){
		uint8_t h,l;
		tmr = tmr -TIMER1_1SEC;
		h = tmr>>8;	l = tmr;
		T1CONbits.ON = false;
		TMR1H = h;	TMR1L = l;
		T1CONbits.ON = true;
		return true;
	}else return false;
}

void StartTimeCount(void){
	T1CONbits.ON = true;
}

void DispOffTimerLevel( uint16_t offTimerValue ){
	uint8_t lv;		// 7200 = 2H
	if( offTimerValue == 0 ) lv = 0;
	else lv = (offTimerValue +(7200-1)) /7200 ;

	oLED_OFFTIMER2 = (lv==1) ?1 :0 ;
	oLED_OFFTIMER4 = (lv==2) ?1 :0 ;
	oLED_OFFTIMER6 = (lv==3) ?1 :0 ;
	oLED_OFFTIMER8 = (lv==4) ?1 :0 ;
}

void DispFanStatus( stCommData *fanValue ){
	oLED_MAIN_HIGH	= (fanValue->FanLevel==1) ?1 :0;
	oLED_MAIN_MID	= (fanValue->FanLevel==2) ?1 :0;
	oLED_MAIN_LOW	= (fanValue->FanLevel==3) ?1 :0;
	oLED_LR			= fanValue->LRFanOn;
	oLED_UD			= fanValue->UDFanOn;	
}

uint8_t euartBuf[16];
uint8_t wp,rp;
void TaskEuart(void){
	if( PIR3bits.RCIF || TX1STAbits.TRMT ){
		if( wp != rp ){
			TX1REG = euartBuf[rp];
			rp = (rp+1) &0x0F;
		}
		PIR3bits.RCIF = false;	
	}
}


// App

#define IRDATA_TIMEOUT	(86*3)			// 1Packet : 85.5ms
void ReadIrData( uint8_t *rev ){
	static uint16_t Timeout;
	if( IsIrReceived ){
		IsIrReceived = 0;
		if( (sComm.dat32 != 0) && ((sComm.dat8[0]+sComm.dat8[1]) == 0xFF) && ((sComm.dat8[2]+sComm.dat8[3] == 0xFF)) ){
			Timeout = IRDATA_TIMEOUT;
			*rev = sComm.dat8[2];
			sComm.dat32 = 0;
		}
	}else{
		if( Timeout ) Timeout --;
		else *rev = 0;
	}
}

void TaskOfftimer(void){
	if( IsTime1Sec() && sOffCount ){
		sOffCount --;
		isOfftimerEnd = (sOffCount) ?0 :1 ;
	}
}

uint8_t IsOffTime(void){
	if( isOfftimerEnd ){
		isOfftimerEnd = 0;
		return 1;
	}else return 0;
}

// Code
// 0 : None
// 1 : MainFanOn
// 2 : MainFanMove
// 3 : Timer
// 4 : LeftRightFan
// 5 : UpDownFan

void ContFanValue( uint8_t code, stInput *in, stCommData *out ){
	static uint8_t exCode = 0;
	uint8_t plsCode;
	
	if( exCode != code ){
		exCode = code;
		plsCode = code;
	}else plsCode = 0;
	
	if( in->FanOn || (plsCode==1) ){
		if( out->FanLevel != 0 ) out->FanLevel = 0;
		else out->FanLevel = 1;
	}

	if( (in->FanMove || (plsCode==2)) && out->FanLevel ){
		if( out->FanLevel < 3 ) out->FanLevel ++;
		else out->FanLevel = 1;
	}

	if( in->LROn || (plsCode==4) ) out->LRFanOn ^= 1;

	out->UDFanOn = ( in->UDOn || (exCode==5) ) ?1 :0 ;
	
	if( in->Timer || (plsCode==3) ){
		if( sOffCount > (3600*6) )sOffCount = 0;
		sOffCount = sOffCount -(sOffCount%7200) +7200;
	}else if( IsOffTime() ){
		out->FanLevel = 0;
	}
	
	if( !out->FanLevel ){
		out->LRFanOn = 0;
		out->UDFanOn = 0;
	}
}

void SendEuart( uint8_t dat){
	euartBuf[wp] = dat;
	wp = (wp+1) &0x0F;
	euartBuf[wp] = ~dat;
	wp = (wp+1) &0x0F;
}

#ifndef TDD
void main(void){
	static uint8_t Dly;
	static uint8_t IrCode;
	
    SYSTEM_Initialize();
	TMR0_SetInterruptHandler(isrTimer0);
	CCP5_SetCallBack(isrCcp1);
	TMR3_StartTimer();
//	InitTimeCount();
	
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

	
//	StartTimeCount();
    while (1){
		TaskEuart();
		TaskIrReceive();
		
		if( gInterval ){
			gInterval --;
						
//			TaskOfftimer();
			TaskInput( &gIn );
			
			ReadIrData( &IrCode );
			ContFanValue( IrCode, &gIn, &gComm );
			
			DispOffTimerLevel( sOffCount );
			DispFanStatus( &gComm );
			
			if( Dly < 100 ) Dly++;
			else{
				uint8_t *p = (uint8_t *)&gComm;
				gComm.Check1 = 2;
				Dly = 0;
				SendEuart('S');
				SendEuart(*p);
			}
		}
	}
}
#endif
