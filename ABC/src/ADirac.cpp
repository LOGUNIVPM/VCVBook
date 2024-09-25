#include "ABC.hpp"

struct ADirac : Module {
	enum ParamIds {
		PUSH_BUTTON_1,
		NUM_PARAMS,
	};

	enum InputIds {
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT1,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	dsp::BooleanTrigger bt;

	ADirac() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs &args) override;

};

void ADirac::process(const ProcessArgs &args) {

	// READ BUTTON
	float status = params[PUSH_BUTTON_1].getValue();

	float triggered = bt.process(status);

	// WRITE OUTPUT
	outputs[OUTPUT1].setVoltage(5.f * triggered);

}

struct ADiracWidget : ModuleWidget {

	ADiracWidget(ADirac* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("ADirac");
			addChild(title);
		}

		addParam(createParam<CKD6>(Vec(30, 70), module, ADirac::PUSH_BUTTON_1));

		addOutput(createOutputCentered<PJ301MPort>(Vec(45, 150), module, ADirac::OUTPUT1));


	}

};



Model * modelADirac = createModel<ADirac, ADiracWidget>("ADirac");

