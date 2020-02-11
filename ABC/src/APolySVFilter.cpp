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
#include "SVF.hpp"

#define POLYCHMAX 16

struct APolySVFilter : Module {
	enum ParamIds {
		PARAM_CUTOFF,
		PARAM_DAMP,
		NUM_PARAMS,
	};

	enum InputIds {
		POLY_IN,
		POLY_CUTOFF_CV,
		NUM_INPUTS,
	};

	enum OutputIds {
		POLY_LPF_OUT,
		POLY_BPF_OUT,
		POLY_HPF_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	SVF<float> * filter[POLYCHMAX];
	float hpf, bpf, lpf;

	APolySVFilter() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_CUTOFF, 1.f, 2.5f, 2.f, "Cutoff");
		configParam(PARAM_DAMP, 0.000001f, 0.5f, 0.25f);

		for (int ch = 0; ch < POLYCHMAX; ch++)
			filter[ch] = new SVF<float>(100, 0.1);
	}

	void process(const ProcessArgs &args) override;

};

void APolySVFilter::process(const ProcessArgs &args) {

	float knobFc = pow(params[PARAM_CUTOFF].getValue(), 10.f);
	float damp = params[PARAM_DAMP].getValue();

	int inChanN = std::min(POLYCHMAX, inputs[POLY_IN].getChannels());

	int ch;
	for (ch = 0; ch < inChanN; ch++) {

		float fc = knobFc +
				std::pow(rescale(inputs[POLY_CUTOFF_CV].getVoltage(ch), -10.f, 10.f, 0.f, 2.f), 10.f);

		filter[ch]->setCoeffs(fc, damp);

		filter[ch]->process(inputs[POLY_IN].getVoltage(ch), &hpf, &bpf, &lpf);

		outputs[POLY_LPF_OUT].setVoltage(lpf, ch);
		outputs[POLY_BPF_OUT].setVoltage(bpf, ch);
		outputs[POLY_HPF_OUT].setVoltage(hpf, ch);
	}

	outputs[POLY_LPF_OUT].setChannels(ch + 1);
	outputs[POLY_BPF_OUT].setChannels(ch + 1);
	outputs[POLY_HPF_OUT].setChannels(ch + 1);

}

struct APolySVFilterWidget : ModuleWidget {
	APolySVFilterWidget(APolySVFilter * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("APolySVFilter");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(20, 35));
			title->setText("CUTOFF");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(30, 115));
			title->setText("DAMP");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(13, 250));
			title->setText("IN");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 200));
			title->setText("LPF");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 250));
			title->setText("BPF");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 300));
			title->setText("HPF");
			addChild(title);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 280), module, APolySVFilter::POLY_IN));

		addOutput(createOutput<PJ301MPort>(Vec(55, 230), module, APolySVFilter::POLY_LPF_OUT));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, APolySVFilter::POLY_BPF_OUT));
		addOutput(createOutput<PJ301MPort>(Vec(55, 330), module, APolySVFilter::POLY_HPF_OUT));

		addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, APolySVFilter::PARAM_CUTOFF));
		addParam(createParam<RoundBlackKnob>(Vec(30, 150), module, APolySVFilter::PARAM_DAMP));

		addInput(createInput<PJ301MPort>(Vec(2, 70), module, APolySVFilter::POLY_CUTOFF_CV));

	}

};

Model *modelAPolySVFilter = createModel<APolySVFilter, APolySVFilterWidget>("APolySVFilter");
