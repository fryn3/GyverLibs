/*************************************************************************************
* Developed for AlexGyver https://github.com/AlexGyver/  by Egor 'Nich1con' Zaharov  *
* Library for generating interrupts on hardware timers ATmega328p					 *
* Distributed under a free license indicating the source							 *
* Supported MCU's : ATmega328p, ATmega2560											 *
* v1.0 from 18.02.2020 (Release)				
* v1.1 - исправлена ошибка в расчёте периодов
*************************************************************************************/

/*
	Настройка и контроль прерываний по аппаратным таймерам:
		- Поддерживаются все три таймера на ATmega328 и шесть таймеров на ATmega2560;		
		- Настройка периода (мкс) и частоты (Гц) прерываний:
			- 8 бит таймеры: 31 Гц - 1 МГц (32 258 мкс - 1 мкс);
			- 16 бит таймеры: 0.11 Гц - 1 МГц (9 000 000 мкс - 1 мкс);
		- Автоматическая корректировка настройки периода от частоты тактирования (F_CPU);
		- Функция возвращает точный установившийся период/частоту для отладки (частота ограничена разрешением таймера);
		- Поддержка многоканального режима работы: один таймер вызывает 2 (ATmega328) или
		3 (ATmega2560, таймеры 1, 3, 4, 5) прерывания с настраиваемым сдвигом по фазе 0-360 градусов;
		- Настраиваемое действие аппаратного вывода таймера по прерыванию: высокий сигнал, низкий сигнал, переключение. 
		Позволяет генерировать меандр (одно- и двухтактный);
		- Контроль работы таймера: старт/стоп/пауза/продолжить/инициализация;
*/

/*
----------------------------------- Arduino NANO (ATmega328) ----------------------------------------
Таймер	| Разрядность	| Частоты			| Периоды			| Выходы	| Пин Arduino	| Пин МК|
--------|---------------|-------------------|-------------------|-----------|---------------|-------|
Timer0	| 8 бит			| 31 Гц - 1 МГц		| 32 258 - 1 мкс	| CHANNEL_A	| D6			| PD6	|
		| 				| 					| 					| CHANNEL_B	| D5			| PD5	|
--------|---------------|-------------------|-------------------|-----------|---------------|-------|
Timer1	| 16 бит		| 0.11 Гц - 1 МГц	| 9 000 000 - 1 мкс	| CHANNEL_A	| D9			| PB1	|
		| 				| 					| 					| CHANNEL_B	| D10			| PB2	|
--------|---------------|-------------------|-------------------|-----------|---------------|-------|
Timer2	| 8 бит			| 31 Гц - 1 МГц		| 32 258 - 1 мкс	| CHANNEL_A	| D11			| PB3	|
		| 				| 					| 					| CHANNEL_B	| D3			| PD3	|
-----------------------------------------------------------------------------------------------------
						
--------------------------------- Arduino MEGA (ATmega2560) -----------------------------------------
Таймер	| Разрядность	| Частоты			| Периоды			| Выходы	| Пин Arduino	| Пин МК|
--------|---------------|-------------------|-------------------|-----------|---------------|-------|
Timer0	| 8 бит			| 31 Гц - 1 МГц		| 32 258 - 1 мкс	| CHANNEL_A	| 13			| PB7	|
		| 				| 					| 					| CHANNEL_B	| 4				| PG5	|
--------|---------------|-------------------|-------------------|-----------|---------------|-------|
Timer1	| 16 бит		| 0.11 Гц - 1 МГц	| 9 000 000 - 1 мкс	| CHANNEL_A	| 11			| PB5	|
		| 				| 					| 					| CHANNEL_B	| 12			| PB6	|
		| 				| 					| 					| CHANNEL_C	| 13			| PB7	|
--------|---------------|-------------------|-------------------|-----------|---------------|-------|
Timer2	| 8 бит			| 31 Гц - 1 МГц		| 32 258 - 1 мкс	| CHANNEL_A	| 10			| PB4	|
		| 				| 					| 					| CHANNEL_B	| 9				| PH6	|
--------|---------------|-------------------|-------------------|-----------|---------------|-------|
Timer3	| 16 бит		| 0.11 Гц - 1 МГц	| 9 000 000 - 1 мкс	| CHANNEL_A	| 5				| PE3	|
		| 				| 					| 					| CHANNEL_B	| 2				| PE4	|
		| 				| 					| 					| CHANNEL_C	| 3				| PE5	|
--------|---------------|-------------------|-------------------|-----------|---------------|-------|
Timer4	| 16 бит		| 0.11 Гц - 1 МГц	| 9 000 000 - 1 мкс	| CHANNEL_A	| 6				| PH3	|
		| 				| 					| 					| CHANNEL_B	| 7				| PH4	|
		| 				| 					| 					| CHANNEL_C	| 8				| PH5	|
--------|---------------|-------------------|-------------------|-----------|---------------|-------|
Timer5	| 16 бит		| 0.11 Гц - 1 МГц	| 9 000 000 - 1 мкс	| CHANNEL_A	| 46			| PL3	|
		| 				| 					| 					| CHANNEL_B	| 45			| PL4	|
		| 				| 					| 					| CHANNEL_C	| 44			| PL5	|
-----------------------------------------------------------------------------------------------------
*/

