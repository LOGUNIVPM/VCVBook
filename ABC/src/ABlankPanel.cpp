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
