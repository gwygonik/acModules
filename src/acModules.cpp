#include "acModules.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;
	p->addModel(modelPick6);
	p->addModel(modelPick6p);
	p->addModel(modelOv3rCross);
	p->addModel(modelChord4Roy);
}