/*
	setPeriod(период) - установка периода в микросекундах и запуск таймера. Возвращает реальный период (точность ограничена разрешением таймера).
	setFrequency(частота) - установка частоты в Герцах и запуск таймера. Возвращает реальную частоту (точность ограничена разрешением таймера).
	setFrequencyFloat(частота float) - установка частоты в Герцах и запуск таймера, разрешены десятичные дроби. Возвращает реальную частоту (точность ограничена разрешением таймера).
	enableISR(источник, фаза) - включить прерывания, канал прерываний CHANNEL_A или CHANNEL_B (+ CHANNEL_C у Mega2560), сдвиг фазы 0-359 (без указания параметров будет включен канал А и сдвиг фазы 0).
	disableISR(источник) - выключить прерывания, канал CHANNEL_A или CHANNEL_B. Счёт таймера не останавливается (без указания параметров будет выключен канал А).
	pause() - приостановить счёт таймера, не сбрасывая счётчик
	resume() - продолжить счёт после паузы
	stop() - остановить счёт и сбросить счётчик
	restart() - перезапустить таймер (сбросить счётчик)
	setDefault() - установить параметры таймера по умолчанию ("Ардуино-умолчания")
	outputEnable(канал, режим) - канал: включить выход таймера CHANNEL_A или CHANNEL_B (+ CHANNEL_C у Mega2560). Режим: TOGGLE_PIN, CLEAR_PIN, SET_PIN (переключить/выключить/включить пин по прерыванию)
	outputDisable(канал) - отключить выход таймера CHANNEL_A или CHANNEL_B (+ CHANNEL_C у Mega2560, см. такблицу таймеров)
	outputState(канал, состояние) - сменить состояние канала: HIGH / LOW
*/

#pragma once
#include <Arduino.h>

/* ==========  Constants ========== */
#define CHANNEL_A 0x00
#define CHANNEL_B 0x01
#define CHANNEL_C 0x02

#define TOGGLE_PIN 0x01
#define CLEAR_PIN 0x02
#define SET_PIN 0x03

#define TIMER0_A  TIMER0_COMPA_vect
#define TIMER0_B  TIMER0_COMPB_vect
#define TIMER1_A  TIMER1_COMPA_vect
#define TIMER1_B  TIMER1_COMPB_vect
#define TIMER2_A  TIMER2_COMPA_vect
#define TIMER2_B  TIMER2_COMPB_vect

#if defined(__AVR_ATmega2560__)
#define TIMER1_C  TIMER1_COMPC_vect
#define TIMER3_A  TIMER3_COMPA_vect
#define TIMER3_B  TIMER3_COMPB_vect
#define TIMER3_C  TIMER3_COMPC_vect
#define TIMER4_A  TIMER4_COMPA_vect
#define TIMER4_B  TIMER4_COMPB_vect
#define TIMER4_C  TIMER4_COMPC_vect
#define TIMER5_A  TIMER5_COMPA_vect
#define TIMER5_B  TIMER5_COMPB_vect
#define TIMER5_C  TIMER5_COMPC_vect
#endif

/* ================ Сlasses of timers ================ */
class Timer_0 {                       					  // Timer 0
public:
	uint32_t setPeriod(uint32_t _timer0_period);          // Set timer period [us]

	inline __attribute__((always_inline))
	uint32_t setFrequency(uint32_t _timer0_frequency) {   // Set timer frequency [Hz]
		return 1000000UL / (Timer_0::setPeriod(1000000UL / _timer0_frequency)); // Convert frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	float setFrequencyFloat(float _timer0_frequency) {  	  // Set timer float frequency [Hz]
		return 1000000.0F / (Timer_0::setPeriod(1000000.0F / _timer0_frequency)); // Convert float frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	void enableISR(bool source = CHANNEL_A , uint16_t phase = 0x00) {          // Enable timer interrupt , channel A or B , set phase shift between interrupts
		if (!source) TIMSK0 |= (1 << OCIE0A);                   // Channel A , interrupt enable
		else {                                  // Channel B
			TIMSK0 |= (1 << OCIE0B);                        // Interrupt enable
			OCR0B = map(phase, 0, 360,  0, OCR0A);                  // Convert 0...360 degrees to 0...timer top (channel B only)
		}
	}

