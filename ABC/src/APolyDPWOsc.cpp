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
#include "DPW.hpp"

using namespace::dsp;

#define DPWOSC_TYPE double
#define POLYCHMAX 16


template <typename T>
struct APolyDPWOsc : Module {
	enum ParamIds {
		PITCH_PARAM,
		FMOD_PARAM,
		NUM_PARAMS,
	};

	enum InputIds {
		POLY_VOCT_IN,
		POLY_FMOD_IN,
		NUM_INPUTS,
	};
	enum OutputIds {
		POLY_SAW_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	DPW<T> *Osc[POLYCHMAX];
	unsigned int dpwOrder = 1;

	xpander16f xpMsg[2];


	APolyDPWOsc() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PITCH_PARAM, -54.f, 54.f, 0.f, "Pitch", " Hz", std::pow(2.f, 1.f/12.f), dsp::FREQ_C4, 0.f);
		configParam(FMOD_PARAM, 0.f, 1.f, 0.f, "Modulation");
		for (int ch = 0; ch < POLYCHMAX; ch++)
			Osc[ch] = new DPW<T>();
		rightExpander.producerMessage = (xpander16f*) &xpMsg[0];
		rightExpander.consumerMessage = (xpander16f*) &xpMsg[1];

	}

	void process(const ProcessArgs &args) override;

	void onDPWOrderChange(unsigned int newdpw) {
		for (int ch = 0; ch < POLYCHMAX; ch++)
			dpwOrder = Osc[ch]->onDPWOrderChange(newdpw); // this function also checks the validity of the input
	}

};



template <typename T> void APolyDPWOsc<T>::process(const ProcessArgs &args) {

	float pitchKnob = params[PITCH_PARAM].getValue();

	int inChanN = clamp(inputs[POLY_VOCT_IN].getChannels(), 1, POLYCHMAX);

	int ch;
	for (ch = 0; ch < inChanN; ch++) {

		float pitchCV = 12.f * inputs[POLY_VOCT_IN].getVoltage(ch);
		if (inputs[POLY_FMOD_IN].isConnected()) {
			pitchCV += quadraticBipolar(params[FMOD_PARAM].getValue()) * 12.f * inputs[POLY_FMOD_IN].getPolyVoltage(ch);
		}
		T pitch = dsp::FREQ_C4 * std::pow(2.f, (pitchKnob + pitchCV) / 12.f);

		Osc[ch]->setPitch(pitch);
		T out = Osc[ch]->process();

		outputs[POLY_SAW_OUT].setVoltage(out, ch);

	}
	outputs[POLY_SAW_OUT].setChannels(ch+1);

	bool expanderPresent = (rightExpander.module && rightExpander.module->model == modelAPolyXpander);
	if (expanderPresent) {
		xpander16f* wrMsg = (xpander16f*)rightExpander.producerMessage;
		for (ch = 0; ch < POLYCHMAX; ch++) {
			wrMsg->outs[ch] = outputs[POLY_SAW_OUT].getVoltage(ch);
		}
		rightExpander.messageFlipRequested = true;
	}


}

struct APolyDPWOscWidget : ModuleWidget {
	APolyDPWOscWidget(APolyDPWOsc<DPWOSC_TYPE> * module);
	void appendContextMenu(Menu *menu) override;
};

APolyDPWOscWidget::APolyDPWOscWidget(APolyDPWOsc<DPWOSC_TYPE> * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("APolyDPWOsc");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(5, 100));
		title->setText("V/OCT");
		addChild(title);
	}
	{
		ATextLabel * title = new ATextLabel(Vec(5, 130));
		title->setText("FM");
		addChild(title);
	}
	{
		ATextLabel * title = new ATextLabel(Vec(26, 68));
		title->setText("PITCH");
		addChild(title);
	}
	{
		ATextHeading * title = new ATextHeading(Vec(15, 190));
		title->setText("SAWTOOTH");
		addChild(title);
	}


	addInput(createInput<PJ301MPort>(Vec(55, 108), module, APolyDPWOsc<DPWOSC_TYPE>::POLY_VOCT_IN));
	addInput(createInput<PJ301MPort>(Vec(55, 138), module, APolyDPWOsc<DPWOSC_TYPE>::POLY_FMOD_IN));

	addParam(createParam<RoundBlackKnob>(Vec(30, 40), module, APolyDPWOsc<DPWOSC_TYPE>::PITCH_PARAM));
	addParam(createParam<RoundSmallBlackKnob>(Vec(23,140), module, APolyDPWOsc<DPWOSC_TYPE>::FMOD_PARAM));

	addOutput(createOutput<PJ3410Port>(Vec(30, 230), module, APolyDPWOsc<DPWOSC_TYPE>::POLY_SAW_OUT));

}

struct OscPolyDPWOrderMenuItem : MenuItem {
	APolyDPWOsc<DPWOSC_TYPE> *dpwosc;
	unsigned int dpword;
	void onAction(const event::Action &e) override{
		dpwosc->onDPWOrderChange(dpword);
	}
};

void APolyDPWOscWidget::appendContextMenu(Menu *menu) {
	APolyDPWOsc<DPWOSC_TYPE> *module = dynamic_cast<APolyDPWOsc<DPWOSC_TYPE>*>(this->module);

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "DPW ORDER";
	menu->addChild(modeLabel);

	OscPolyDPWOrderMenuItem *dpw1Item = new OscPolyDPWOrderMenuItem();
	dpw1Item->text = "1st";
	dpw1Item->dpwosc = module;
	dpw1Item->dpword = DPW_1;
	dpw1Item->rightText = CHECKMARK(module->dpwOrder == dpw1Item->dpword);
	menu->addChild(dpw1Item);


	OscPolyDPWOrderMenuItem *dpw2Item = new OscPolyDPWOrderMenuItem();
	dpw2Item->text = "2nd";
	dpw2Item->dpwosc = module;
	dpw2Item->dpword = DPW_2;
	dpw2Item->rightText = CHECKMARK(module->dpwOrder == dpw2Item->dpword);
	menu->addChild(dpw2Item);

	OscPolyDPWOrderMenuItem *dpw3Item = new OscPolyDPWOrderMenuItem();
	dpw3Item->text = "3rd";
	dpw3Item->dpwosc = module;
	dpw3Item->dpword = DPW_3;
	dpw3Item->rightText = CHECKMARK(module->dpwOrder == dpw3Item->dpword);
	menu->addChild(dpw3Item);

	OscPolyDPWOrderMenuItem *dpw4Item = new OscPolyDPWOrderMenuItem();
	dpw4Item->text = "4th";
	dpw4Item->dpwosc = module;
	dpw4Item->dpword = DPW_4;
	dpw4Item->rightText = CHECKMARK(module->dpwOrder == dpw4Item->dpword);
	menu->addChild(dpw4Item);

	/* additional spacer for future content
	MenuLabel *spacerLabel2 = new MenuLabel();
	menu->addChild(spacerLabel2);

	*/
}

Model *modelAPolyDPWOsc = createModel<APolyDPWOsc<DPWOSC_TYPE>, APolyDPWOscWidget>("APolyDPWOsc");
