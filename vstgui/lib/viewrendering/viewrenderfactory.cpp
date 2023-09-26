// This file is a contribution to VSTGUI.
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.

#include "viewrenderfactory.h"
#include "viewrenderframe.h"
#include "viewrenderdefs.h"

using namespace Presonus;
using namespace VSTGUI;

//************************************************************************************************
// ViewRenderFactory
//************************************************************************************************

ViewRenderFactory::ViewRenderFactory (PlatformFactoryPtr&& platformFactory)
: platformFactory (std::move (platformFactory))
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

PlatformFramePtr ViewRenderFactory::createFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent, VSTGUI::PlatformType parentType,
	IPlatformFrameConfig* config) const noexcept
{
	if(parentType == VSTGUI::PlatformType (PlatformType::kViewRendering))
	{
		return makeOwned<ViewRenderFrame> (frame, size, static_cast<IPlugRenderingFrame*> (parent), parentType, config);
	}
	return platformFactory->createFrame (frame, size, parent, parentType, config);
}
