// This file is a contribution to VSTGUI.
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.

#ifndef _vst3viewrendereditor_h
#define _vst3viewrendereditor_h

#include "vstgui/plugin-bindings/vst3editor.h"

#include "vstgui/lib/platform/platformfactory.h"

#include "presonus/ipslviewrendering.h"

namespace Presonus {

//************************************************************************************************
// VST3ViewRenderEditor
//************************************************************************************************

class VST3ViewRenderEditor: public VSTGUI::VST3Editor,
							public IPlugViewCoordinateUnitSupport,
							public IPlugViewRendering,
							public IPlugViewMouseInput
{
public:
	using VST3Editor::VST3Editor;

	// VST3Editor
	Steinberg::tresult PLUGIN_API isPlatformTypeSupported (Steinberg::FIDString type) override;
	Steinberg::tresult PLUGIN_API onFocus (Steinberg::TBool state) override;
	bool requestResize (const VSTGUI::CPoint& newSize) override;

	// IPlugViewCoordinateUnitSupport
	Steinberg::int32 PLUGIN_API getCoordinateUnit () override;

	// IPlugViewRendering
	Steinberg::tresult PLUGIN_API isRenderingTypeSupported (Steinberg::TUID type, Steinberg::int32 pixelFormat) override;
	Steinberg::tresult PLUGIN_API render (Steinberg::FUnknown* target, Steinberg::ViewRect* updateRect) override;

	// IPlugViewMouseInput
	Steinberg::tresult PLUGIN_API onMouseEvent (PlugViewMouseEvent* mouseEvent) override;

	DELEGATE_REFCOUNT (VST3Editor)
	Steinberg::tresult PLUGIN_API queryInterface (const Steinberg::TUID iid, void** obj) override;

protected:
	VSTGUI::IPlatformFactory::COffscreenContextPtr offscreenContext;

	// VST3Editor
	Steinberg::tresult PLUGIN_API attached (void* parent, Steinberg::FIDString type) override;
};

} // namespace Presonus

#endif // _vst3viewrendereditor_h
