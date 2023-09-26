// This file is a contribution to VSTGUI.
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.

#ifndef _viewrenderframe_h
#define _viewrenderframe_h

#include "vstgui4/vstgui/lib/platform/iplatformframe.h"
#include "vstgui4/vstgui/lib/platform/iplatformtextedit.h"
#include "vstgui4/vstgui/lib/platform/iplatformoptionmenu.h"
#include "vstgui4/vstgui/lib/platform/iplatformviewlayer.h"
#include "vstgui4/vstgui/lib/platform/iplatformopenglview.h"

#include "vstgui4/vstgui/lib/cframe.h"
#include "vstgui4/vstgui/lib/crect.h"

#include "presonus/ipslviewrendering.h"

namespace Presonus {

//************************************************************************************************
// ViewRenderFrame
//************************************************************************************************

class ViewRenderFrame: public VSTGUI::IPlatformFrame
{
public:
	ViewRenderFrame (VSTGUI::IPlatformFrameCallback* frame, const VSTGUI::CRect& size, IPlugRenderingFrame* parent, VSTGUI::PlatformType parentType, VSTGUI::IPlatformFrameConfig* config);
	~ViewRenderFrame ();

	// IPlatformFrame
	bool getGlobalPosition (VSTGUI::CPoint& pos) const override;
	bool setSize (const VSTGUI::CRect& newSize) override;
	bool getSize (VSTGUI::CRect& size) const override;
	bool getCurrentMousePosition (VSTGUI::CPoint& mousePosition) const override;
	bool getCurrentMouseButtons (VSTGUI::CButtonState& buttons) const override;
	bool getCurrentModifiers (VSTGUI::Modifiers& modifiers) const override;
	bool setMouseCursor (VSTGUI::CCursorType type) override;
	bool invalidRect (const VSTGUI::CRect& rect) override;
	bool scrollRect (const VSTGUI::CRect& src, const VSTGUI::CPoint& distance) override;
	bool showTooltip (const VSTGUI::CRect& rect, const char* utf8Text) override;
	bool hideTooltip () override;
	void* getPlatformRepresentation () const override;
	VSTGUI::SharedPointer<VSTGUI::IPlatformTextEdit> createPlatformTextEdit (VSTGUI::IPlatformTextEditCallback* textEdit) override;
	VSTGUI::SharedPointer<VSTGUI::IPlatformOptionMenu> createPlatformOptionMenu () override;
	#if VSTGUI_OPENGL_SUPPORT
	VSTGUI::SharedPointer<VSTGUI::IPlatformOpenGLView> createPlatformOpenGLView () override;
	#endif
	VSTGUI::SharedPointer<VSTGUI::IPlatformViewLayer> createPlatformViewLayer (VSTGUI::IPlatformViewLayerDelegate* drawDelegate, VSTGUI::IPlatformViewLayer* parentLayer = nullptr) override;
	#if VSTGUI_ENABLE_DEPRECATED_METHODS
	VSTGUI::DragResult doDrag (VSTGUI::IDataPackage* source, const VSTGUI::CPoint& offset, VSTGUI::CBitmap* dragBitmap) override;
	#endif
	bool doDrag (const VSTGUI::DragDescription& dragDescription, const VSTGUI::SharedPointer<VSTGUI::IDragCallback>& callback) override;
	VSTGUI::PlatformType getPlatformType () const override;
	void onFrameClosed () override;
	VSTGUI::Optional<VSTGUI::UTF8String> convertCurrentKeyEventToText () override;
	bool setupGenericOptionMenu (bool use, VSTGUI::GenericOptionMenuTheme* theme = nullptr) override;

private:
	IPlugRenderingFrame* plugRenderingFrame;
	VSTGUI::CRect size;
};

} // namespace Presonus

#endif // _viewrenderfactory_h
