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

#define WF_THRESHOLD (0.7f)
#define SMALL_NUMERIC_TH (1e-30f)
#define EXERCISE_2

template <typename T> int inline sign(T val) {
    return (T(0) < val) - (val < T(0));
}

struct AWavefolder : Module {
	enum ParamIds {
		PARAM_GAIN,
		PARAM_OFFSET,
		PARAM_GAIN_CV,
		PARAM_OFFSET_CV,
		NUM_PARAMS,
	};

	enum InputIds {
		MAIN_IN,
		GAIN_IN,
		OFFSET_IN,
		NUM_INPUTS,
	};

	enum OutputIds {
		MAIN_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	double mu, musqr;
	double Fn1, xn1;
	bool antialias = true;

	AWavefolder() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_GAIN_CV, 0.0, 1.0, 0.0, "Gain CV Amount");
		configParam(PARAM_OFFSET_CV, 0.0, 1.0, 0.0, "Offset CV Amount");
		configParam(PARAM_GAIN, 0.1, 3.0, 1.0, "Input Gain");
		configParam(PARAM_OFFSET, -5.0, 5.0, 0.0, "Input Offset");
		mu = 5.0 * WF_THRESHOLD;
		musqr = mu*mu;
		Fn1 = xn1 = 0.0;
	}

	void setAntialiasing(bool onOff) {
		antialias = onOff;
	}

	void process(const ProcessArgs &args) override;

};

void AWavefolder::process(const ProcessArgs &args) {

	double offset =  params[PARAM_OFFSET_CV].getValue() * inputs[OFFSET_IN].getVoltage() / 10.0 + params[PARAM_OFFSET].getValue();
	double gain = params[PARAM_GAIN_CV].getValue() * inputs[GAIN_IN].getVoltage() / 10.0 + params[PARAM_GAIN].getValue();
	double out, x, z;

	x = z = out = gain * inputs[MAIN_IN].getVoltage() + offset;

	if(antialias) {
		double dif = x - xn1;
		if (dif < SMALL_NUMERIC_TH && dif > -SMALL_NUMERIC_TH) {
			if (x > mu || x < -mu) {
				double avg = 0.5*(x + xn1);
				out = (sign(avg) * 2 * mu - avg);
			}
		} else {
			double F;
			if (x > mu || x < -mu)
				F = -0.5 * x*x + sign(x) * 2 * mu * x - (musqr);
			else
				F = 0.5 * x*x;
			out = (F - Fn1) / (dif);
			Fn1 = F;
		}
		xn1 = z;
	} else {
		if (x > mu || x < -mu)
			out = sign(x) * 2 * mu - x;
	}

#ifdef EXERCISE_2
	out = out * 1.f / gain;
#endif


	if (outputs[MAIN_OUT].isConnected())
		outputs[MAIN_OUT].setVoltage(out);

}

struct AWavefolderWidget : ModuleWidget {
	AWavefolderWidget(AWavefolder * module);
	void appendContextMenu(Menu *menu) override;
};

AWavefolderWidget::AWavefolderWidget(AWavefolder * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("AWavefolder");
		addChild(title);
	}

	{
		ATextHeading * title = new ATextHeading(Vec(18, 30));
		title->setText("IN GAIN");
		addChild(title);
	}
	{
		ATextHeading * title = new ATextHeading(Vec(20, 140));
		title->setText("OFFSET");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(17, 250));
		title->setText("IN");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(50, 250));
		title->setText("OUT");
		addChild(title);
	}

	addInput(createInput<PJ301MPort>(Vec(15, 290), module, AWavefolder::MAIN_IN));
	addInput(createInput<PJ301MPort>(Vec(10, 108), module, AWavefolder::GAIN_IN));
	addInput(createInput<PJ301MPort>(Vec(10, 218), module, AWavefolder::OFFSET_IN));
	addParam(createParam<Trimpot>(Vec(40, 110), module, AWavefolder::PARAM_GAIN_CV));
	addParam(createParam<Trimpot>(Vec(40, 222), module, AWavefolder::PARAM_OFFSET_CV));

	addOutput(createOutput<PJ301MPort>(Vec(50, 290), module, AWavefolder::MAIN_OUT));

	addParam(createParam<RoundBlackKnob>(Vec(35, 70), module, AWavefolder::PARAM_GAIN));
	addParam(createParam<RoundBlackKnob>(Vec(35, 180), module, AWavefolder::PARAM_OFFSET));


}

struct AWavefolderMenuItem : MenuItem {
	AWavefolder *wavefolder;
	bool antialias;
	void onAction(const event::Action &e) override{
		wavefolder->setAntialiasing(antialias);
	}

};

void AWavefolderWidget::appendContextMenu(Menu *menu) {
	AWavefolder *module = dynamic_cast<AWavefolder*>(this->module);

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Antialiasing";
	menu->addChild(modeLabel);

	AWavefolderMenuItem *noantialiasItem1 = new AWavefolderMenuItem();
	noantialiasItem1->text = "OFF";
	noantialiasItem1->wavefolder = module;
	noantialiasItem1->antialias = 0;
	noantialiasItem1->rightText = CHECKMARK(module->antialias == noantialiasItem1->antialias);
	menu->addChild(noantialiasItem1);

	AWavefolderMenuItem *noantialiasItem2 = new AWavefolderMenuItem();
	noantialiasItem2->text = "ON";
	noantialiasItem2->wavefolder = module;
	noantialiasItem2->antialias = 1;
	noantialiasItem2->rightText = CHECKMARK(module->antialias == noantialiasItem2->antialias);
	menu->addChild(noantialiasItem2);

	/* additional spacer for future content
	MenuLabel *spacerLabel2 = new MenuLabel();
	menu->addChild(spacerLabel2);
	*/

}

Model *modelAWavefolder = createModel<AWavefolder, AWavefolderWidget>("AWavefolder");