	inline __attribute__((always_inline))
	void disableISR(bool source = CHANNEL_A) {             			  // Disable timer interrupt , channel A or B
		TIMSK0 &= ~ (source ? (1 << OCIE0B) : (1 << OCIE0A));   // Disable timer interrupt , channel A or B
	}

	inline __attribute__((always_inline))
	void pause(void) {                   				  // Disable timer clock , not cleaning the counter
		_timer0_clock = (TCCR0B & 0x07);    // Save timer clock settings
		TCCR0B = (TCCR0B & 0xF8);       	// Clear timer clock bits
	}

	inline __attribute__((always_inline))
	void resume(void) {                 				      // Return clock timer settings
		TCCR0B = ((TCCR0B & 0xF8) |  _timer0_clock);  // Return clock timer settings
	}

	inline __attribute__((always_inline))
	void stop(void) {                   					  // Disable timer clock , and cleaning the counter
		Timer_0::pause();
		TCNT0 = 0x00;             // Clear timer counter
	}

	inline __attribute__((always_inline))
	void restart(void) {                  				  // Return clock timer settings & reset counter
		Timer_0::resume();
		TCNT0 = 0x00;
	}

	inline __attribute__((always_inline))
	void setDefault(void) {               			      // Set default timer settings
		TCCR0A = 0x03;  // Fast PWM , 8 bit
		TCCR0B = 0x03;  // Prescaler /64
		OCR0B = 0x00;   // Clear COMPA
		OCR0A = 0x00;   // Clear COMPB
		TCNT0 = 0x00;   // Clear counter
	}

	inline __attribute__((always_inline))
	void outputEnable(uint8_t channel, uint8_t mode) {	  // Enable and configurate timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR0A = (TCCR0A & 0x3F) | (mode << 6);  // set mode bits 
			return;
		case CHANNEL_B:
			TCCR0A = (TCCR0A & 0xCF) | (mode << 4);
			return;
		}
	}

	inline __attribute__((always_inline))
	void outputDisable(uint8_t channel) {				  // Disable timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR0A = (TCCR0A & 0x3F);	// disable output from timer
			return;
		case CHANNEL_B:
			TCCR0A = (TCCR0A & 0xCF); 
			return;
		}
	}

	inline __attribute__((always_inline))
	void outputState(uint8_t channel, bool state) {		  // Set High / Low on the timer output 
		switch (channel) {
		case CHANNEL_A:
			TCCR0B = (TCCR0B & 0x7F) | (state << FOC0A);
			return;
		case CHANNEL_B:
			TCCR0B = (TCCR0B & 0xBF) | (state << FOC0B);
			return;
		}
	}

private:
	uint8_t _timer0_clock = 0x00;           			  // Variable to store timer clock settings
};

class Timer_1 {                      					  // Timer 1
public:
	uint32_t setPeriod(uint32_t _timer1_period);      	  // Set timer period [Hz]

	inline __attribute__((always_inline))
	uint32_t setFrequency(uint32_t _timer1_frequency) {   // Set timer frequency [Hz]
		return 1000000UL / (Timer_1::setPeriod(1000000UL / _timer1_frequency)); // Convert frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	float setFrequencyFloat(float _timer1_frequency) {  	  // Set timer float frequency [Hz]
		return 1000000.0F / (Timer_1::setPeriod(1000000.0F / _timer1_frequency)); // Convert float frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	void enableISR(uint8_t source = CHANNEL_A , uint16_t phase = 0x00) {       // Enable timer interrupt , channel A or B , set phase shift between interrupts
	switch (source) {
		case CHANNEL_A:
			TIMSK1 |= (1 << OCIE1A);                        // Interrupt enable
			OCR1A = map(phase, 0, 360, 0, ICR1);                    // Convert 0...360 degrees to 0...timer top
			return;
		case CHANNEL_B:
			TIMSK1 |= (1 << OCIE1B);                        // Interrupt enable
			OCR1B = map(phase, 0, 360, 0, ICR1);                  // Convert 0...360 degrees to 0...timer top
			return;
#if defined(__AVR_ATmega2560__)
		case CHANNEL_C:
			TIMSK1 |= (1 << OCIE1C);                        // Interrupt enable
			OCR1C = map(phase, 0, 360, 0, ICR1);                  // Convert 0...360 degrees to 0...timer top
			return;
#endif
		}
	}

