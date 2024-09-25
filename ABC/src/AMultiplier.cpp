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

struct AMultiplier : Module {
	enum ParamIds {
		GAIN_A1,
		GAIN_B1,
		GAIN_A2,
		GAIN_B2,
		NUM_PARAMS,
	};
	enum InputIds {
		INPUT_X1,
		INPUT_Y1,
		INPUT_X2,
		INPUT_Y2,
		NUM_INPUTS,
	};
	enum OutputIds {
		OUTPUT_Z1,
		OUTPUT_Z2,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		LIGHT_1,
		LIGHT_2,
		NUM_LIGHTS,
	};

	AMultiplier() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(GAIN_A1, 0.1f, 2.f, 1.f, "Gain");
		configParam(GAIN_B1, 0.1f, 2.f, 1.f, "Gain");
		configParam(GAIN_A2, 0.1f, 2.f, 1.f, "Gain");
		configParam(GAIN_B2, 0.1f, 2.f, 1.f, "Gain");
	}

	void process(const ProcessArgs &args) override;

};

void AMultiplier::process(const ProcessArgs &args) {

	float in1, in2, out;
	if (inputs[INPUT_X1].isConnected())
		in1 = params[GAIN_A1].getValue() * inputs[INPUT_X1].getVoltage();
	else
		in1 = params[GAIN_A1].getValue();

	if (inputs[INPUT_Y1].isConnected())
		in2 = params[GAIN_B1].getValue() * inputs[INPUT_Y1].getVoltage();
	else
		in2 = params[GAIN_B1].getValue();

	out = in1 * in2;
	outputs[OUTPUT_Z1].setVoltage(out);
	lights[LIGHT_1].setBrightness(out);

	if (inputs[INPUT_X2].isConnected())
		in1 = params[GAIN_A2].getValue() * inputs[INPUT_X2].getVoltage();
	else
		in1 = params[GAIN_A2].getValue();

	if (inputs[INPUT_Y2].isConnected())
		in2 = params[GAIN_B2].getValue() * inputs[INPUT_Y2].getVoltage();
	else
		in2 = params[GAIN_B2].getValue();

	out = in1 * in2;
	outputs[OUTPUT_Z2].setVoltage(out);
	lights[LIGHT_2].setBrightness(out);

}

struct AMultiplierWidget : ModuleWidget {

	AMultiplierWidget(AMultiplier* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("AMultiplier");
			addChild(title);
		}

		{
			ATextHeading * hd = new ATextHeading(Vec(0, 20));
			hd->setText("MULTIPLIER 1");
			addChild(hd);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(65, 60-8));
			lbl->setText("X1");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(65, 110-8));
			lbl->setText("Y1");
			addChild(lbl);
		}

		{
			ATextHeading * hd = new ATextHeading(Vec(0, 180));
			hd->setText("MULTIPLIER 2");
			addChild(hd);
		}


		{
			ATextLabel * lbl = new ATextLabel(Vec(65, 220-8));
			lbl->setText("X2");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(65, 270-8));
			lbl->setText("Y2");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(65, 160-12));
			lbl->setText("Z1");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(65, 320-12));
			lbl->setText("Z2");
			addChild(lbl);
		}
		addInput(createInput<PJ3410Port>(Vec(30, 60), module, AMultiplier::INPUT_X1));
		addInput(createInput<PJ3410Port>(Vec(30, 110), module, AMultiplier::INPUT_Y1));
		addInput(createInput<PJ3410Port>(Vec(30, 220), module, AMultiplier::INPUT_X2));
		addInput(createInput<PJ3410Port>(Vec(30, 270), module, AMultiplier::INPUT_Y2));

		{
			ATextLabel * lbl = new ATextLabel(Vec(65+3, 78));
			lbl->setText("x");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(65+3, 130-5));
			lbl->setText("=");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(65+3, 238));
			lbl->setText("x");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(65+3, 290-5));
			lbl->setText("=");
			addChild(lbl);
		}

		addParam(createParam<Trimpot>(Vec(8, 60+6), module, AMultiplier::GAIN_A1));
		addParam(createParam<Trimpot>(Vec(8, 110+6), module, AMultiplier::GAIN_B1));
		addParam(createParam<Trimpot>(Vec(8, 220+6), module, AMultiplier::GAIN_A2));
		addParam(createParam<Trimpot>(Vec(8, 270+6), module, AMultiplier::GAIN_B2));

		addOutput(createOutput<PJ301MPort>(Vec(40-6, 160), module, AMultiplier::OUTPUT_Z1));
		addOutput(createOutput<PJ301MPort>(Vec(40-6, 320), module, AMultiplier::OUTPUT_Z2));



		addChild(createLight<SmallLight<GreenLight>>(Vec(25, 160+8), module, AMultiplier::LIGHT_1));
		addChild(createLight<SmallLight<GreenLight>>(Vec(25, 320+8), module, AMultiplier::LIGHT_2));

	}

};



Model * modelAMultiplier = createModel<AMultiplier, AMultiplierWidget>("AMultiplier");
