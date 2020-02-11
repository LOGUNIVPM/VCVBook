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

template <typename T>
struct RCDiode : RCFilter<T> {

	RCDiode(T aCoeff) {
		this->a = aCoeff;
		this->reset();
	}

	T charge(T vi) {
		return this->yn = this->yn1 = vi;
	}

};

struct AEnvFollower : Module {
	enum ParamIds {
		PARAM_TAU,
		NUM_PARAMS,
	};

	enum InputIds {
		MAIN_IN,
		TAU_CV_IN,
		NUM_INPUTS,
	};

	enum OutputIds {
		MAIN_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		ENV_LIGHT,
		NUM_LIGHTS,
	};

	AEnvFollower() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_TAU, 0.0, 0.5, 0.01);
	}

	RCDiode<float> * rcd = new RCDiode<float>(0.999f);
	float env = 0.0;
	void process(const ProcessArgs &args) override;

};

void AEnvFollower::process(const ProcessArgs &args) {
#ifndef EXERCISE_1
	float tau = clamp(params[PARAM_TAU].getValue(), EPSILON, 5.0);

	rcd->setTau(tau);
	float rectified = std::abs(inputs[MAIN_IN].getVoltage());
	if (rectified > env)
		env = rcd->charge(rectified);
	else
		env = rcd->process(rectified);

#else // LINEAR DECAY
	float Rstep = (-1.0) / (EPSILON + args.sampleRate * params[PARAM_TAU].getValue());

	float in = std::abs(inputs[MAIN_IN].getVoltage());
	if (in > env + 0.001) {
		env = in;
	} else {
		env = std::max(env + Rstep, 0.f);
	}
#endif

	if (outputs[MAIN_OUT].isConnected()) {
		outputs[MAIN_OUT].setVoltage(lights[ENV_LIGHT].value = env);
	}

}

struct AEnvFollowerWidget : ModuleWidget {
	AEnvFollowerWidget(AEnvFollower * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);


		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("AEnvFollower");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(30, 70));
			title->setText("TAU");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(13, 250));
			title->setText("IN");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 250));
			title->setText("OUT");
			addChild(title);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 280), module, AEnvFollower::MAIN_IN));

		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, AEnvFollower::MAIN_OUT));

		addParam(createParam<RoundBlackKnob>(Vec(30, 110), module, AEnvFollower::PARAM_TAU));

		addChild(createLight<SmallLight<GreenLight>>(Vec(20, 310), module, AEnvFollower::ENV_LIGHT));

	}
};



Model *modelAEnvFollower = createModel<AEnvFollower, AEnvFollowerWidget>("AEnvFollower");
