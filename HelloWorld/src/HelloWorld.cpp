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


#include "HelloWorld.hpp"


Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;

	p->addModel(modelHello);

}
