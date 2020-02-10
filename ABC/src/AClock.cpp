#include "ABC.hpp"
#include "dsp/digital.hpp"

#define TRIG_TIME 1e-3f

struct AClock : Module {
	enum ParamIds {
		BPM_KNOB,
		NUM_PARAMS,
	};
	enum InputIds {
		NUM_INPUTS,
	};
	enum OutputIds {
		PULSE_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		PULSE_LIGHT,
		NUM_LIGHTS,
	};

	dsp::PulseGenerator pgen;
	float counter, period;

	AClock() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BPM_KNOB, 30.0, 360.0, 120.0, "Tempo", "BPM");
		counter = period = 0.f;
	}

	void process(const ProcessArgs &args) override;
};

void AClock::process(const ProcessArgs &args) {

	float BPM = params[BPM_KNOB].getValue();
	period = 60.f * args.sampleRate / BPM; // samples

	if (counter > period) {
		pgen.trigger(TRIG_TIME);
		counter -= period; // keep the fractional part
	}

	counter++;
	float out = pgen.process( args.sampleTime );
	outputs[PULSE_OUT].setVoltage(10.f * out);
	lights[PULSE_LIGHT].setSmoothBrightness(out, 5e-6f);

}

struct AClockWidget : ModuleWidget {
	AClockWidget(AClock * module);
};

AClockWidget::AClockWidget(AClock * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("AClock");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(23, 25));
		title->setText("TEMPO");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(17, 140));
		title->setText("CLK OUT");
		addChild(title);
	}

	addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, AClock::BPM_KNOB));

	addOutput(createOutput<PJ3410Port>(Vec(30, 180), module, AClock::PULSE_OUT));

	addChild(createLight<MediumLight<GreenLight>>(Vec(66, 190), module, AClock::PULSE_LIGHT));

}

Model *modelAClock = createModel<AClock, AClockWidget>("AClock");
