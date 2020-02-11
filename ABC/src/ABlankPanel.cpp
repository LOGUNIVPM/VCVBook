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

struct ABlank : Module {

	ABlank() {

	}



};


struct ABlankPanelWidget : ModuleWidget {
	ABlankPanelWidget(engine::Module *module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);


		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("ABC PLUGINS");
			addChild(title);
		}


	}

};



Model * modelABlankPanel = createModel<ABlank, ABlankPanelWidget>("ABlankPanel");
