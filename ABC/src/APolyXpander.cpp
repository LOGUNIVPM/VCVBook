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

using namespace::dsp;

#define DPWOSC_TYPE double
#define POLYCHMAX 16


struct APolyXpander : Module {
	enum ParamIds {
		NUM_PARAMS,
	};

	enum InputIds {
		NUM_INPUTS,
	};

	enum OutputIds {
		ENUMS(SEPARATE_OUTS,16),
		NUM_OUTPUTS,
	};

	enum LightsIds {
		//ENUMS(SEPARATE_LIGHTS,16),
		NUM_LIGHTS,
	};




	APolyXpander() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs &args) override;

};



void APolyXpander::process(const ProcessArgs &args) {

	bool parentConnected = leftExpander.module && leftExpander.module->model == modelAPolyDPWOsc;
    if (parentConnected) {
    	xpander16f* rdMsg = (xpander16f*)leftExpander.module->rightExpander.consumerMessage;
		for (int ch = 0; ch < POLYCHMAX; ch++) {
			outputs[SEPARATE_OUTS+ch].setVoltage(rdMsg->outs[ch]);
		}
    } else {
    	for (int ch = 0; ch < POLYCHMAX; ch++) {
			outputs[SEPARATE_OUTS+ch].setVoltage(0.f);
		}
    }

}

struct APolyXpanderWidget : ModuleWidget {
	APolyXpanderWidget(APolyXpander * module);
};

APolyXpanderWidget::APolyXpanderWidget(APolyXpander * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("AP-XP");
		addChild(title);
	}

	for (int i = 0; i < 7; i++) {
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.7, 37+i*11)), module, APolyXpander::SEPARATE_OUTS + i));
	}

	for (int i = 8; i < 15; i++) {
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.2, 37+(i-8)*11)), module, APolyXpander::SEPARATE_OUTS + i));
	}

}


Model *modelAPolyXpander = createModel<APolyXpander, APolyXpanderWidget>("APolyXpander");
