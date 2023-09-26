// This file is a contribution to VSTGUI.
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.

#include "viewrenderframe.h"
#include "viewrenderdefs.h"

#include "pluginterfaces/gui/iplugview.h"

#if SMTG_OS_LINUX
#include "vstgui4/vstgui/lib/platform/linux/x11platform.h"
#endif

using namespace Presonus;
using namespace VSTGUI;

//************************************************************************************************
// ViewRenderFrame
//************************************************************************************************

ViewRenderFrame::ViewRenderFrame (IPlatformFrameCallback* frame, const CRect& size, IPlugRenderingFrame* parent, VSTGUI::PlatformType parentType, IPlatformFrameConfig* config)
: IPlatformFrame (frame),
  plugRenderingFrame (parent)
{
	#if SMTG_OS_LINUX
	auto cfg = dynamic_cast<X11::FrameConfig*> (config);
	if(cfg && cfg->runLoop)
	{
		X11::RunLoop::init (cfg->runLoop);
	}
	#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////

ViewRenderFrame::~ViewRenderFrame ()
{
	#if SMTG_OS_LINUX
	X11::RunLoop::exit ();
	#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::getGlobalPosition (CPoint& pos) const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::setSize (const CRect& newSize)
{
	size = newSize;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::getSize (CRect& result) const
{
	result = size;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::getCurrentMousePosition (CPoint& mousePosition) const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::getCurrentMouseButtons (CButtonState& buttons) const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::getCurrentModifiers (Modifiers& modifiers) const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::setMouseCursor (CCursorType type)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::invalidRect (const CRect& rect)
{
	if(plugRenderingFrame)
	{
		double contentScaleFactor = 1.;
		CFrame* cFrame = dynamic_cast<CFrame*> (frame);
		if(cFrame)
			contentScaleFactor = cFrame->getScaleFactor ();
		Steinberg::ViewRect dirtyRect (rect.left / contentScaleFactor, rect.top / contentScaleFactor, rect.right / contentScaleFactor, rect.bottom / contentScaleFactor);
		plugRenderingFrame->invalidateViewRect (&dirtyRect);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::scrollRect (const CRect& src, const CPoint& distance)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::showTooltip (const CRect& rect, const char* utf8Text)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::hideTooltip ()
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void* ViewRenderFrame::getPlatformRepresentation () const
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SharedPointer<IPlatformTextEdit> ViewRenderFrame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SharedPointer<IPlatformOptionMenu> ViewRenderFrame::createPlatformOptionMenu ()
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SharedPointer<IPlatformOpenGLView> ViewRenderFrame::createPlatformOpenGLView ()
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SharedPointer<IPlatformViewLayer> ViewRenderFrame::createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer)
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

DragResult ViewRenderFrame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	return kDragRefused;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

VSTGUI::PlatformType ViewRenderFrame::getPlatformType () const
{
	return VSTGUI::PlatformType (PlatformType::kViewRendering);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ViewRenderFrame::onFrameClosed ()
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

Optional<UTF8String> ViewRenderFrame::convertCurrentKeyEventToText ()
{
	return {};
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewRenderFrame::setupGenericOptionMenu (bool use, GenericOptionMenuTheme* theme)
{
	return false;
}
