#include <rack.hpp>
#include "acModules.hpp"

template <class TModule>
struct Pul5esDisplay : rack::TransparentWidget {
	TModule* module;
	rack::Vec displaySize;
	float pulseCount = 0.f;
	std::string fontPath = rack::asset::system("res/fonts/ShareTechMono-Regular.ttf");
	//std::string fontPath = rack::asset::system("res/fonts/DSEG7ClassicMini-Regular.ttf");


	void drawLayer(const DrawArgs& args, int layer) override {

		if (layer == 1 && module) {

			if (module->curSampleRate == 0.f) return;

			rack::Vec p;

			pulseCount = module->pulseOnCount;

			nvgScissor(args.vg, RECT_ARGS(args.clipBox));

			std::shared_ptr<rack::Font> font = APP->window->loadFont(rack::asset::plugin(pluginInstance, fontPath));
			if (font) {
				nvgFontSize(args.vg, 16);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

				std::string stepsStr = std::to_string(pulseCount);
				stepsStr = stepsStr.substr(0, stepsStr.find("."));
				nvgFillColor(args.vg, nvgRGB(0xd0,0xd0,0xd0));
				nvgText(args.vg, rack::mm2px(6.f),rack::mm2px(4.0f), stepsStr.c_str(), NULL);

			}

		}


		nvgResetScissor(args.vg);
		Widget::drawLayer(args, layer);
	}


};
