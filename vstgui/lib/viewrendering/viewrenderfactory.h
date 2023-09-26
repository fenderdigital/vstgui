// This file is a contribution to VSTGUI.
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.

#ifndef _viewrenderfactory_h
#define _viewrenderfactory_h

#include "vstgui4/vstgui/lib/platform/platformfactory.h"

#include "vstgui4/vstgui/lib/platform/iplatformresourceinputstream.h"
#include "vstgui4/vstgui/lib/platform/iplatformgradient.h"
#include "vstgui4/vstgui/lib/platform/iplatformfont.h"
#include "vstgui4/vstgui/lib/platform/iplatformframe.h"
#include "vstgui4/vstgui/lib/platform/iplatformbitmap.h"
#include "vstgui4/vstgui/lib/platform/iplatformstring.h"
#include "vstgui4/vstgui/lib/platform/iplatformtimer.h"

#include "vstgui4/vstgui/lib/coffscreencontext.h"

namespace Presonus {

//************************************************************************************************
// ViewRenderFactory
//************************************************************************************************

class ViewRenderFactory: public VSTGUI::IPlatformFactory
{
public:
	ViewRenderFactory (VSTGUI::PlatformFactoryPtr&& platformFactory);

	// IPlatformFactory
	VSTGUI::PlatformFramePtr createFrame (VSTGUI::IPlatformFrameCallback* frame, const VSTGUI::CRect& size, void* parent, VSTGUI::PlatformType parentType,
		VSTGUI::IPlatformFrameConfig* config = nullptr) const noexcept override;

	uint64_t getTicks () const noexcept override { return platformFactory->getTicks (); }
	VSTGUI::PlatformFontPtr createFont (const VSTGUI::UTF8String& name, const VSTGUI::CCoord& size, const int32_t& style) const noexcept override { return platformFactory->createFont (name, size, style); }
	bool getAllFontFamilies (const VSTGUI::FontFamilyCallback& callback) const noexcept override { return platformFactory->getAllFontFamilies (callback); }
	VSTGUI::PlatformBitmapPtr createBitmap (const VSTGUI::CPoint& size) const noexcept override { return platformFactory->createBitmap (size); }
	VSTGUI::PlatformBitmapPtr createBitmap (const VSTGUI::CResourceDescription& desc) const noexcept override { return platformFactory->createBitmap (desc); }
	VSTGUI::PlatformBitmapPtr createBitmapFromPath (VSTGUI::UTF8StringPtr absolutePath) const noexcept override { return platformFactory->createBitmapFromPath (absolutePath); }
	VSTGUI::PlatformBitmapPtr createBitmapFromMemory (const void* ptr, uint32_t memSize) const noexcept override { return platformFactory->createBitmapFromMemory (ptr, memSize); }
	VSTGUI::PNGBitmapBuffer createBitmapMemoryPNGRepresentation (const VSTGUI::PlatformBitmapPtr& bitmap) const noexcept override { return platformFactory->createBitmapMemoryPNGRepresentation (bitmap); }
	VSTGUI::PlatformResourceInputStreamPtr createResourceInputStream (const VSTGUI::CResourceDescription& desc) const noexcept override { return platformFactory->createResourceInputStream (desc); }
	VSTGUI::PlatformStringPtr createString (VSTGUI::UTF8StringPtr utf8String = nullptr) const noexcept override { return platformFactory->createString (utf8String); }
	VSTGUI::PlatformTimerPtr createTimer (VSTGUI::IPlatformTimerCallback* callback) const noexcept override { return platformFactory->createTimer (callback); }
	bool setClipboard (const DataPackagePtr& data) const noexcept override { return platformFactory->setClipboard (data); }
	DataPackagePtr getClipboard () const noexcept override { return platformFactory->getClipboard (); }
	COffscreenContextPtr createOffscreenContext (const VSTGUI::CPoint& size, double scaleFactor = 1.) const noexcept override { return platformFactory->createOffscreenContext (size, scaleFactor); }
	VSTGUI::PlatformGradientPtr createGradient () const noexcept override { return platformFactory->createGradient (); }
	VSTGUI::PlatformFileSelectorPtr createFileSelector (VSTGUI::PlatformFileSelectorStyle style, VSTGUI::IPlatformFrame* frame) const noexcept override { return platformFactory->createFileSelector (style, frame); }

	const VSTGUI::LinuxFactory* asLinuxFactory () const noexcept override { return platformFactory->asLinuxFactory (); }
	const VSTGUI::MacFactory* asMacFactory () const noexcept override { return platformFactory->asMacFactory (); }
	const VSTGUI::Win32Factory* asWin32Factory () const noexcept override { return platformFactory->asWin32Factory (); }

private:
	VSTGUI::PlatformFactoryPtr platformFactory;
};

} // namespace Presonus

#endif // _viewrenderfactory_h