	inline __attribute__((always_inline))
	void disableISR(uint8_t source = CHANNEL_A) {           			  // Disable timer interrupt , channel A or B
		switch (source) {
		case CHANNEL_A:
			TIMSK1 &= ~ (1 << OCIE1A);
			return;
		case CHANNEL_B:
			TIMSK1 &= ~ (1 << OCIE1B);
			return;
#if defined(__AVR_ATmega2560__)
		case CHANNEL_C:
			TIMSK1 &= ~ (1 << OCIE1C);
			return;
#endif
		}
	}

	inline __attribute__((always_inline))
	void pause(void) {                   				  // Disable timer clock , not cleaning the counter
		_timer1_clock = (TCCR1B & 0x07);    // Save timer clock settings
		TCCR1B = (TCCR1B & 0xF8);       	// Clear timer clock bits
	}

	inline __attribute__((always_inline))
	void resume(void) {                    				  // Return clock timer settings
		TCCR1B = ((TCCR1B & 0xF8) |  _timer1_clock);  // Return clock timer settings
	}

	inline __attribute__((always_inline))
	void stop(void) {                    				  // Disable timer clock , and cleaning the counter	
		Timer_1::pause();
		TCNT1 = 0x00;             // Clear timer counter
	}

	inline __attribute__((always_inline))
	void restart(void) {                   				  // Return clock timer settings & reset counter
		Timer_1::resume();
		TCNT1 = 0x00;
	}

	inline __attribute__((always_inline))
	void setDefault(void) {                  			  // Set default timer settings
		TCCR1A = 0x01;  // Phasecorrect PWM , 8 bit
		TCCR1B = 0x0B;  // Prescaler /64
		OCR1B = 0x00;   // Clear COMPA
		OCR1A = 0x00;   // Clear COMPB
		TCNT1 = 0x00;   // Clear counter
	}

	inline __attribute__((always_inline))
	void outputEnable(uint8_t channel, uint8_t mode) {	  // Enable and configurate timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR1A = (TCCR1A & 0x3F) | (mode << 6);  // set mode bits 
			return;
		case CHANNEL_B:
			TCCR1A = (TCCR1A & 0xCF) | (mode << 4);
			return;
#if defined(__AVR_ATmega2560__)
		case CHANNEL_C:
			TCCR1A = (TCCR1A & 0xF3) | (mode << 2);
			return;
#endif
		}
	}

	inline __attribute__((always_inline))
	void outputDisable(uint8_t channel) {				  // Disable timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR1A = (TCCR1A & 0x3F);	// disable output from timer
			return;
		case CHANNEL_B:
			TCCR1A = (TCCR1A & 0xCF);
			return;
#if defined(__AVR_ATmega2560__)
		case CHANNEL_C:
			TCCR1A = (TCCR1A & 0xF3);
			return;
#endif
		}
	}

	inline __attribute__((always_inline))
	void outputState(uint8_t channel, bool state) {		  // Set High / Low on the timer output  
		switch (channel) {
		case CHANNEL_A:
			TCCR1C = (TCCR1C & 0x7F) | (state << FOC1A);
			return;
		case CHANNEL_B:
			TCCR1C = (TCCR1C & 0xBF) | (state << FOC1B);
			return;
#if defined(__AVR_ATmega2560__)
		case CHANNEL_C:
			TCCR1C = (TCCR1C & 0xDF) | (state << FOC1C);
			return;
#endif
		}
	}

private:
	uint8_t _timer1_clock = 0x00;             			  // Variable to store timer clock settings
};

class Timer_2 {                       					  // Timer 2
public:
	uint32_t setPeriod(uint32_t _timer2_period);      	  // Set timer period [Hz]
	
	inline __attribute__((always_inline))
	uint32_t setFrequency(uint32_t _timer2_frequency) {   // Set timer frequency [Hz]
		return 1000000UL / (Timer_2::setPeriod(1000000UL / _timer2_frequency)); // Convert frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	float setFrequencyFloat(float _timer2_frequency) {  	  // Set timer float frequency [Hz]
		return 1000000.0F / (Timer_2::setPeriod(1000000.0F / _timer2_frequency)); // Convert float frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	void enableISR(bool source = CHANNEL_A , uint16_t phase = 0x00) {      	  // Enable timer interrupt , channel A or B , set phase shift between interrupts
		if (!source) TIMSK2 |= (1 << OCIE2A);                   // Channel A , interrupt enable
		else {                                  // Channel B
			TIMSK2 |= (1 << OCIE2B);                        // Interrupt enable
			OCR2B = map(phase, 0, 360, 0, OCR2A);                 // Convert 0...360 degrees to 0...timer top (channel B only)
		}
	}

