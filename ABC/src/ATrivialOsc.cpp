#include "ABC.hpp"
#include "dsp/resampler.hpp"
#include "dsp/common.hpp"

using namespace::dsp;

enum {
	OVSF_1 = 1,
	OVSF_2 = 2,
	OVSF_4 = 4,
	OVSF_8 = 8,
	MAX_OVERSAMPLE = OVSF_8,
};

struct ATrivialOsc : Module {
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

	float saw_out[MAX_OVERSAMPLE] = {};
	float out;
	unsigned int ovsFactor = 1;
	Decimator<2,2> d2;
	Decimator<4,4> d4;
	Decimator<8,8> d8;
	int delme = 0; unsigned long cum = 0;

	ATrivialOsc() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(PITCH_PARAM, -54.0, 54.0, 0.0, "Pitch", " Hz", std::pow(2.f, 1.f/12.f), dsp::FREQ_C4, 0.f);
		configParam(FMOD_PARAM, 0.0, 1.0, 0.0, "Modulation");

		out = 0.0;
	}

	void process(const ProcessArgs &args) override;

	void onOvsFactorChange(unsigned int newovsf) {
		ovsFactor = newovsf;
		memset(saw_out, 0, sizeof(saw_out));
	}

};

void ATrivialOsc::process(const ProcessArgs &args) {

	float pitchKnob = params[PITCH_PARAM].getValue();
	float pitchCV = 12.f * inputs[VOCT_IN].getVoltage();
	if (inputs[FMOD_IN].isConnected()) {
		pitchCV += quadraticBipolar(params[FMOD_PARAM].getValue()) * 12.f * inputs[FMOD_IN].getVoltage();
	}
	float pitch = dsp::FREQ_C4 * std::pow(2.f, (pitchKnob + pitchCV) / 12.f);

	float incr = pitch / ((float)ovsFactor * args.sampleRate);

	if (ovsFactor > 1) {
		saw_out[0] = saw_out[ovsFactor-1] + incr;
		for (unsigned int i = 1; i < ovsFactor; i++) {
			saw_out[i] = saw_out[i-1] + incr;
			if (saw_out[i] > 1.0) saw_out[i] -= 1.0;
		}
	} else {
		saw_out[0] += incr;
		if (saw_out[0] > 1.0) saw_out[0] -= 1.0;
	}

	switch(ovsFactor) {
	case OVSF_2:
		out = d2.process(saw_out);
		break;
	case OVSF_4:
		out = d4.process(saw_out);
		break;
	case OVSF_8:
		out = d8.process(saw_out);
		break;
	case OVSF_1:
	default:
		out = saw_out[0];
		break;
	}


	if(outputs[SAW_OUT].isConnected()) {
		outputs[SAW_OUT].setVoltage(10.f * (out - 0.5));
	}

}

struct ATrivialOscWidget : ModuleWidget {
	void appendContextMenu(Menu *menu) override;
	ATrivialOscWidget(ATrivialOsc * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("ATrivialOsc");
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
			ATextHeading * title = new ATextHeading(Vec(25, 190));
			title->setText("SAW");
			addChild(title);
		}


		addInput(createInput<PJ301MPort>(Vec(55, 108), module, ATrivialOsc::VOCT_IN));
		addInput(createInput<PJ301MPort>(Vec(55, 138), module, ATrivialOsc::FMOD_IN));

		addParam(createParam<RoundBlackKnob>(Vec(30, 40), module, ATrivialOsc::PITCH_PARAM));
		addParam(createParam<RoundSmallBlackKnob>(Vec(23,140), module, ATrivialOsc::FMOD_PARAM));

		addOutput(createOutput<PJ3410Port>(Vec(30, 230), module, ATrivialOsc::SAW_OUT));

	}
};



struct OscOversamplingMenuItem : MenuItem {
	ATrivialOsc *module;
	unsigned int ovsf;
	void onAction(const event::Action &e) override{
		module->onOvsFactorChange(ovsf);
	}
};

void ATrivialOscWidget::appendContextMenu(Menu *menu) {
	ATrivialOsc *module = dynamic_cast<ATrivialOsc*>(this->module);

	menu->addChild(new MenuEntry);


	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Oversampling";
	menu->addChild(modeLabel);

	OscOversamplingMenuItem *ovsf1Item = new OscOversamplingMenuItem();
	ovsf1Item->text = "1x";
	ovsf1Item->module = module;
	ovsf1Item->ovsf = OVSF_1;
	ovsf1Item->rightText = CHECKMARK(module->ovsFactor == ovsf1Item->ovsf);
	menu->addChild(ovsf1Item);

	OscOversamplingMenuItem *ovsf2Item = new OscOversamplingMenuItem();
	ovsf2Item->text = "2x";
	ovsf2Item->module = module;
	ovsf2Item->ovsf = OVSF_2;
	ovsf2Item->rightText = CHECKMARK(module->ovsFactor == ovsf2Item->ovsf);
	menu->addChild(ovsf2Item);

	OscOversamplingMenuItem *ovsf4Item = new OscOversamplingMenuItem();
	ovsf4Item->text = "4x";
	ovsf4Item->module = module;
	ovsf4Item->ovsf = OVSF_4;
	ovsf4Item->rightText = CHECKMARK(module->ovsFactor == ovsf4Item->ovsf);
	menu->addChild(ovsf4Item);

	OscOversamplingMenuItem *ovsf8Item = new OscOversamplingMenuItem();
	ovsf8Item->text = "8x";
	ovsf8Item->module = module;
	ovsf8Item->ovsf = OVSF_8;
	ovsf8Item->rightText = CHECKMARK(module->ovsFactor == ovsf8Item->ovsf);
	menu->addChild(ovsf8Item);

	/* additional spacer for future content
	MenuLabel *spacerLabel2 = new MenuLabel();
	menu->addChild(spacerLabel2);
	*/

}

Model *modelATrivialOsc = createModel<ATrivialOsc, ATrivialOscWidget>("ATrivialOsc");
