// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Originally written and contributed to VSTGUI by PreSonus Software Ltd.

#pragma once

#include "waylandplatform.h"

struct wl_subsurface;
struct wl_buffer;
struct wl_shm_pool;

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Wayland {

//------------------------------------------------------------------------
class ChildWindow
{
public:
	ChildWindow (IWaylandFrame* waylandFrame, CPoint size);
	~ChildWindow () noexcept;

	void setFrame (IPlatformFrameCallback* frame);
	IPlatformFrameCallback* getFrame () const;

	void setSize (const CRect& rect);
	const CPoint& getSize () const;

	void* getBuffer () const;
	int getBufferStride () const;

	wl_surface* getSurface () const;

	void commit (const CRect& rect);

private:
	SharedPointer<IWaylandFrame> waylandFrame;
	IPlatformFrameCallback* frameCallback;
	bool initialized;
	CPoint size;
	void* data;

	wl_surface* surface;
	wl_subsurface* subSurface;
	wl_buffer* buffer;
	wl_shm_pool* pool;
	int allocatedSize;
	int byteSize;
	int fd;

	void initialize ();
	void terminate ();
	void createBuffer ();
	void destroyBuffer ();
};

//------------------------------------------------------------------------

} // Wayland
} // VSTGUI
