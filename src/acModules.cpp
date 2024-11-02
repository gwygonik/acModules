#include "acModules.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;
	p->addModel(modelPick6);
	p->addModel(modelPick6p);
	p->addModel(modelOv3rCross);
	p->addModel(modelChord4Roy);
	p->addModel(modelRhythm1101);
	p->addModel(modelMerc8or);
	p->addModel(modelPul5es);
}
