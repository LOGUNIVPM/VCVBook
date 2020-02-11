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
#include "RCFilter.hpp"

#define EPSILON 1e-9

struct AExpADSR : Module {
	enum ParamIds {
		PARAM_ATK,
		PARAM_DEC,
		PARAM_SUS,
		PARAM_REL,
		NUM_PARAMS,
	};

	enum InputIds {
		IN_GATE,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUT_ENVELOPE,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		LIGHT_GATE,
		NUM_LIGHTS,
	};

	AExpADSR() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_ATK, 0.0, 5.0, 0.5, "Attack Time", " s");
		configParam(PARAM_DEC, 0.0, 5.0, 0.5, "Decay Time", " s");
		configParam(PARAM_SUS, 0.0, 1.0, 0.5, "Sustain Time", " s");
		configParam(PARAM_REL, 0.0, 5.0, 0.5, "Release Time", " s");

		isAtk = false;
		isRunning = false;
		env = 0.0;
		Atau = Dtau = Rtau = 0.f;
	}

	dsp::SchmittTrigger gateDetect;
	RCFilter<float> * rcf = new RCFilter<float>(0.999);
	bool isAtk, isRunning;
	float Atau, Dtau, Rtau;
	float env;

	void process(const ProcessArgs &args) override;

};


void AExpADSR::process(const ProcessArgs &args) {
	float sus = params[PARAM_SUS].getValue();

	Atau = clamp(params[PARAM_ATK].getValue(), EPSILON, 5.0);
	Dtau = clamp(params[PARAM_DEC].getValue(), EPSILON, 5.0);
	Rtau = clamp(params[PARAM_REL].getValue(), EPSILON, 5.0);

	bool gate = inputs[IN_GATE].getVoltage() >= 1.0;
	if (gateDetect.process(gate)) {
		isAtk = true;
		isRunning = true;
	}

	if (isRunning) {
		if (gate) {
			if (isAtk) {
				// ATK
				rcf->setTau(Atau);
				env = rcf->process(1.0);
				if (env >= 1.0 - 0.001) {
					isAtk = false;
				}
			}
			else {
				// DEC
				rcf->setTau(Dtau);
				if (env <= sus + 0.001)
					env = sus;
				else
					env = rcf->process(sus);
			}
		} else {
			// REL
			rcf->setTau(Rtau);
			env = rcf->process(0.0);
			if (env <= 0.001)
				isRunning = false;
		}
	} else {
		env = 0.0;
	}


	if (outputs[OUT_ENVELOPE].isConnected()) {
		outputs[OUT_ENVELOPE].setVoltage(10.0 * env);
	}

}

struct AExpADSRWidget : ModuleWidget {
	AExpADSRWidget(AExpADSR * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("AExpADSR");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(12, 52));
			title->setText("ATK");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(12, 102));
			title->setText("DEC");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(12, 152));
			title->setText("SUS");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(12, 202));
			title->setText("REL");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(8, 250));
			title->setText("GATE");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 250));
			title->setText("OUT");
			addChild(title);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 280), module, AExpADSR::IN_GATE));

		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, AExpADSR::OUT_ENVELOPE));

		addParam(createParam<RoundBlackKnob>(Vec(45, 60), module, AExpADSR::PARAM_ATK));
		addParam(createParam<RoundBlackKnob>(Vec(45, 110), module, AExpADSR::PARAM_DEC));
		addParam(createParam<RoundBlackKnob>(Vec(45, 160), module, AExpADSR::PARAM_SUS));
		addParam(createParam<RoundBlackKnob>(Vec(45, 210), module, AExpADSR::PARAM_REL));

		addChild(createLight<SmallLight<GreenLight>>(Vec(20, 310), module, AExpADSR::LIGHT_GATE));

	}

};



Model *modelAExpADSR = createModel<AExpADSR, AExpADSRWidget>("AExpADSR");
