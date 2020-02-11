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

#define TRIG_TIME 1e-3f

struct div2 {
    bool status;

    div2() { status = false; }

    bool process() {
        status ^= 1;
        return status;
    }
};


struct ADivider : Module {
	enum ParamIds {
		NUM_PARAMS,
	};
	enum InputIds {
		MAIN_IN,
		NUM_INPUTS,
	};
	enum OutputIds {
		OUTPUT1, // this output will be hidden
		OUTPUT2,
		OUTPUT4,
		OUTPUT8,
		OUTPUT16,
		OUTPUT32,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		LIGHTS1,
		LIGHT2,
		LIGHT4,
		LIGHT8,
		LIGHT16,
		LIGHT32,
		NUM_LIGHTS,
	};

	div2 dividers[NUM_OUTPUTS];
	dsp::PulseGenerator pgen[NUM_OUTPUTS];

	void iterActiv(int idx) {
		if (idx > NUM_OUTPUTS-1) return; // stop iteration
		bool activation = dividers[idx].process();
		pgen[idx].trigger(TRIG_TIME);
		if (activation) {
			iterActiv(idx+1);
		}
	}

	dsp::SchmittTrigger edgeDetector;

	ADivider() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs &args) override;

};

void ADivider::process(const ProcessArgs &args) {

	if (edgeDetector.process(inputs[MAIN_IN].getVoltage())) {
		iterActiv(0); // this will run the first divider (/2) and iterate through the next if necessary
	}

	for (int o = 0; o < NUM_OUTPUTS; o++) {
		float out = pgen[o].process( args.sampleTime );
		outputs[o].setVoltage(10.f * out);
		lights[o].setSmoothBrightness(out, 5e-6f);
	}

}

struct ADividerWidget : ModuleWidget {
	ADividerWidget(ADivider * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ADivider.svg")));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.812, 24.746)), module, ADivider::MAIN_IN));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.261, 52.338)), module, ADivider::OUTPUT2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.261, 66.954)), module, ADivider::OUTPUT4));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.261, 81.511)), module, ADivider::OUTPUT8));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.261, 96.035)), module, ADivider::OUTPUT16));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.261, 110.615)), module, ADivider::OUTPUT32));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.78, 44.878)), module, ADivider::LIGHT2));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.78, 59.166)), module, ADivider::LIGHT4));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.78, 73.982)), module, ADivider::LIGHT8));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.732, 88.515)), module, ADivider::LIGHT16));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.78, 103.086)), module, ADivider::LIGHT32));

	}
};



Model *modelADivider = createModel<ADivider, ADividerWidget>("ADivider");
