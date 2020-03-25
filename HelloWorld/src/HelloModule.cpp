/*-------------------------- HelloWorld ----------------------------*
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

/* NOTE: uncomment the define below only if you are reading Chapter 10.1.5,
   it activates the print functions described in Chapter 10.1.5, writing
   to a log file in your working directory */
/* #define CHAPTER_10_1_5 */


#include "HelloWorld.hpp"

/* MODULE */
struct HelloModule : Module {
	enum ParamIds {
		NUM_PARAMS,
	};

	enum InputIds {
		NUM_INPUTS,
	};

	enum OutputIds {
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

#ifdef CHAPTER_10_1_5
	FILE *dbgFile;
	const char * moduleName = "HelloModule";
#endif

	HelloModule() {

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
#ifdef CHAPTER_10_1_5
		std::string dbgFilename = asset::user("dbgHello.log");
		dbgFile = fopen(dbgFilename.c_str(), "w");
		dbgPrint("HelloModule constructor\n");
#endif
	}

#ifdef CHAPTER_10_1_5
	~HelloModule() {
		dbgPrint("HelloModule destructor\n");
		fclose(dbgFile);
	}
#endif

	void process(const ProcessArgs &args) override {
		;
	}

#ifdef CHAPTER_10_1_5
	void dbgPrint(const char *format, ...) {
#ifdef DEBUG
		va_list args;
		va_start(args, format);
		vfprintf(dbgFile, format, args);
		fflush(dbgFile);
		va_end(args);
#endif
	}
#endif

};

/* MODULE WIDGET */
struct HelloModuleWidget : ModuleWidget {
	HelloModuleWidget(HelloModule* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HelloModule.svg")));
	}

};


Model * modelHello = createModel<HelloModule, HelloModuleWidget>("HelloWorld");

