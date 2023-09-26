// This file is a contribution to VSTGUI.
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.

#include "vst3viewrendereditor.h"
#include "vstgui/lib/viewrendering/viewrenderdefs.h"

#include "pluginterfaces/base/keycodes.h"

using namespace Presonus;

//************************************************************************************************
// VST3ViewRenderEditor
//************************************************************************************************

Steinberg::tresult PLUGIN_API VST3ViewRenderEditor::queryInterface (const Steinberg::TUID iid, void** obj)
{
	QUERY_INTERFACE (iid, obj, IPlugViewCoordinateUnitSupport::iid, IPlugViewCoordinateUnitSupport)
	QUERY_INTERFACE (iid, obj, IPlugViewRendering::iid, IPlugViewRendering)
	QUERY_INTERFACE (iid, obj, IPlugViewMouseInput::iid, IPlugViewMouseInput)
	return VST3Editor::queryInterface (iid, obj);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Steinberg::int32 PLUGIN_API VST3ViewRenderEditor::getCoordinateUnit ()
{
	return kCoordinateUnitScalablePoints;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Steinberg::tresult PLUGIN_API VST3ViewRenderEditor::isPlatformTypeSupported (Steinberg::FIDString type)
{
	if(::strcmp (type, kPlatformTypePlugViewRendering) == 0)
		return Steinberg::kResultTrue;
	return VST3Editor::isPlatformTypeSupported (type);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Steinberg::tresult PLUGIN_API VST3ViewRenderEditor::isRenderingTypeSupported (Steinberg::TUID type, Steinberg::int32 pixelFormat)
{
	if(Steinberg::FUnknownPrivate::iidEqual (type, IBitmapAccessor::iid) && pixelFormat == kPixelFormatRGBA)
		return Steinberg::kResultOk;
	return Steinberg::kResultFalse;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Steinberg::tresult PLUGIN_API VST3ViewRenderEditor::render (Steinberg::FUnknown* target, Steinberg::ViewRect* updateRect)
{
	Steinberg::FUnknownPtr<IBitmapAccessor> bitmapAccessor (target);
	if(bitmapAccessor == nullptr || updateRect == nullptr)
		return Steinberg::kResultFalse;

	BitmapLockScope lockScope (bitmapAccessor);
	BitmapPixelBuffer& buffer = lockScope.data;
	VSTGUI::CPoint size (buffer.width, buffer.height);

	if(offscreenContext == nullptr || offscreenContext->getSurfaceRect ().getSize () != size)
		offscreenContext = VSTGUI::getPlatformFactory ().createOffscreenContext (size, 1);
	if(offscreenContext == nullptr)
		return Steinberg::kResultFalse;

	offscreenContext->beginDraw ();
	VSTGUI::CRect rect (updateRect->left * contentScaleFactor, updateRect->top * contentScaleFactor, updateRect->right * contentScaleFactor, updateRect->bottom * contentScaleFactor);
	offscreenContext->clearRect (rect);
	getFrame ()->drawRect (offscreenContext, rect);
	offscreenContext->endDraw ();

	VSTGUI::CBitmap* bitmap = offscreenContext->getBitmap ();
	if(bitmap == nullptr)
		return Steinberg::kResultFalse;

	VSTGUI::IPlatformBitmap* platformBitmap = bitmap->getPlatformBitmap ();
	if(platformBitmap == nullptr)
		return Steinberg::kResultFalse;

	VSTGUI::SharedPointer<VSTGUI::IPlatformBitmapPixelAccess> pixels = platformBitmap->lockPixels (true);
	for(int lineIndex = 0; lineIndex < buffer.height; lineIndex++)
		::memcpy (static_cast<char*> (buffer.scan0) + buffer.rowBytes * lineIndex, pixels->getAddress () + pixels->getBytesPerRow () * lineIndex, pixels->getBytesPerRow ());

	return Steinberg::kResultOk;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool VST3ViewRenderEditor::requestResize (const VSTGUI::CPoint& newSize)
{
	VSTGUI::CPoint size (newSize);
	size /= contentScaleFactor;
	return VST3Editor::requestResize (size);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Steinberg::tresult PLUGIN_API VST3ViewRenderEditor::onFocus (Steinberg::TBool state)
{
	if(VSTGUI::CFrame* frame = getFrame ())
		frame->onActivate (state);
	return Steinberg::kResultOk;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Steinberg::tresult PLUGIN_API VST3ViewRenderEditor::onMouseEvent (PlugViewMouseEvent* mouseEvent)
{
	VSTGUI::CPoint where (mouseEvent->x, mouseEvent->y);
	where *= contentScaleFactor;

	VSTGUI::MouseButton button = VSTGUI::MouseButton::Left;
	switch(mouseEvent->button)
	{
	case PlugViewMouseEvent::kLeftButton :
		button = VSTGUI::MouseButton::Left;
		break;
	case PlugViewMouseEvent::kMiddleButton:
		button = VSTGUI::MouseButton::Middle;
		break;
	case PlugViewMouseEvent::kRightButton:
		button = VSTGUI::MouseButton::Right;
		break;
	}

	VSTGUI::Modifiers modifiers;
	if(mouseEvent->modifiers & Steinberg::kShiftKey)
		modifiers.add (VSTGUI::ModifierKey::Shift);
	if(mouseEvent->modifiers & Steinberg::kAlternateKey)
		modifiers.add (VSTGUI::ModifierKey::Alt);
	if(mouseEvent->modifiers & Steinberg::kCommandKey)
		modifiers.add (VSTGUI::ModifierKey::Control);
	if(mouseEvent->modifiers & Steinberg::kControlKey)
		modifiers.add (VSTGUI::ModifierKey::Super);

	switch(mouseEvent->type)
	{
	case PlugViewMouseEvent::kMouseDown :
		{
			VSTGUI::MouseDownEvent e (where, button);
			getFrame ()->onMouseDownEvent (e);
		}
		return Steinberg::kResultOk;
	case PlugViewMouseEvent::kMouseUp :
		{
			VSTGUI::MouseUpEvent e (where, button);
			getFrame ()->onMouseUpEvent (e);
		}
		return Steinberg::kResultOk;
	case PlugViewMouseEvent::kMouseEnter :
		{
			VSTGUI::MouseEnterEvent e (where, button, modifiers);
			getFrame ()->onMouseEnterEvent (e);
		}
		return Steinberg::kResultOk;
	case PlugViewMouseEvent::kMouseMove :
		{
			VSTGUI::MouseMoveEvent e (where, button);
			getFrame ()->onMouseMoveEvent (e);
		}
		return Steinberg::kResultOk;
	case PlugViewMouseEvent::kMouseLeave :
		{
			VSTGUI::MouseExitEvent e (where, button, modifiers);
			getFrame ()->onMouseExitEvent (e);
		}
		return Steinberg::kResultOk;
	}
	
	return Steinberg::kResultFalse;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Steinberg::tresult PLUGIN_API VST3ViewRenderEditor::attached (void* parent, Steinberg::FIDString type)
{
	if(Steinberg::FIDStringsEqual (type, kPlatformTypePlugViewRendering))
	{
		Steinberg::FUnknownPtr<IPlugRenderingFrame> plugRenderingFrame (plugFrame);
		if(plugRenderingFrame && open (plugRenderingFrame, VSTGUI::PlatformType (Presonus::PlatformType::kViewRendering)))
		{
			Steinberg::ViewRect vr (0, 0, int32_t (frame->getWidth ()), int32_t (frame->getHeight ()));
			setRect (vr);
			if(plugFrame)
				plugFrame->resizeView (this, &vr);

			if(timer)
				timer->start ();

			return EditorView::attached (parent, type);
		}
	}
	return VST3Editor::attached (parent, type);
}
