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


Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

	p->addModel(modelAComparator);
	p->addModel(modelAMuxDemux);
	p->addModel(modelAClock);
	p->addModel(modelASequencer);
	p->addModel(modelADivider);
	p->addModel(modelARandom);

	p->addModel(modelALinADSR);
	p->addModel(modelAExpADSR);
	p->addModel(modelAEnvFollower);
	p->addModel(modelASVFilter);
	p->addModel(modelAPolySVFilter);

	p->addModel(modelAModal);
	p->addModel(modelAModalGUI);
	p->addModel(modelATrivialOsc);
	p->addModel(modelADPWOsc);
	p->addModel(modelAPolyDPWOsc);
	p->addModel(modelAWavefolder);

	p->addModel(modelABlankPanel);
	p->addModel(modelAPolyXpander);

}
