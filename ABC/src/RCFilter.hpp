#include "rack.hpp"

template <typename T>
struct RCFilter {
	T yn, yn1, a;

	RCFilter(T aCoeff) {
		this->a = aCoeff;
		reset();
	}

	RCFilter() {
		this->a = 0.9f;
		reset();
	}

	void setTau(T tau) {
		this->a = tau / (tau + APP->engine->getSampleTime());
	}

	void setCutoff(T fc) {
		this->a = 1 - fc / APP->engine->getSampleRate();

	}


	void reset(T rstval = 0.0) {
		yn = yn1 = rstval;
	}

	T process(T xn) {
		yn = a * yn1 + (1-a) * xn;
		yn1 = yn;
		return yn;
	}
};