	inline __attribute__((always_inline))
	void disableISR(bool source = CHANNEL_A) {             			  // Disable timer interrupt , channel A or B
		TIMSK2 &= ~ (source ? (1 << OCIE2B) : (1 << OCIE2A));   // Disable timer interrupt , channel A or B
	}

	inline __attribute__((always_inline))
	void pause(void) {                   				  // Disable timer clock , not cleaning the counter
		_timer2_clock = (TCCR2B & 0x07);    // Save timer clock settings
		TCCR2B = (TCCR2B & 0xF8);       	// Clear timer clock bits
	}

	inline __attribute__((always_inline))
	void resume(void) {                 				      // Return clock timer settings
		TCCR2B = ((TCCR2B & 0xF8) |  _timer2_clock);  // Return clock timer settings
	}

	inline __attribute__((always_inline))
	void stop(void) {                     				  // Disable timer clock , and cleaning the counter
		this->pause();
		TCNT2 = 0x00;             // Clear timer counter
	}

	inline __attribute__((always_inline))
	void restart(void) {                  				  // Return clock timer settings & reset counter
		this->resume();
		TCNT2 = 0x00;
	}

	inline __attribute__((always_inline))
	void setDefault(void) {                  			  // Set default timer settings
		TCCR2A = 0x01;  // Phasecorrect PWM , 8 bit
		TCCR2B = 0x04;  // Prescaler /64
		OCR2B = 0x00;   // Clear COMPA
		OCR2A = 0x00;   // Clear COMPB
		TCNT2 = 0x00;   // Clear counter
	}

	inline __attribute__((always_inline))
	void outputEnable(uint8_t channel, uint8_t mode) {	  // Enable and configurate timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR2A = (TCCR2A & 0x3F) | (mode << 6);  // set mode bits 
			return;
		case CHANNEL_B:
			TCCR2A = (TCCR2A & 0xCF) | (mode << 4);
			return;
		}
	}

	inline __attribute__((always_inline))
	void outputDisable(uint8_t channel) {				  // Disable timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR2A = (TCCR2A & 0x3F);	// disable output from timer
			return;
		case CHANNEL_B:
			TCCR2A = (TCCR2A & 0xCF);
			return;
		}
	}

	inline __attribute__((always_inline))
	void outputState(uint8_t channel, bool state) {		  // Set High / Low on the timer output  
		switch (channel) {
		case CHANNEL_A:
			TCCR2B = (TCCR2B & 0x7F) | (state << FOC2A);
			return;
		case CHANNEL_B:
			TCCR2B = (TCCR2B & 0xBF) | (state << FOC2B);
			return;
		}
	}
private:
	uint8_t _timer2_clock = 0x00;             			  // Variable to store timer clock settings	
};

#if defined(__AVR_ATmega2560__)
class Timer_3 {                       					  // Timer 3
public:
	uint32_t setPeriod(uint32_t _timer3_period);     	  // Set timer period [Hz]
	
	inline __attribute__((always_inline))
	uint32_t setFrequency(uint32_t _timer3_frequency) {   // Set timer frequency [Hz]
		return 1000000UL / (Timer_3::setPeriod(1000000UL / _timer3_frequency)); // Convert frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	float setFrequencyFloat(float _timer3_frequency) {  	  // Set timer float frequency [Hz]
		return 1000000.0F / (Timer_3::setPeriod(1000000.0F / _timer3_frequency)); // Convert float frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	void enableISR(uint8_t source = CHANNEL_A , uint16_t phase = 0x00) {       // Enable timer interrupt , channel A or B , set phase shift between interrupts
		switch (source) {
		case CHANNEL_A:
			TIMSK3 |= (1 << OCIE3A);                        // Interrupt enable
			OCR3A = map(phase, 0, 360, 0, ICR3);                    // Convert 0...360 degrees to 0...timer top
			return;
		case CHANNEL_B:
			TIMSK3 |= (1 << OCIE3B);                        // Interrupt enable
			OCR3B = map(phase, 0, 360, 0, ICR3);                  // Convert 0...360 degrees to 0...timer top
			return;
		case CHANNEL_C:
			TIMSK3 |= (1 << OCIE3C);                        // Interrupt enable
			OCR3C = map(phase, 0, 360, 0, ICR3);                  // Convert 0...360 degrees to 0...timer top
			return;
		}
	}

