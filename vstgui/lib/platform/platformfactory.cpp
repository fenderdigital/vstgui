// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "platformfactory.h"

#include "../viewrendering/viewrenderfactory.h"

#if MAC
#include "mac/macfactory.h"
using VSTGUIPlatformFactory = VSTGUI::MacFactory;
#elif WINDOWS
#include "win32/win32factory.h"
using VSTGUIPlatformFactory = VSTGUI::Win32Factory;
#elif LINUX
#include "linux/linuxfactory.h"
using VSTGUIPlatformFactory = VSTGUI::LinuxFactory;
#endif

//-----------------------------------------------------------------------------
namespace VSTGUI {

static PlatformFactoryPtr gPlatformFactory;

//-----------------------------------------------------------------------------
void setPlatformFactory (PlatformFactoryPtr&& f)
{
	gPlatformFactory = std::move (f);
}

//-----------------------------------------------------------------------------
void initPlatform (PlatformInstanceHandle instance)
{
	vstgui_assert (!gPlatformFactory);
	std::unique_ptr<IPlatformFactory> platformFactory (new VSTGUIPlatformFactory(instance));
	setPlatformFactory (std::unique_ptr<IPlatformFactory> (new Presonus::ViewRenderFactory (std::move (platformFactory))));
}

//-----------------------------------------------------------------------------
void exitPlatform ()
{
	vstgui_assert (gPlatformFactory);
	setPlatformFactory (nullptr);
}

//-----------------------------------------------------------------------------
const IPlatformFactory& getPlatformFactory ()
{
	vstgui_assert (gPlatformFactory);
	return *gPlatformFactory.get ();
}

//-----------------------------------------------------------------------------
} // VSTGUI
