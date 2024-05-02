#include "ABC.hpp"
#include "dsp/filter.hpp"

using namespace rack::dsp;

struct ASimpleFilter : Module {
	enum ParamIds {
		PARAM_CUTOFF,
		NUM_PARAMS,
	};

	enum InputIds {
		MAIN_IN,
		NUM_INPUTS,
	};

	enum OutputIds {
		LPF_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	BiquadFilter LPF;

	ASimpleFilter() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_CUTOFF, 1.f, 4.3f, 2.f, "Cutoff"); // 10^x
		LPF.setParameters(BiquadFilter::LOWPASS, 1000.f, 2.f, 0.f);
	}

	void process(const ProcessArgs &args) override;

};

void ASimpleFilter::process(const ProcessArgs &args) {

	float cutoff = powf(10.f, params[PARAM_CUTOFF].getValue());

	float x = inputs[MAIN_IN].getVoltage();

	float norm_cutoff = 0.5f * args.sampleTime * cutoff; // normalize in range 0-0.5 (1=Fs)
 	LPF.setParameters(BiquadFilter::LOWPASS, norm_cutoff, 2.f, 0.f);

	float y = LPF.process(x);

	outputs[LPF_OUT].setVoltage(y);
}

/* ---------------------------- */

struct ASimpleFilterWidget : ModuleWidget {
	ASimpleFilterWidget(ASimpleFilter * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		addInput(createInput<PJ301MPort>(Vec(32, 230), module, ASimpleFilter::MAIN_IN));
		addOutput(createOutput<PJ301MPort>(Vec(32, 280), module, ASimpleFilter::LPF_OUT));
		addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, ASimpleFilter::PARAM_CUTOFF));

	 	{
			ATitle * title = new ATitle(box.size.x);
			title->setText("BIQUAD LPF");
			addChild(title);
		}
        {
            ATextLabel * title = new ATextLabel(Vec(25, 240));
            title->setText("INPUT");
            addChild(title);
	    }
        {
            ATextLabel * title = new ATextLabel(Vec(22, 290));
            title->setText("OUTPUT");
            addChild(title);
	    }
        {
            ATextLabel * title = new ATextLabel(Vec(20, 90));
            title->setText("CUTOFF");
            addChild(title);
	    }
	}

};

Model *modelASimpleFilter = createModel<ASimpleFilter, ASimpleFilterWidget>("ASimpleFilter");