	inline __attribute__((always_inline))
	void disableISR(uint8_t source = CHANNEL_A) {          			  // Disable timer interrupt , channel A or B
		switch (source) {
		case CHANNEL_A:
			TIMSK3 &= ~ (1 << OCIE3A);
			return;
		case CHANNEL_B:
			TIMSK3 &= ~ (1 << OCIE3B);
			return;
		case CHANNEL_C:
			TIMSK3 &= ~ (1 << OCIE3C);
			return;
		}
	}

	inline __attribute__((always_inline))
	void pause(void) {                  					  // Disable timer clock , not cleaning the counter
		_timer3_clock = (TCCR3B & 0x07);    // Save timer clock settings
		TCCR3B = (TCCR3B & 0xF8);       	// Clear timer clock bits
	}

	inline __attribute__((always_inline))
	void resume(void) {                   				  // Return clock timer settings
		TCCR3B = ((TCCR3B & 0xF8) |  _timer3_clock);  // Return clock timer settings
	}

	inline __attribute__((always_inline))
	void stop(void) {                    				  // Disable timer clock , and cleaning the counter
		Timer_3::pause();
		TCNT3 = 0x00;             // Clear timer counter
	}

	inline __attribute__((always_inline))
	void restart(void) {                   				  // Return clock timer settings & reset counter
		Timer_3::resume();
		TCNT3 = 0x00;
	}

	inline __attribute__((always_inline))
	void setDefault(void) {                				  // Set default timer settings
		TCCR3A = 0x01;  // Phasecorrect PWM , 8 bit
		TCCR3B = 0x0B;  // Prescaler /64
		OCR3B = 0x00;   // Clear COMPA
		OCR3A = 0x00;   // Clear COMPB
		TCNT3 = 0x00;   // Clear counter
	}

	inline __attribute__((always_inline))
	void outputEnable(uint8_t channel, uint8_t mode) {	  // Enable and configurate timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR3A = (TCCR3A & 0x3F) | (mode << 6);  // set mode bits 
			return;
		case CHANNEL_B:
			TCCR3A = (TCCR3A & 0xCF) | (mode << 4);
			return;
		case CHANNEL_C:
			TCCR3A = (TCCR3A & 0xF3) | (mode << 2);
			return;
		}
	}

	inline __attribute__((always_inline))
	void outputDisable(uint8_t channel) {				  // Disable timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR3A = (TCCR3A & 0x3F);	// disable output from timer
			return;
		case CHANNEL_B:
			TCCR3A = (TCCR3A & 0xCF);
			return;
		case CHANNEL_C:
			TCCR3A = (TCCR3A & 0xF3);
			return;
		}
	}

	inline __attribute__((always_inline))
	void outputState(uint8_t channel, bool state) {		  // Set High / Low on the timer output 
		switch (channel) {
		case CHANNEL_A:
			TCCR3C = (TCCR3C & 0x7F) | (state << FOC3A);
			return;
		case CHANNEL_B:
			TCCR3C = (TCCR3C & 0xBF) | (state << FOC3B);
			return;
		case CHANNEL_C:
			TCCR3C = (TCCR3C & 0xDF) | (state << FOC3C);
			return;
		}
	}

private:
	uint8_t _timer3_clock = 0x00;             			  // Variable to store timer clock settings
};

class Timer_4 {                      					  // Timer 4
public:
	uint32_t setPeriod(uint32_t _timer4_period);      	  // Set timer period [Hz]
	
	inline __attribute__((always_inline))
	uint32_t setFrequency(uint32_t _timer4_frequency) {   // Set timer frequency [Hz]
		return 1000000UL / (Timer_4::setPeriod(1000000UL / _timer4_frequency)); // Convert frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	float setFrequencyFloat(float _timer4_frequency) {  	  // Set timer float frequency [Hz]
		return 1000000.0F / (Timer_4::setPeriod(1000000.0F / _timer4_frequency)); // Convert float frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	void enableISR(uint8_t source = CHANNEL_A , uint16_t phase = 0x00) {       // Enable timer interrupt , channel A or B , set phase shift between interrupts
		switch (source) {
		case CHANNEL_A:
			TIMSK4 |= (1 << OCIE4A);                        // Interrupt enable
			OCR4A = map(phase, 0, 360, 0, ICR4);                    // Convert 0...360 degrees to 0...timer top
			return;
		case CHANNEL_B:
			TIMSK4 |= (1 << OCIE4B);                        // Interrupt enable
			OCR4B = map(phase, 0, 360, 0, ICR4);                  // Convert 0...360 degrees to 0...timer top
			return;
		case CHANNEL_C:
			TIMSK4 |= (1 << OCIE4C);                        // Interrupt enable
			OCR4C = map(phase, 0, 360, 0, ICR4);                  // Convert 0...360 degrees to 0...timer top
			return;
		}
	}

