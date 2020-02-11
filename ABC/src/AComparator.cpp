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

struct AComparator : Module {
	enum ParamIds {
		NUM_PARAMS,
	};
	enum InputIds {
		INPUTA1,
		INPUTB1,
		INPUTA2,
		INPUTB2,
		NUM_INPUTS,
	};
	enum OutputIds {
		OUTPUT1,
		OUTPUT2,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		LIGHT_1,
		LIGHT_2,
		NUM_LIGHTS,
	};

	AComparator() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs &args) override;

};

void AComparator::process(const ProcessArgs &args) {

	for (int o = 0; o < NUM_OUTPUTS; o++) {
		if (inputs[o*2].isConnected() && inputs[o*2+1].isConnected()) {
			float out = inputs[o*2].getVoltage() >= inputs[o*2+1].getVoltage();
			outputs[o].setVoltage(out * 10.f);
			lights[o].setBrightness(out);
		}
	}
}

struct AComparatorWidget : ModuleWidget {

	AComparatorWidget(AComparator* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AComparator.svg")));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.077, 28.048)), module, AComparator::INPUTA1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.077, 38.631)), module, AComparator::INPUTB1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.783, 84.252)), module, AComparator::INPUTA2));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.783, 95.541)), module, AComparator::INPUTB2));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.675, 50.146)), module, AComparator::OUTPUT1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.783, 106.829)), module, AComparator::OUTPUT2));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(27.781, 50.448)), module, AComparator::LIGHT_1));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(27.781, 107.069)), module, AComparator::LIGHT_2));

	}

};



Model * modelAComparator = createModel<AComparator, AComparatorWidget>("AComparator");
