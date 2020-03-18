#include "GyverTimers.h"

/* ================ Source code ================ */

////////////////////////////// setPeriod //////////////////////////////
uint32_t Timer_0::setPeriod(uint32_t _timer0_period)  {
	_timer0_period = constrain(_timer0_period, 1, 32258);

	uint32_t _timer0_cycles = F_CPU / 1000000 * _timer0_period;  // Calculation of the number of timer cycles per period
	uint8_t _timer0_prescaler = 0x00;
	uint16_t _timer0_divider = 0x00;

	if (_timer0_cycles < 256UL) {   // Сhoose optimal divider for the timer
		_timer0_prescaler = 0x01;
		_timer0_divider = 1UL;
	} else if (_timer0_cycles < 256UL * 8) {
		_timer0_prescaler = 0x02;
		_timer0_divider = 8UL;
	} else if (_timer0_cycles < 256UL * 64) {
		_timer0_prescaler = 0x03;
		_timer0_divider = 64UL;
	} else if (_timer0_cycles < 256UL * 256) {
		_timer0_prescaler = 0x04;
		_timer0_divider = 256UL;
	} else {
		_timer0_prescaler = 0x05;
		_timer0_divider = 1024UL;
	}

	uint8_t _timer0_top = (_timer0_cycles < 256UL * 1024 ? (_timer0_cycles / _timer0_divider) : 256UL) ;

	TCCR0A = (TCCR0A & 0xF0)|(1 << WGM21);      // CTC - mode
	TCCR0B = _timer0_prescaler;   // Set timer prescaler
	OCR0A = _timer0_top - 1;      // Set timer top

	return (2000000UL / ((F_CPU / _timer0_divider) / _timer0_top));   // Return real timer period
}

uint32_t Timer_1::setPeriod(uint32_t _timer1_period)  {
	_timer1_period = constrain(_timer1_period, 1, 9000000);
	
	uint32_t _timer1_cycles = F_CPU / 1000000 * _timer1_period;  // Calculation of the number of timer cycles per period
	uint8_t _timer1_prescaler = 0x00;
	uint16_t _timer1_divider = 0x00;

	if (_timer1_cycles < 65536UL) {   // Сhoose optimal divider for the timer
		_timer1_prescaler = 0x01;
		_timer1_divider = 1UL;
	} else if (_timer1_cycles < 65536UL * 8) {
		_timer1_prescaler = 0x02;
		_timer1_divider = 8UL;
	} else if (_timer1_cycles < 65536UL * 64) {
		_timer1_prescaler = 0x03;
		_timer1_divider = 64UL;
	} else if (_timer1_cycles < 65536UL * 256) {
		_timer1_prescaler = 0x04;
		_timer1_divider = 256UL;
	} else {
		_timer1_prescaler = 0x05;
		_timer1_divider = 1024UL;
	}

	uint16_t _timer1_top = (_timer1_cycles < 65536UL * 1024 ? (_timer1_cycles / _timer1_divider) : 65536UL) ;
#if defined(__AVR_ATmega2560__)
	TCCR1A = (TCCR1A & 0xFC);
#else
	TCCR1A = (TCCR1A & 0xF0);
#endif
	TCCR1B = ((1 << WGM13) | (1 << WGM12) | _timer1_prescaler);   // CTC mode + set prescaler
	ICR1 = _timer1_top - 1;                     // Set timer top

	return (2000000UL / ((F_CPU / _timer1_divider) / _timer1_top));   // Return real timer period
}

uint32_t Timer_2::setPeriod(uint32_t _timer2_period)  {
	_timer2_period = constrain(_timer2_period, 1, 32258);
	
	uint32_t _timer2_cycles = F_CPU / 1000000 * _timer2_period;  // Calculation of the number of timer cycles per period
	uint8_t _timer2_prescaler = 0x00;
	uint16_t _timer2_divider = 0x00;

	if (_timer2_cycles < 256UL) {   // Сhoose optimal divider for the timer
		_timer2_prescaler = 0x01;
		_timer2_divider = 1UL;
	} else if (_timer2_cycles < 256UL * 8) {
		_timer2_prescaler = 0x02;
		_timer2_divider = 8UL;
	} else if (_timer2_cycles < 256UL * 32) {
		_timer2_prescaler = 0x03;
		_timer2_divider = 32UL;
	} else if (_timer2_cycles < 256UL * 64) {
		_timer2_prescaler = 0x04;
		_timer2_divider = 64UL;
	} else if (_timer2_cycles < 256UL * 128) {
		_timer2_prescaler = 0x05;
		_timer2_divider = 128UL;
	} else if (_timer2_cycles < 256UL * 256) {
		_timer2_prescaler = 0x06;
		_timer2_divider = 256UL;
	} else {
		_timer2_prescaler = 0x07;
		_timer2_divider = 1024UL;
	}

	uint8_t _timer2_top = (_timer2_cycles < 256UL * 1024 ? (_timer2_cycles / _timer2_divider) : 256UL);

	TCCR2A = (TCCR2A & 0xF0)|(1 << WGM21);      // CTC - mode
	TCCR2B = _timer2_prescaler;   // Set timer prescaler
	OCR2A = _timer2_top - 1;      // Set timer top

	return (2000000UL / ((F_CPU / _timer2_divider) / _timer2_top));   // Return real timer period
}

