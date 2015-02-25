#ifndef __ADC_H__
#define __ADC_H__

typedef enum { vcc, internal, external } analog_ref_t;

/**
 * Analog-to-Digital conversion.
 * 
 * FIXME: analogReadResolution()
 */
class Analog: public Device {
public:
	// pin is the analog input, e.g., A0-A7
	Analog(int pin, analog_ref_t ref = vcc): 
		Device(pin), _pin(pin), _ref(ref) {}

	// not enabled by default
	bool begin();

	// changes the analog input
	void pin(int pin) { _pin = pin; _mux(); }

	// returns last converted value or 0xffff if not ready
	unsigned read();

	unsigned sleepmode();

protected:
	void _enable(bool enabled);

private:
	void _mux();

	int _pin;
	analog_ref_t _ref;
};

#endif