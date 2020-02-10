#include "ABC.hpp"
#include "SVF.hpp"

//#define EXERCISE_2
//#define EXERCISE_4

struct ASVFilter : Module {
	enum ParamIds {
		PARAM_CUTOFF,
		PARAM_DAMP,
		NUM_PARAMS,
	};

	enum InputIds {
		MAIN_IN,
#ifdef EXERCISE_4
		CUTOFF_CV,
#endif
		NUM_INPUTS,
	};

	enum OutputIds {
		LPF_OUT,
		BPF_OUT,
		HPF_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	SVF<float> * filter = new SVF<float>(100, 0.1);
	float hpf, bpf, lpf;

	ASVFilter() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
#ifndef EXERCISE_2
	configParam(PARAM_CUTOFF, 0.f, 0.2f, 0.01f, "Cutoff");
#else
	configParam(PARAM_CUTOFF, 1.f, 2.5f, 2.f, "Cutoff");
#endif
	configParam(PARAM_DAMP, 0.000001f, 0.5f, 0.25f);

		hpf = bpf = lpf = 0.f;
	}

	void process(const ProcessArgs &args) override;

};

void ASVFilter::process(const ProcessArgs &args) {
#ifndef EXERCISE_2
	float fc = args.sampleRate * params[PARAM_CUTOFF].getValue();
#else
	float fc = pow(params[PARAM_CUTOFF].getValue(), 10.f);
#endif
#ifdef EXERCISE_4
	fc += pow(rescale(inputs[CUTOFF_CV].getVoltage(), -10.f, 10.f, 0.f, 2.f), 10.f);
#endif

 	filter->setCoeffs(fc, params[PARAM_DAMP].getValue());

	filter->process(inputs[MAIN_IN].getVoltageSum(), &hpf, &bpf, &lpf);

	outputs[LPF_OUT].setVoltage(lpf);
	outputs[BPF_OUT].setVoltage(bpf);
	outputs[HPF_OUT].setVoltage(hpf);

}

struct ASVFilterWidget : ModuleWidget {
	ASVFilterWidget(ASVFilter * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("ASVFilter");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(20, 35));
			title->setText("CUTOFF");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(30, 115));
			title->setText("DAMP");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(13, 250));
			title->setText("IN");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 200));
			title->setText("LPF");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 250));
			title->setText("BPF");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 300));
			title->setText("HPF");
			addChild(title);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 280), module, ASVFilter::MAIN_IN));

		addOutput(createOutput<PJ301MPort>(Vec(55, 230), module, ASVFilter::LPF_OUT));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, ASVFilter::BPF_OUT));
		addOutput(createOutput<PJ301MPort>(Vec(55, 330), module, ASVFilter::HPF_OUT));

		addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, ASVFilter::PARAM_CUTOFF));
		addParam(createParam<RoundBlackKnob>(Vec(30, 150), module, ASVFilter::PARAM_DAMP));

	#ifdef EXERCISE_4
		addInput(createInput<PJ301MPort>(Vec(2, 70), module, ASVFilter::CUTOFF_CV));
	#endif

	}

};

Model *modelASVFilter = createModel<ASVFilter, ASVFilterWidget>("ASVFilter");