#if defined(__AVR_ATmega2560__)

uint32_t Timer_3::setPeriod(uint32_t _timer3_period)  {
	_timer3_period = constrain(_timer3_period, 1, 9000000);
	
	uint32_t _timer3_cycles = F_CPU / 1000000 * _timer3_period;  // Calculation of the number of timer cycles per period
	uint8_t _timer3_prescaler = 0x00;
	uint16_t _timer3_divider = 0x00;

	if (_timer3_cycles < 65536UL) {   // Сhoose optimal divider for the timer
		_timer3_prescaler = 0x01;
		_timer3_divider = 1UL;
	} else if (_timer3_cycles < 65536UL * 8) {
		_timer3_prescaler = 0x02;
		_timer3_divider = 8UL;
	} else if (_timer3_cycles < 65536UL * 64) {
		_timer3_prescaler = 0x03;
		_timer3_divider = 64UL;
	} else if (_timer3_cycles < 65536UL * 256) {
		_timer3_prescaler = 0x04;
		_timer3_divider = 256UL;
	} else {
		_timer3_prescaler = 0x05;
		_timer3_divider = 1024UL;
	}

	uint16_t _timer3_top = (_timer3_cycles < 65536UL * 1024 ? (_timer3_cycles / _timer3_divider) : 65536UL) ;

	TCCR3A = (TCCR3A & 0xFC);
	TCCR3B = ((1 << WGM33) | (1 << WGM32) | _timer3_prescaler);   // CTC mode + set prescaler
	ICR3 = _timer3_top - 1;                     // Set timer top

	return (2000000UL / ((F_CPU / _timer3_divider) / _timer3_top));   // Return real timer period
}

uint32_t Timer_4::setPeriod(uint32_t _timer4_period)  {
	_timer4_period = constrain(_timer4_period, 1, 9000000);
	
	uint32_t _timer4_cycles = F_CPU / 1000000 * _timer4_period;  // Calculation of the number of timer cycles per period
	uint8_t _timer4_prescaler = 0x00;
	uint16_t _timer4_divider = 0x00;

	if (_timer4_cycles < 65536UL) {   // Сhoose optimal divider for the timer
		_timer4_prescaler = 0x01;
		_timer4_divider = 1UL;
	} else if (_timer4_cycles < 65536UL * 8) {
		_timer4_prescaler = 0x02;
		_timer4_divider = 8UL;
	} else if (_timer4_cycles < 65536UL * 64) {
		_timer4_prescaler = 0x03;
		_timer4_divider = 64UL;
	} else if (_timer4_cycles < 65536UL * 256) {
		_timer4_prescaler = 0x04;
		_timer4_divider = 256UL;
	} else {
		_timer4_prescaler = 0x05;
		_timer4_divider = 1024UL;
	}

	uint16_t _timer4_top = (_timer4_cycles < 65536UL * 1024 ? (_timer4_cycles / _timer4_divider) : 65536UL) ;

	TCCR4A = (TCCR4A & 0xFC);
	TCCR4B = ((1 << WGM43) | (1 << WGM42) | _timer4_prescaler);   // CTC mode + set prescaler
	ICR4 = _timer4_top - 1;                     // Set timer top

	return (2000000UL / ((F_CPU / _timer4_divider) / _timer4_top));   // Return real timer period
}

uint32_t Timer_5::setPeriod(uint32_t _timer5_period)  {
	_timer5_period = constrain(_timer5_period, 1, 9000000);
	
	uint32_t _timer5_cycles = F_CPU / 1000000 * _timer5_period;  // Calculation of the number of timer cycles per period
	uint8_t _timer5_prescaler = 0x00;
	uint16_t _timer5_divider = 0x00;

	if (_timer5_cycles < 65536UL) {   // Сhoose optimal divider for the timer
		_timer5_prescaler = 0x01;
		_timer5_divider = 1UL;
	} else if (_timer5_cycles < 65536UL * 8) {
		_timer5_prescaler = 0x02;
		_timer5_divider = 8UL;
	} else if (_timer5_cycles < 65536UL * 64) {
		_timer5_prescaler = 0x03;
		_timer5_divider = 64UL;
	} else if (_timer5_cycles < 65536UL * 256) {
		_timer5_prescaler = 0x04;
		_timer5_divider = 256UL;
	} else {
		_timer5_prescaler = 0x05;
		_timer5_divider = 1024UL;
	}

	uint16_t _timer5_top = (_timer5_cycles < 65536UL * 1024 ? (_timer5_cycles / _timer5_divider) : 65536UL) ;

	TCCR5A = (TCCR5A & 0xFC);
	TCCR5B = ((1 << WGM53) | (1 << WGM52) | _timer5_prescaler);   // CTC mode + set prescaler
	ICR5 = _timer5_top - 1;                     // Set timer top

	return (2000000UL / ((F_CPU / _timer5_divider) / _timer5_top));   // Return real timer period
}

#endif

Timer_0 Timer0 = Timer_0();
Timer_1 Timer1 = Timer_1();
Timer_2 Timer2 = Timer_2();

#if defined(__AVR_ATmega2560__)
Timer_3 Timer3 = Timer_3();
Timer_4 Timer4 = Timer_4();
Timer_5 Timer5 = Timer_5();
#endif