	inline __attribute__((always_inline))
	void disableISR(uint8_t source = CHANNEL_A) {           			  // Disable timer interrupt , channel A or B
		switch (source) {
		case CHANNEL_A:
			TIMSK4 &= ~ (1 << OCIE4A);
			return;
		case CHANNEL_B:
			TIMSK4 &= ~ (1 << OCIE4B);
			return;
		case CHANNEL_C:
			TIMSK4 &= ~ (1 << OCIE4C);
			return;
		}
	}

	inline __attribute__((always_inline))
	void pause(void) {                  					  // Disable timer clock , not cleaning the counter
		_timer4_clock = (TCCR4B & 0x07);    // Save timer clock settings
		TCCR4B = (TCCR4B & 0xF8);       	// Clear timer clock bits
	}

	inline __attribute__((always_inline))
	void resume(void) {                    				  // Return clock timer settings
		TCCR4B = ((TCCR4B & 0xF8) |  _timer4_clock);  // Return clock timer settings
	}

	inline __attribute__((always_inline))
	void stop(void) {                   				      // Disable timer clock , and cleaning the counter
		Timer_4::pause();
		TCNT4 = 0x00;             // Clear timer counter
	}

	inline __attribute__((always_inline))
	void restart(void) {                        			  // Return clock timer settings & reset counter
		Timer_4::resume();
		TCNT4 = 0x00;
	}

	inline __attribute__((always_inline))
	void setDefault(void) {                  			  // Set default timer settings
		TCCR4A = 0x01;  // Phasecorrect PWM , 8 bit
		TCCR4B = 0x0B;  // Prescaler /64
		OCR4B = 0x00;   // Clear COMPA
		OCR4A = 0x00;   // Clear COMPB
		TCNT4 = 0x00;   // Clear counter
	}

	inline __attribute__((always_inline))
	void outputEnable(uint8_t channel, uint8_t mode) {	  // Enable and configurate timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR4A = (TCCR4A & 0x3F) | (mode << 6);  // set mode bits 
			return;
		case CHANNEL_B:
			TCCR4A = (TCCR4A & 0xCF) | (mode << 4);
			return;
		case CHANNEL_C:
			TCCR4A = (TCCR4A & 0xF3) | (mode << 2);
			return;
		}
	}

	inline __attribute__((always_inline))
	void outputDisable(uint8_t channel) {				  // Disable timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR4A = (TCCR4A & 0x3F);	// disable output from timer
			return;
		case CHANNEL_B:
			TCCR4A = (TCCR4A & 0xCF);
			return;
		case CHANNEL_C:
			TCCR4A = (TCCR4A & 0xF3);
			return;
		}
	}

	inline __attribute__((always_inline))
	void outputState(uint8_t channel, bool state) {		  // Set High / Low on the timer output 
		switch (channel) {
		case CHANNEL_A:
			TCCR4C = (TCCR4C & 0x7F) | (state << FOC4A);
			return;
		case CHANNEL_B:
			TCCR4C = (TCCR4C & 0xBF) | (state << FOC4B);
			return;
		case CHANNEL_C:
			TCCR4C = (TCCR4C & 0xDF) | (state << FOC4C);
			return;
		}	
	}

private:
	uint8_t _timer4_clock = 0x00;            			  // Variable to store timer clock settings
};

class Timer_5 {                     					  // Timer 5
public:
	uint32_t setPeriod(uint32_t _timer5_period);          // Set timer period [Hz]
	
	inline __attribute__((always_inline))
	uint32_t setFrequency(uint32_t _timer5_frequency) {   // Set timer frequency [Hz]
		return 1000000UL / (Timer_5::setPeriod(1000000UL / _timer5_frequency)); // Convert frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	float setFrequencyFloat(float _timer5_frequency) {  	  // Set timer float frequency [Hz]
		return 1000000.0F / (Timer_5::setPeriod(1000000.0F / _timer5_frequency)); // Convert float frequency to period [Hz -> us]
	}
	
