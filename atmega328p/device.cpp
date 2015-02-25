#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include <Arduino.h>
#include <stdarg.h>

#include "device.h"

void Devices::begin() {
	// "...[it] is therefore required to turn off the watchdog 
	// early during program startup..." (from avr/wdt.h)
	wdt_disable();

	// turn off ADC and analog comparator
	ADCSRA &= ~bit(ADEN);
	ACSR |= bit(ACD);
	power_adc_disable();	// FIXME: power_all_disable()?

	for (int i = 2; i <= A5; i++) {
		pinMode(i, INPUT);
		digitalWrite(i, LOW);
	}

	for (int i = 0; i < _n; i++)
		_devices[i]->enable(_devices[i]->begin());

	sei();
}

unsigned Device::sleepmode() {
	return SLEEP_MODE_PWR_DOWN;
}

// required because there's no defined ordering of modes...
static unsigned update_mode(unsigned m, unsigned mode) {
	switch (mode) {
	case SLEEP_MODE_IDLE:
		return mode;

	case SLEEP_MODE_ADC:
		if (m != SLEEP_MODE_IDLE)
			return mode;
		break;
	case SLEEP_MODE_PWR_SAVE:
		if (m != SLEEP_MODE_IDLE && m != SLEEP_MODE_ADC)
			return mode;
		break;
	case SLEEP_MODE_EXT_STANDBY:
		if (m == SLEEP_MODE_PWR_DOWN || m == SLEEP_MODE_STANDBY)
			return mode;
		break;
	case SLEEP_MODE_STANDBY:
		if (m == SLEEP_MODE_PWR_DOWN)
			return mode;
		break;
	case SLEEP_MODE_PWR_DOWN:
		break;
	}
	return m;
}

int Devices::select() {
again:
	// so we don't miss an interrupt while checking...
	cli();
	unsigned mode = SLEEP_MODE_PWR_DOWN;
	for (int i = 0; i < _n; i++) {
		Device *d = _devices[i];
		if (d->is_ready()) {
			sei();
			return d->id();
		}
		if (d->is_enabled())
			mode = update_mode(mode, d->sleepmode());
	}

	set_sleep_mode(mode);
	sleep_enable();

	// arduino 1.5.8 finally updated the avr toolchain
	sleep_bod_disable();
	sei();
	sleep_cpu();
	sleep_disable();

	goto again;
}
