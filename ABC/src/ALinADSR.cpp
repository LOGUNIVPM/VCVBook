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

#define EPSILON 1e-9f

struct ALinADSR : Module {
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

	ALinADSR() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_ATK, 0.f, 5.f, 0.5f, "Attack", " s");
		configParam(PARAM_DEC, 0.f, 5.f, 0.5f, "Decay", " s");
		configParam(PARAM_SUS, 0.f, 1.f, 0.5f, "Sustain");
		configParam(PARAM_REL, 0.f, 5.f, 0.5f, "Release", " s");
		isAtk = false;
		isRunning = false;
		env = 0.0;
	}

	dsp::SchmittTrigger gateDetect;
	bool isAtk, isRunning;

	float env;

	void process(const ProcessArgs &args) override;

};

void ALinADSR::process(const ProcessArgs &args) {

	float sus = params[PARAM_SUS].getValue();
	float Astep = 1.f / (EPSILON + args.sampleRate * params[PARAM_ATK].getValue());
	float Dstep = (sus - 1.0) / (EPSILON + args.sampleRate * params[PARAM_DEC].getValue());
	float Rstep = -(sus + EPSILON) / (EPSILON + args.sampleRate * params[PARAM_REL].getValue());

	Astep = clamp(Astep, EPSILON, 0.5);
	Dstep = std::max(Dstep, -0.5f);//risolvere problema: quando d è al minimo ad ogni step env oscilla tra -0.5 e 0.0
	Rstep = std::max(Rstep, -1.f);

	bool gate = inputs[IN_GATE].getVoltage() >= 1.0;
	if (gateDetect.process(gate)) {
		isAtk = true;
		isRunning = true;
	}


	if (isRunning) {
		if (gate) {
			// ATK
			if (isAtk) {
				env += Astep;
				if (env >= 1.0)
					isAtk = false;
			}
			else {
				// DEC
				if (env <= sus + 0.001) {
					env = sus;
				}
				else {
					env += Dstep;
				}
			}
		} else {
			// REL
			env += Rstep;
			if (env <= Rstep)
				isRunning = false;
		}
	} else {
		env = 0.0;
	}

	if (outputs[OUT_ENVELOPE].isConnected()) {
		outputs[OUT_ENVELOPE].setVoltage(10.f * env);
	}
}

struct ALinADSRWidget : ModuleWidget {
	ALinADSRWidget(ALinADSR * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("ALinADSR");
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

		addInput(createInput<PJ301MPort>(Vec(10, 280), module, ALinADSR::IN_GATE));

		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, ALinADSR::OUT_ENVELOPE));

		addParam(createParam<RoundBlackKnob>(Vec(45, 60), module, ALinADSR::PARAM_ATK));
		addParam(createParam<RoundBlackKnob>(Vec(45, 110), module, ALinADSR::PARAM_DEC));
		addParam(createParam<RoundBlackKnob>(Vec(45, 160), module, ALinADSR::PARAM_SUS));
		addParam(createParam<RoundBlackKnob>(Vec(45, 210), module, ALinADSR::PARAM_REL));

		addChild(createLight<SmallLight<GreenLight>>(Vec(20, 310), module, ALinADSR::LIGHT_GATE));

	}
};



Model *modelALinADSR = createModel<ALinADSR, ALinADSRWidget>("ALinADSR");