	inline __attribute__((always_inline))
	void enableISR(uint8_t source = CHANNEL_A , uint16_t phase = 0x00) {       // Enable timer interrupt , channel A or B , set phase shift between interrupts
		switch (source) {
		case CHANNEL_A:
			TIMSK5 |= (1 << OCIE5A);                        // Interrupt enable
			OCR5A = map(phase, 0, 360, 0, ICR5);                    // Convert 0...360 degrees to 0...timer top
			return;
		case CHANNEL_B:
			TIMSK5 |= (1 << OCIE5B);                        // Interrupt enable
			OCR5B = map(phase, 0, 360, 0, ICR5);                  // Convert 0...360 degrees to 0...timer top
			return;
		case CHANNEL_C:
			TIMSK5 |= (1 << OCIE5C);                        // Interrupt enable
			OCR5C = map(phase, 0, 360, 0, ICR5);                  // Convert 0...360 degrees to 0...timer top
			return;
		}
	}

	inline __attribute__((always_inline))
	void disableISR(uint8_t source = CHANNEL_A) {           			  // Disable timer interrupt , channel A or B
		switch (source) {
		case CHANNEL_A:
			TIMSK5 &= ~ (1 << OCIE5A);
			return;
		case CHANNEL_B:
			TIMSK5 &= ~ (1 << OCIE5B);
			return;
		case CHANNEL_C:
			TIMSK5 &= ~ (1 << OCIE5C);
			return;
		}
	}

	inline __attribute__((always_inline))
	void pause(void) {                  					  // Disable timer clock , not cleaning the counter
		_timer5_clock = (TCCR5B & 0x07);    // Save timer clock settings
		TCCR5B = (TCCR5B & 0xF8);       	// Clear timer clock bits
	}

	inline __attribute__((always_inline))
	void resume(void) {                    				  // Return clock timer settings
		TCCR5B = ((TCCR5B & 0xF8) |  _timer5_clock);  // Return clock timer settings
	}

	inline __attribute__((always_inline))
	void stop(void) {                    				  // Disable timer clock , and cleaning the counter
		Timer_5::pause();
		TCNT5 = 0x00;             // Clear timer counter
	}

	inline __attribute__((always_inline))
	void restart(void) {                  				  // Return clock timer settings & reset counter
		Timer_5::resume();
		TCNT5 = 0x00;
	}

	inline __attribute__((always_inline))
	void setDefault(void) {                 				  // Set default timer settings
		TCCR5A = 0x01;  // Phasecorrect PWM , 8 bit
		TCCR5B = 0x0B;  // Prescaler /64
		OCR5B = 0x00;   // Clear COMPA
		OCR5A = 0x00;   // Clear COMPB
		TCNT5 = 0x00;   // Clear counter
	}

	inline __attribute__((always_inline))
	void outputEnable(uint8_t channel, uint8_t mode) {	  // Enable and configurate timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR5A = (TCCR5A & 0x3F) | (mode << 6);  // set mode bits 
			return;
		case CHANNEL_B:
			TCCR5A = (TCCR5A & 0xCF) | (mode << 4);
			return;
		case CHANNEL_C:
			TCCR5A = (TCCR5A & 0xF3) | (mode << 2);
			return;
		}
	}

	inline __attribute__((always_inline))
	void outputDisable(uint8_t channel) {				  // Disable timer hardware output
		switch (channel) {
		case CHANNEL_A:
			TCCR5A = (TCCR5A & 0x3F);	// disable output from timer
			return;
		case CHANNEL_B:
			TCCR5A = (TCCR5A & 0xCF);
			return;
		case CHANNEL_C:
			TCCR5A = (TCCR5A & 0xF3);
			return;
		}
	}

	inline __attribute__((always_inline))
	void outputState(uint8_t channel, bool state) {		  // Set High / Low on the timer output 
		switch (channel) {
		case CHANNEL_A:
			TCCR5C = (TCCR5C & 0x7F) | (state << FOC5A);
			return;
		case CHANNEL_B:
			TCCR5C = (TCCR5C & 0xBF) | (state << FOC5B);
			return;
		case CHANNEL_C:
			TCCR5C = (TCCR5C & 0xDF) | (state << FOC5C);
			return;
		}
	}

private:
	uint8_t _timer5_clock = 0x00;                		  // Variable to store timer clock settings
};

#endif


////////////////////////////// objects //////////////////////////////
extern Timer_0 Timer0;
extern Timer_1 Timer1;
extern Timer_2 Timer2;

#if defined(__AVR_ATmega2560__)
extern Timer_3 Timer3;
extern Timer_4 Timer4;
extern Timer_5 Timer5;
#endif