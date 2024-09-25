#include "ABC.hpp"
#include "RCFilter.hpp"

#define MIN_PITCH (20.f)
#define MAX_FS (192000.f)
#define DLY_LEN (unsigned int)(1.f/MIN_PITCH * MAX_FS)

struct AKarplus : Module {
	enum ParamIds {
		PARAM_DELAY,
		PARAM_FC,
		PARAM_GAIN,
		PARAM_DISP,
		NUM_PARAMS,
	};

	enum InputIds {
		MAIN_IN,
		VOCT_IN,
		LPF_IN,
		GAIN_IN,
		NUM_INPUTS,
	};

	enum OutputIds {
		MAIN_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	float dly[DLY_LEN];
	RCFilter<float> filt;

	float intz1 = 0.f;
	unsigned int w_i, r_i;
	unsigned int delay_smp = 1000;
	float feedback = 0.f;
	float yn1, yn2, xn1, xn2;
	int k, n;
	#define N_APF (2) // order (not SOSs)
	float a[N_APF+1];
	float Dz1 = 4.f;

	AKarplus() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_DELAY,20e-6, 1.f/20.f, 1e-3, "Delay", "[s]");
		configParam(PARAM_FC, 20.f, 20000.f, 1000.f, "Cutoff", "[Hz]");
		configParam(PARAM_GAIN, 0.f, 1.f-1e-6, 1.f, "Gain", "");
		configParam(PARAM_DISP, 0.f, 16.f, 4.f, "Dispersion", "[smp]");

		configInput(MAIN_IN, "Input sound/pulse");
		configInput(VOCT_IN, "V/oct voltage input");
		configInput(LPF_IN, "Cutoff voltage input");
		configInput(GAIN_IN, "Feedback gain voltage in");

		configOutput(MAIN_OUT, "Karplus output");

		r_i = 0;
		setIndexDelay(delay_smp);
		memset(dly, 0, sizeof(dly));
		computeAPF(Dz1);
	}

	void computeAPF(float D) {

		// D < 1 --> make it 0
		if (D < 1.f) D = 0.f; // anything < 1 will produce NaN --> make it 0 and treat it like an instantaneous system

		// D integer within filter length
		if (D - floor(D) <= 1e-4) {
			D = floor(D);
			// when frac = 0 and is <= the number of states of the filter, the APF is a simple delay line (computing the APF coeffs as below will generate NaN)
			if (D <= N_APF) {
				memset(a, 0, sizeof(a));
				a[(int)D] = 1.f;
				return;
			}
		}

		// else: D real
		a[0] = 1.f; // useless
		for (k = 1; k <= N_APF; k++) {

			a[k] = (k==1? -1 : 1) * (factorial(N_APF) / (factorial(k)*factorial(N_APF-k)));
		    for (n = 0; n <= N_APF; n++) {
		        a[k] = a[k] * (float)(D-N_APF+n)/(float)(D-N_APF+k+n);
		    }
		}
	}

	void setIndexDelay(unsigned int delay_smp) {
		w_i = r_i+delay_smp;
		if (w_i > DLY_LEN) {
			w_i = w_i - DLY_LEN;
		}
	}

	void process(const ProcessArgs &args) override;

	void onSampleRateChange() override {
		// TODO: EXERCISE FOR STUDENTS
	}

};

void AKarplus::process(const ProcessArgs &args) {

	// READ PARAMS
	float fc = params[PARAM_FC].getValue();

	float delay_s = params[PARAM_DELAY].getValue();
	float gain = params[PARAM_GAIN].getValue();


	// READ INPUTS
	float in = inputs[MAIN_IN].getVoltage();
	if (inputs[VOCT_IN].isConnected()) {
		float Voct = inputs[VOCT_IN].getVoltage();
		delay_s *= powf(2, -Voct); // since delay in inversely proportional to pitch must invert
	}
	if (inputs[LPF_IN].isConnected())
		fc *= inputs[LPF_IN].getVoltage() / 10.f;
	if (inputs[GAIN_IN].isConnected())
		gain *= inputs[GAIN_IN].getVoltage() / 10.f;

	filt.setCutoff(2*fc);
	delay_smp = floor(delay_s * args.sampleRate);
	float frac = delay_s * args.sampleRate - delay_smp;
	setIndexDelay(delay_smp);


	// DELAY LINE
	dly[w_i] = in + feedback;
	float dly_out = dly[r_i];
	w_i++;
	if (w_i > DLY_LEN) w_i = w_i - DLY_LEN;
	r_i++;
	if (r_i > DLY_LEN) r_i = r_i - DLY_LEN;


	// INTERPOLATE
	float out_int = dly_out * frac + intz1 * (1.f - frac);
	intz1 = dly_out;

	// FILTER + GAIN
	float out = gain * filt.process(out_int);

	// DISPERSION
	float D = params[PARAM_DISP].getValue();
	if (D != Dz1) {
		computeAPF(D);
		Dz1 = D;
	}

	float x = out;
	float y = a[2] * x + a[1] * xn1 + xn2 - a[1] * yn1 - a[2] * yn2;
	yn2 = yn1;
	yn1 = y;
	xn2 = xn1;
	xn1 = x;

	out = y;

	feedback = out;

	// WRITE OUTPUT
	outputs[MAIN_OUT].setVoltage(out);

}

struct AKarplusWidget : ModuleWidget {

	AKarplusWidget(AKarplus* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(8*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("AKarplus");
			addChild(title);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(20-2, 60-35));
			lbl->setText("DELAY/PITCH");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(20+2, 120-35));
			lbl->setText("FBK CUTOFF");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(20+4, 180-35));
			lbl->setText("LOOP GAIN");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(20+1, 240-35));
			lbl->setText("DISPERSION");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(20, 310-35));
			lbl->setText("IN");
			addChild(lbl);
		}

		{
			ATextLabel * lbl = new ATextLabel(Vec(80, 310-35));
			lbl->setText("OUT");
			addChild(lbl);
		}

		addParam(createParam<RoundBlackKnob>(Vec(20, 60), module, AKarplus::PARAM_DELAY));
		addParam(createParam<RoundBlackKnob>(Vec(20, 120), module, AKarplus::PARAM_FC));
		addParam(createParam<RoundBlackKnob>(Vec(20, 180), module, AKarplus::PARAM_GAIN));
		addParam(createParam<RoundSmallBlackKnob>(Vec(20, 240), module, AKarplus::PARAM_DISP));

		addInput(createInput<PJ301MPort>(Vec(70, 60), module, AKarplus::VOCT_IN));
		addInput(createInput<PJ301MPort>(Vec(70, 120), module, AKarplus::LPF_IN));
		addInput(createInput<PJ301MPort>(Vec(70, 180), module, AKarplus::GAIN_IN));


		addInput(createInput<PJ301MPort>(Vec(20, 310), module, AKarplus::MAIN_IN));
		addOutput(createOutput<PJ301MPort>(Vec(80, 310), module, AKarplus::MAIN_OUT));

	}

};



Model * modelAKarplus = createModel<AKarplus, AKarplusWidget>("AKarplus");

