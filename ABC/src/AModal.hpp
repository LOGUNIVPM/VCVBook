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

#include "ABC.hpp"
#include "dsp/digital.hpp"
#include "SVF.hpp"

#define MAX_OSC 64
#define DAMP_SLOPE_MAX 0.01
#define SCOPE_BUFFERSIZE 512
#define MASS_BOX_W (15*6)
#define JSON_XCOORD_KEY "hitPoint"
#define JSON_NOSC_KEY "nActiveOsc"


struct AModal : Module {
	enum ParamIds {
		PARAM_F0,		// fundamental frequency
		PARAM_DAMP,		// overall damping
		PARAM_INHARM,	// inharmonicity
		PARAM_DAMPSLOPE,// damping slope in frequency
		PARAM_MOD_CV,	// modulation input amount
		NUM_PARAMS,
	};

	enum InputIds {
		MAIN_IN,		// main input (excitation)
		MOD1_IN,		// modulation input for AM
		VOCT_IN,
		NUM_INPUTS,
	};

	enum OutputIds {
		MAIN_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	SVF<float> * osc[MAX_OSC];
	float out;
	float f0, inhrm, damp, dsl;
	float nActiveOsc;

	AModal() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_F0, 1.f, 1.8f, 1.f);
		configParam(PARAM_DAMP, 0.0000001f, 0.1f, 0.01f);
		configParam(PARAM_INHARM, 0.5f, 2.f, 1.f);
		configParam(PARAM_DAMPSLOPE, -DAMP_SLOPE_MAX, DAMP_SLOPE_MAX, 0.f);
		configParam(PARAM_MOD_CV, 0.f, 1.f, 0.f);
		out = 0.0;
		f0 = 100;
		inhrm = 0.0;
		damp = 0.5;
		dsl = 0.0;
		for (int i = 0; i < MAX_OSC; i++)
			osc[i] = new SVF<float>(100*i, 0.1);
		nActiveOsc = 16;
	}

	void process(const ProcessArgs &args) override;

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;


};



struct AModalGUI : AModal {

	float hitVelocity, hitPoint = 0.f;
	float audioBuffer[SCOPE_BUFFERSIZE];
	unsigned int idx = 0; // buffer index
	bool hitPointChanged = false;

	AModalGUI() {
		hitVelocity = 0.f;
	}

	void process(const ProcessArgs &args) override;

	void impact(float v, float x) {
		hitVelocity = v;
		hitPoint = ((x / MASS_BOX_W) - 0.5) * DAMP_SLOPE_MAX;
	}

	json_t *dataToJson() override;
	void dataFromJson(json_t *rootJ) override;

};


struct HammDisplay : Widget {
	AModalGUI *module = NULL;
	float massX;
	float massY = 0;
	float massV = 0;
	const float massA = 2.f;
	float impactY = 0;
	const float thresV = 1;
	const float massR = 0.8;
	const float massRadius = 5;

	HammDisplay(float hitPoint = MASS_BOX_W/2.f) {
		massX = hitPoint;
	}

	void onButton(const event::Button &e) override {
		e.stopPropagating();
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			moveMass(e.pos.x, e.pos.y);
			e.consume(NULL);
		}
	}

	void moveMass(float x, float y) {
		massX = x;
		massY = y;
	}

	void draw(const DrawArgs &args) override {

		//background (this will be rendered in the module browser)
		nvgFillColor(args.vg, nvgRGB(20, 30, 33));
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFill(args.vg);

		if (module == NULL) return;
		// if we are in the module browser we will not cross this point

		if (module->hitPointChanged) {
			massX = (module->hitPoint/DAMP_SLOPE_MAX + 0.5f)*MASS_BOX_W;
			module->hitPointChanged = false;
		}

		// FALL MODEL
		if (massY <= impactY) {
			// free mass
			massV += massA;
			massY += massV;
		} else {
			// impact
			module->impact(massV, massX); // transmit velocity
			massV = -massR * massV; // massRatio 0.8: (m1-m2)*v1/(m1+m2) perche v2 = -v1, m2 è trascurabile risp a m2
			if (fabs(massV) < thresV)
				massV = 0.f;
			else
				massY = impactY;

		}

		// Draw waveform
		nvgStrokeColor(args.vg, nvgRGBA(0xe1, 0x02, 0x78, 0xc0));
		float * buf = module->audioBuffer;
		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15*2)));
		nvgBeginPath(args.vg);
		unsigned int idx = module->idx;
		for (int i = 0; i < SCOPE_BUFFERSIZE; i++) {
			float x, y;
			x = (float)i / float(SCOPE_BUFFERSIZE-1);
			y = buf[idx++];
			if (idx > SCOPE_BUFFERSIZE) idx = 0;
			Vec p;
			p.x = b.pos.x + b.size.x * x;
			p.y = impactY + y * 10.f;
			if (i == 0)
				nvgMoveTo(args.vg, p.x, p.y);
			else
				nvgLineTo(args.vg, p.x, p.y);
		}
		nvgLineCap(args.vg, NVG_ROUND);
		nvgMiterLimit(args.vg, 2.0);
		nvgStrokeWidth(args.vg, 1.5);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgStroke(args.vg);

		// Draw Mass
		NVGcolor massColor = nvgRGB(25, 150, 252);
		nvgFillColor(args.vg, massColor);
		nvgStrokeColor(args.vg, massColor);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, massX, massY, massRadius);
		nvgFill(args.vg);
		nvgStroke(args.vg);
	}
};

/* Context Menu Item for changing the number of oscillators */
struct nActiveOscMenuItem : MenuItem {
	AModal *module;
	unsigned int nOsc;
	void onAction(const event::Action &e) override{
		module->nActiveOsc = nOsc;
	}

};

