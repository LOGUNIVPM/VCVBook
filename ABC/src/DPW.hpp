/*--------------------------- ABC ---------------------------------*
 *
 * Author: Leonardo Gabrielli <l.gabrielli@univpm.it>
 * License: GPLv3
 *
 * For a detailed guide of the code and functions see the book:
 * "Developing Virtual Synthesizers with VCV Rack" by L.Gabrielli
 *
 * Copyright 2020, Leonardo Gabrielli
 *
 *-----------------------------------------------------------------*/

#include "rack.hpp"

using namespace rack;


inline int factorial(int n) {
	if (n > 1) return n * factorial(n-1);
	else return 1;
}

typedef enum {
	DPW_1 = 1,
	DPW_2 = 2,
	DPW_3 = 3,
	DPW_4 = 4,
	MAX_ORDER = DPW_4,
} DPWORDER;

typedef enum {
	TYPE_SAW,
	TYPE_SQU,
	TYPE_TRI,
} WAVETYPE;

template <typename T>
struct DPW {
	T pitch, phase;
	T gain = 1.0;
	unsigned int dpwOrder = 1;
	WAVETYPE waveType;
	T diffB[MAX_ORDER];
	unsigned int dbw = 0; // diffB write index
	int init;

	DPW() {
		waveType = TYPE_SAW;
		memset(diffB, 0, sizeof(T));
		paramsCompute();
		init = dpwOrder;
	}

	unsigned int onDPWOrderChange(unsigned int newdpw) {
		if (newdpw > MAX_ORDER)
			newdpw = MAX_ORDER;

		dpwOrder = newdpw;
		memset(diffB, 0, sizeof(diffB));
		paramsCompute();
		init = dpwOrder;
		return newdpw;
	}

	/**
	 * 	Differentiate ord-1 times
	 */
	T dpwDiff(int ord) {
		ord = clamp(ord, 0, MAX_ORDER);

		T tmpA[dpwOrder];
		memset(tmpA, 0, sizeof(tmpA));
		int dbr = (dbw - 1) % ord;

		for (int i = 0; i < ord; i++) {
			tmpA[i] = diffB[dbr--];
			if (dbr < 0) dbr = ord - 1;
		}

		while(ord) {
			for (int i = 0; i < ord-1; i++) {
				tmpA[i] = gain * ( tmpA[i] - tmpA[i+1] );
			}
			ord--;
		}
		return tmpA[0];
	}

	/**
	 * Compute the polynomial and call the diff
	 */
	T process() {

		// next step of the trivial waveform, advance phase
		T triv = trivialStep(phase);
		phase += pitch * APP->engine->getSampleTime();
		if (phase >= 1.0) phase -= 1.0;

		T sqr = triv * triv;
		T poly;

		switch (dpwOrder) {
		case DPW_1:
		default:
			return triv;
		case DPW_2:
			poly = sqr;
			break;
		case DPW_3:
			poly = sqr * triv - triv;
			return poly;
			break;
		case DPW_4:
			poly = sqr * sqr - 2.0 * sqr;
			break;
		}

		diffB[dbw++] = poly;
		if (dbw >= dpwOrder) dbw = 0;
		if (init) {
			init--;
			return poly;
		}
		return dpwDiff(dpwOrder);
	}

	/*
	 * Generate the trivial waveform
	 */
	T trivialStep(int type) {
		switch(type) {
		case TYPE_SAW:
			return 2 * phase - 1;
		default:
			return 0; // implementing other trivial waveforms is left as an exercise
		}
	}

	/*
	 * Diff gain compute
	 */
	void paramsCompute() {

		if (dpwOrder > 1)
			gain = std::pow(1.f / factorial(dpwOrder) * std::pow(M_PI / (2.f*sin(M_PI*pitch * APP->engine->getSampleTime())),
					dpwOrder-1.f), 1.0 / (dpwOrder-1.f));
		else
			gain=1.0;
	}

	void setPitch(T newPitch) {
		if (pitch != newPitch) {
			pitch = newPitch;
			paramsCompute();
		}
	}

};
