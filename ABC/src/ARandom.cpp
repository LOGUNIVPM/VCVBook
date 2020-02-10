#include "ABC.hpp"


struct ARandom : Module {
	enum ParamIds {
		PARAM_HOLD,
		PARAM_DISTRIB,
		NUM_PARAMS,
	};
	enum InputIds {
		NUM_INPUTS,
	};
	enum OutputIds {
		RANDOM_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	enum {
		DISTR_UNIFORM = 0,
		DISTR_NORMAL = 1,
		NUM_DISTRIBUTIONS
	};

	int counter;
	float value;

	ARandom() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
#ifdef EXERCISE_1
		configParam(PARAM_HOLD, -2.f, 6.f, 2.f, "Hold tempo", " BPM", 2.f, 60.f, 0.f);
#else
		configParam(PARAM_HOLD, 0, 1, 1, "Hold time", " s");
#endif
		configParam(PARAM_DISTRIB, 0.0, 1.0, 0.0, "Distribution");
		counter = 0;
		value = 0.f;
	}

#ifdef EXERCISE_1
	void onSampleRateChange() override {
		paramQuantities[PARAM_HOLD]->displayBase = APP->engine->getSampleRate();
		paramQuantities[PARAM_HOLD]->displayMultiplier = 1.f / APP->engine->getSampleRate();
	}
#endif

	void process(const ProcessArgs &args) override;
};

void ARandom::process(const ProcessArgs &args) {

#ifdef EXERCISE_1
	float BPM = std::pow(2.f, params[PARAM_HOLD].getValue());
	int hold = std::floor( args.sampleRate / BPM);
#else
	int hold = std::floor(params[PARAM_HOLD].getValue() * args.sampleRate);
#endif
	int distr = std::round(params[PARAM_DISTRIB].getValue());

	if (counter >= hold) {
		if (distr == DISTR_UNIFORM)
			value = 10.f * random::uniform();
		else
			value = clamp(5.f * random::normal(), -10.f, 10.f);
		counter = 0;
	}

	counter++;

	outputs[RANDOM_OUT].setVoltage(value);
}

struct ARandomWidget : ModuleWidget {
	ARandomWidget(ARandom * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("ARandom");
			addChild(title);
		}

#ifdef EXERCISE_1
		{
			ATextLabel * title = new ATextLabel(Vec(28, 35));
			title->setText("TEMPO");
			addChild(title);
		}
#else
		{
			ATextLabel * title = new ATextLabel(Vec(28, 35));
			title->setText("TIME");
			addChild(title);
		}
#endif

		{
			ATextLabel * title = new ATextLabel(Vec(13, 120));
			title->setText("GAUSSIAN");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(17, 172));
			title->setText("UNIFORM");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(32, 210));
			title->setText("OUT");
			addChild(title);
		}


		addOutput(createOutput<PJ3410Port>(Vec(30, 250), module, ARandom::RANDOM_OUT));

		addParam(createParam<valueKnob>(Vec(30, 70), module, ARandom::PARAM_HOLD));
		addParam(createParam<CKSS>(Vec(38, 160), module, ARandom::PARAM_DISTRIB));

	}
};



Model *modelARandom = createModel<ARandom, ARandomWidget>("ARandom");
