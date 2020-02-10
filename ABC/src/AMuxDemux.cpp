#include "ABC.hpp"


struct AMuxDemux : Module {

	enum ParamIds {
		M_SELECTOR_PARAM,
		D_SELECTOR_PARAM,
		NUM_PARAMS,
	};
	enum InputIds {
		M_INPUT_1,
		M_INPUT_2,
		M_INPUT_3,
		M_INPUT_4,
		D_MAIN_IN,
		NUM_INPUTS,
		N_MUX_IN = M_INPUT_4,
	};
	enum OutputIds {
		D_OUTPUT_1,
		D_OUTPUT_2,
		D_OUTPUT_3,
		D_OUTPUT_4,
		M_MAIN_OUT,
		NUM_OUTPUTS,
		N_DEMUX_OUT = D_OUTPUT_4,
	};

	enum LightsIds {
		M_LIGHT_1,
		M_LIGHT_2,
		M_LIGHT_3,
		M_LIGHT_4,
		D_LIGHT_1,
		D_LIGHT_2,
		D_LIGHT_3,
		D_LIGHT_4,
		NUM_LIGHTS,
	};

	unsigned int selMux, selDemux;
	AMuxDemux() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(M_SELECTOR_PARAM, 0.0, 3.0, 0.0, "Mux Selector");
		configParam(D_SELECTOR_PARAM, 0.0, 3.0, 0.0, "Demux Selector");
		selMux = selDemux = 0;
	}
	void process(const ProcessArgs &args) override ;

};

void AMuxDemux::process(const ProcessArgs &args) {

	/* MUX */
	lights[selMux].setBrightness(0.f);
	selMux = (unsigned int)clamp((int)params[M_SELECTOR_PARAM].getValue(), 0, N_MUX_IN);
	lights[selMux].setBrightness(1.f);

	if (outputs[M_MAIN_OUT].isConnected()) {
		if (inputs[selMux].isConnected()) {
			outputs[M_MAIN_OUT].setVoltage(inputs[selMux].getVoltage());
		}
	}

	/* DEMUX */
	lights[selDemux+N_MUX_IN+1].setBrightness(0.f);
	selDemux = (unsigned int)clamp((int)params[D_SELECTOR_PARAM].getValue(), 0, N_DEMUX_OUT);
	lights[selDemux+N_MUX_IN+1].setBrightness(1.f);

	if (inputs[D_MAIN_IN].isConnected()) {
		if (outputs[selDemux].isConnected()) {
			outputs[selDemux].setVoltage(inputs[D_MAIN_IN].getVoltage());
		}
	}
}

struct AMuxDemuxWidget : ModuleWidget {

	#define D_Y 190 // demux Y
	AMuxDemuxWidget(AMuxDemux * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);


		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("AMux");
			addChild(title);
		}


		{
			ATextHeading * title = new ATextHeading(Vec(30, 20));
			title->setText("MUX");
			addChild(title);
		}


		for (int i = AMuxDemux::M_INPUT_1; i <= AMuxDemux::M_INPUT_4; i++) {
			addInput(createInput<PJ301MPort>(Vec(10, 80+(i*30)), module, i));
			addChild(createLight<TinyLight<GreenLight>>(Vec(40, 90+(i*30)), module, i));
		}

		addOutput(createOutput<PJ3410Port>(Vec(50, 100), module, AMuxDemux::M_MAIN_OUT));

		addParam(createParam<RoundBlackSnapKnob>(Vec(50, 60), module, AMuxDemux::M_SELECTOR_PARAM));

		{
			ATextHeading * title = new ATextHeading(Vec(20, D_Y));
			title->setText("DEMUX");
			addChild(title);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 100+D_Y), module, AMuxDemux::D_MAIN_IN));

		for (int i = AMuxDemux::D_OUTPUT_1; i <= AMuxDemux::D_OUTPUT_4; i++) {
			addOutput(createOutput<PJ3410Port>(Vec(50, (i*30)+50+D_Y), module, i));
			addChild(createLight<TinyLight<GreenLight>>(Vec(44, (i*30)+60+D_Y), module, i+AMuxDemux::D_LIGHT_1));
		}

		addParam(createParam<RoundBlackSnapKnob>(Vec(10, 60+D_Y), module, AMuxDemux::D_SELECTOR_PARAM));
	}

};


Model *modelAMuxDemux = createModel<AMuxDemux, AMuxDemuxWidget>("AMuxDemux");
