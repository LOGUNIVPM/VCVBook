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

template <typename T>
struct ADPWOsc : Module {
	enum ParamIds {
		PITCH_PARAM,
		FMOD_PARAM,
		NUM_PARAMS,
	};

	enum InputIds {
		VOCT_IN,
		FMOD_IN,
		NUM_INPUTS,
	};
	enum OutputIds {
		SAW_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	DPW<T> *Osc;
	unsigned int dpwOrder = 1;

	ADPWOsc() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PITCH_PARAM, -54.f, 54.f, 0.f, "Pitch", " Hz", std::pow(2.f, 1.f/12.f), dsp::FREQ_C4, 0.f);
		configParam(FMOD_PARAM, 0.f, 1.f, 0.f, "Modulation");
		Osc = new DPW<T>();
	}

	void process(const ProcessArgs &args) override;

	void onDPWOrderChange(unsigned int newdpw) {
		dpwOrder = Osc->onDPWOrderChange(newdpw); // this function also checks the validity of the input
	}

};


template <typename T> void ADPWOsc<T>::process(const ProcessArgs &args) {

	float pitchKnob = params[PITCH_PARAM].getValue();
	float pitchCV = 12.f * inputs[VOCT_IN].getVoltage();
	if (inputs[FMOD_IN].isConnected()) {
		pitchCV += quadraticBipolar(params[FMOD_PARAM].getValue()) * 12.f * inputs[FMOD_IN].getVoltage();
	}
	T pitch = dsp::FREQ_C4 * std::pow(2.f, (pitchKnob + pitchCV) / 12.f);

	Osc->setPitch(pitch);
	T out = Osc->process();

	if(outputs[SAW_OUT].isConnected()) {
		outputs[SAW_OUT].setVoltage(5.f * out);
	}

}

struct ADPWOscWidget : ModuleWidget {
	ADPWOscWidget(ADPWOsc<DPWOSC_TYPE> * module);
	void appendContextMenu(Menu *menu) override;
};

ADPWOscWidget::ADPWOscWidget(ADPWOsc<DPWOSC_TYPE> * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("ADPWOsc");
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


	addInput(createInput<PJ301MPort>(Vec(55, 108), module, ADPWOsc<DPWOSC_TYPE>::VOCT_IN));
	addInput(createInput<PJ301MPort>(Vec(55, 138), module, ADPWOsc<DPWOSC_TYPE>::FMOD_IN));

	addParam(createParam<RoundBlackKnob>(Vec(30, 40), module, ADPWOsc<DPWOSC_TYPE>::PITCH_PARAM));
	addParam(createParam<RoundSmallBlackKnob>(Vec(23,140), module, ADPWOsc<DPWOSC_TYPE>::FMOD_PARAM));

	addOutput(createOutput<PJ3410Port>(Vec(30, 230), module, ADPWOsc<DPWOSC_TYPE>::SAW_OUT));

}

struct OscDPWOrderMenuItem : MenuItem {
	ADPWOsc<DPWOSC_TYPE> *dpwosc;
	unsigned int dpword;
	void onAction(const event::Action &e) override{
		dpwosc->onDPWOrderChange(dpword);
	}
};

void ADPWOscWidget::appendContextMenu(Menu *menu) {
	ADPWOsc<DPWOSC_TYPE> *module = dynamic_cast<ADPWOsc<DPWOSC_TYPE>*>(this->module);

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "DPW ORDER";
	menu->addChild(modeLabel);

	OscDPWOrderMenuItem *dpw1Item = new OscDPWOrderMenuItem();
	dpw1Item->text = "1st";
	dpw1Item->dpwosc = module;
	dpw1Item->dpword = DPW_1;
	dpw1Item->rightText = CHECKMARK(module->dpwOrder == dpw1Item->dpword);
	menu->addChild(dpw1Item);


	OscDPWOrderMenuItem *dpw2Item = new OscDPWOrderMenuItem();
	dpw2Item->text = "2nd";
	dpw2Item->dpwosc = module;
	dpw2Item->dpword = DPW_2;
	dpw2Item->rightText = CHECKMARK(module->dpwOrder == dpw2Item->dpword);
	menu->addChild(dpw2Item);

	OscDPWOrderMenuItem *dpw3Item = new OscDPWOrderMenuItem();
	dpw3Item->text = "3rd";
	dpw3Item->dpwosc = module;
	dpw3Item->dpword = DPW_3;
	dpw3Item->rightText = CHECKMARK(module->dpwOrder == dpw3Item->dpword);
	menu->addChild(dpw3Item);

	OscDPWOrderMenuItem *dpw4Item = new OscDPWOrderMenuItem();
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

Model *modelADPWOsc = createModel<ADPWOsc<DPWOSC_TYPE>, ADPWOscWidget>("ADPWOsc");
