#include "rack.hpp"

#define EXERCISE_1

using namespace rack;

template <typename T>
struct SVF {
	T hp, bp, lp, phi, gamma;
	T fc, damp;

public:
	SVF(T fc, T damp) {
		setCoeffs(fc, damp);
		reset();
	}

	void setCoeffs(T fc, T damp) {
#ifdef EXERCISE_1
		if (this->fc != fc || this->damp != damp) {
#endif
			this->fc = fc;
			this->damp = damp;

			phi = clamp( 2.0*sin(M_PI * fc * APP->engine->getSampleTime()),
					0.f, 1.f);

			gamma = clamp(2.0 * damp, 0.f, 1.f);

#ifdef EXERCISE_1
		}
#endif
	}

	void reset() {
		hp = bp = lp = 0.0;
	}

	void process(T xn, T* hpf, T* bpf, T* lpf) {
		bp = *bpf = phi*hp + bp;
		lp = *lpf = phi*bp + lp;
		hp = *hpf = xn - lp - gamma*bp;
	}
};
