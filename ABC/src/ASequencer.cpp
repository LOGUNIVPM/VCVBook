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

#define TRIG_TIME 1e-3f

struct ASequencer : Module {
	enum ParamIds {
		PARAM_STEP_1,
		PARAM_STEP_2,
		PARAM_STEP_3,
		PARAM_STEP_4,
		PARAM_STEP_5,
		PARAM_STEP_6,
		PARAM_STEP_7,
		PARAM_STEP_8,
		NUM_PARAMS,
	};
	enum InputIds {
		MAIN_IN,
		NUM_INPUTS,
	};
	enum OutputIds {
		MAIN_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		LIGHT_STEP_1,
		LIGHT_STEP_2,
		LIGHT_STEP_3,
		LIGHT_STEP_4,
		LIGHT_STEP_5,
		LIGHT_STEP_6,
		LIGHT_STEP_7,
		LIGHT_STEP_8,
		NUM_LIGHTS,
	};


	dsp::SchmittTrigger edgeDetector;
	int stepNr = 0;

	ASequencer() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < ASequencer::NUM_LIGHTS; i++) {
			configParam(PARAM_STEP_1+i, 0.0, 5.0, 1.0);
		}

	}

	void process(const ProcessArgs &args) override;

};

void ASequencer::process(const ProcessArgs &args) {

	if (edgeDetector.process(inputs[MAIN_IN].getVoltage())) {
		stepNr = (stepNr + 1) & 7; // avoids modulus operator
	}

	for (int l = 0; l < NUM_LIGHTS; l++) {
		lights[l].setSmoothBrightness(l == stepNr, 5e-6f);
	}

	outputs[MAIN_OUT].setVoltage(params[stepNr].getValue());
}

struct AStepDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	NVGcolor txtCol;
	ASequencer * module;
	const int fh = 20; // font height


	AStepDisplay(Vec pos) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		setColor(0x00, 0x00, 0x00, 0xFF);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	AStepDisplay(Vec pos, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		box.pos = pos;
		box.size.y = fh;
		box.size.x = fh;
		setColor(r, g, b, a);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	}

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		txtCol.r = r;
		txtCol.g = g;
		txtCol.b = b;
		txtCol.a = a;
	}


	void draw(const DrawArgs &args) override {
		char tbuf[2];

		if (module == NULL) return;
		snprintf(tbuf, sizeof(tbuf), "%d", module->stepNr+1);

		TransparentWidget::draw(args);
		drawBackground(args);
		drawValue(args, tbuf);

	}

	void drawBackground(const DrawArgs &args) {
		Vec c = Vec(box.size.x/2, box.size.y);
		int whalf = box.size.x/2;
		int hfh = floor(fh / 2);

		// Draw rounded rectangle
		nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xF0));
		{
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, c.x -whalf, c.y +2);
			nvgLineTo(args.vg, c.x +whalf, c.y +2);
			nvgQuadTo(args.vg, c.x +whalf +5, c.y +2+hfh, c.x +whalf, c.y+fh+2);
			nvgLineTo(args.vg, c.x -whalf, c.y+fh+2);
			nvgQuadTo(args.vg, c.x -whalf -5, c.y +2+hfh, c.x -whalf, c.y +2);
			nvgClosePath(args.vg);
		}
		nvgFill(args.vg);
		nvgStrokeColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0x0F));
		nvgStrokeWidth(args.vg, 1.f);
		nvgStroke(args.vg);
	}

	void drawValue(const DrawArgs &args, const char * txt) {
		Vec c = Vec(box.size.x/2, box.size.y);

		nvgFontSize(args.vg, fh);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgFillColor(args.vg, nvgRGBA(txtCol.r, txtCol.g, txtCol.b, txtCol.a));
		nvgText(args.vg, c.x, c.y+fh-1, txt, NULL);
	}
};

struct ASequencerWidget : ModuleWidget {
	ASequencerWidget(ASequencer * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(9*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("ASequencer");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(4, 15));
			title->setText("CLK");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(45, 15));
			title->setText("STEP");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(95, 15));
			title->setText("OUT");
			addChild(title);
		}



		addInput(createInput<PJ301MPort>(Vec(6, 50), module, ASequencer::MAIN_IN));

		addOutput(createOutput<PJ3410Port>(Vec(91, 46), module, ASequencer::MAIN_OUT));

		for (int i = 0; i < ASequencer::NUM_LIGHTS; i++) {
			addChild(createLight<SmallLight<GreenLight>>(Vec(60, 95+(i*35)), module, ASequencer::LIGHT_STEP_1+i));
			addParam(createParam<RoundBlackKnob>(Vec(10, 85+(i*35)), module, ASequencer::PARAM_STEP_1+i));
		}

		{
			AStepDisplay * sd = new AStepDisplay(Vec(51,30));
			sd->module = module;
			addChild(sd);
		}

	}
};



Model *modelASequencer = createModel<ASequencer, ASequencerWidget>("ASequencer");
