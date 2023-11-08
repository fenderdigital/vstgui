// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Originally written and contributed to VSTGUI by PreSonus Software Ltd.

#include "waylandplatform.h"
#include "../../cfileselector.h"
#include "../../cframe.h"
#include "../../cstring.h"
#include "../../events.h"
#include "waylandframe.h"
//#include "x11dragging.h"
#include "cairobitmap.h"
#include <cassert>
#include <chrono>
#include <array>
#include <dlfcn.h>
#include <iostream>
#include <locale>
#include <link.h>
#include <unordered_map>
#include <codecvt>
#include <xkbcommon/xkbcommon.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
namespace Wayland {

//------------------------------------------------------------------------
namespace {

using VirtMap = std::unordered_map<xkb_keysym_t, VirtualKey>;
const VirtMap keyMap = {{XKB_KEY_BackSpace, VirtualKey::Back},
						{XKB_KEY_Tab, VirtualKey::Tab},
						{XKB_KEY_Clear, VirtualKey::Clear},
						{XKB_KEY_Return, VirtualKey::Return},
						{XKB_KEY_Pause, VirtualKey::Pause},
						{XKB_KEY_Escape, VirtualKey::Escape},
						{XKB_KEY_space, VirtualKey::Space},
						{XKB_KEY_End, VirtualKey::End},
						{XKB_KEY_Home, VirtualKey::Home},

						{XKB_KEY_Left, VirtualKey::Left},
						{XKB_KEY_Up, VirtualKey::Up},
						{XKB_KEY_Right, VirtualKey::Right},
						{XKB_KEY_Down, VirtualKey::Down},
						{XKB_KEY_Page_Up, VirtualKey::PageUp},
						{XKB_KEY_Page_Down, VirtualKey::PageDown},

						{XKB_KEY_Select, VirtualKey::Select},
						{XKB_KEY_Print, VirtualKey::Print},
						{XKB_KEY_KP_Enter, VirtualKey::Enter},
						{XKB_KEY_Insert, VirtualKey::Insert},
						{XKB_KEY_Delete, VirtualKey::Delete},
						{XKB_KEY_Help, VirtualKey::Help},
						// Numpads ???
						{XKB_KEY_KP_Multiply, VirtualKey::Multiply},
						{XKB_KEY_KP_Add, VirtualKey::Add},
						{XKB_KEY_KP_Separator, VirtualKey::Separator},
						{XKB_KEY_KP_Subtract, VirtualKey::Subtract},
						{XKB_KEY_KP_Decimal, VirtualKey::Decimal},
						{XKB_KEY_KP_Divide, VirtualKey::Divide},
						{XKB_KEY_F1, VirtualKey::F1},
						{XKB_KEY_F2, VirtualKey::F2},
						{XKB_KEY_F3, VirtualKey::F3},
						{XKB_KEY_F4, VirtualKey::F4},
						{XKB_KEY_F5, VirtualKey::F5},
						{XKB_KEY_F6, VirtualKey::F6},
						{XKB_KEY_F7, VirtualKey::F7},
						{XKB_KEY_F8, VirtualKey::F8},
						{XKB_KEY_F9, VirtualKey::F9},
						{XKB_KEY_F10, VirtualKey::F10},
						{XKB_KEY_F11, VirtualKey::F11},
						{XKB_KEY_F12, VirtualKey::F12},
						{XKB_KEY_Num_Lock, VirtualKey::NumLock},
						{XKB_KEY_Scroll_Lock, VirtualKey::Scroll}, // correct ?
#if 0
						{XKB_KEY_Shift_L, VirtualKey::SHIFT},
						{XKB_KEY_Shift_R, VirtualKey::SHIFT},
						{XKB_KEY_Control_L, VirtualKey::CONTROL},
						{XKB_KEY_Control_R, VirtualKey::CONTROL},
						{XKB_KEY_Alt_L, VirtualKey::ALT},
						{XKB_KEY_Alt_R, VirtualKey::ALT},
#endif
						{XKB_KEY_VoidSymbol, VirtualKey::None}};
const VirtMap shiftKeyMap = {{XKB_KEY_KP_Page_Up, VirtualKey::PageUp},
							 {XKB_KEY_KP_Page_Down, VirtualKey::PageDown},
							 {XKB_KEY_KP_Home, VirtualKey::Home},
							 {XKB_KEY_KP_End, VirtualKey::End}};

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
struct RunLoop::Impl: wl_registry_listener, xdg_wm_base_listener, wl_seat_listener, IEventHandler
{
	SharedPointer<IRunLoop> runLoop;
	SharedPointer<IWaylandHost> waylandHost;
	std::atomic<uint32_t> useCount {0};
	cairo_device_t* device {nullptr};
	wl_display* display {nullptr};
	wl_registry* registry {nullptr};
	wl_compositor* compositor {nullptr};
	wl_subcompositor* subCompositor {nullptr};
	xdg_wm_base* windowManager {nullptr};
	wl_shm* sharedMemory {nullptr};
	wl_seat* seat {nullptr};
	uint32_t seatCapabilities {0};
	bool inDispatch {false};

	Impl ()
	{
		global = onGlobal;
		global_remove = onGlobalRemoved;
		ping = onPing;
		capabilities = onSeatCapabilities;
		name = onSeatName;
	}

	void init (const SharedPointer<IRunLoop>& inRunLoop, const SharedPointer<IWaylandHost>& inWaylandHost)
	{
		if (++useCount != 1)
			return;
		
		runLoop = inRunLoop;
		waylandHost = inWaylandHost;

		if(waylandHost == nullptr)
			return;

		display = waylandHost->openWaylandConnection ();
		if(display == nullptr)
			return;
		
		runLoop->registerEventHandler (wl_display_get_fd (display), this);

		registry = wl_display_get_registry (display);
		wl_registry_add_listener (registry, this, this);

		flush ();
	}

	void exit ()
	{
		if (--useCount != 0)
			return;

		cairo_device_finish (device);
		cairo_device_destroy (device);
		device = nullptr;

		runLoop->unregisterEventHandler (this);
		runLoop = nullptr;

		if(seat)
			wl_seat_destroy (seat);
		if(sharedMemory)
			wl_shm_destroy (sharedMemory);
		if(windowManager)
			xdg_wm_base_destroy (windowManager);
		if(subCompositor)
			wl_subcompositor_destroy (subCompositor);
		if(compositor)
			wl_compositor_destroy (compositor);
		if(registry)
			wl_registry_destroy (registry);
		
		flush ();

		if(waylandHost && display)
			waylandHost->closeWaylandConnection (display);
		waylandHost = nullptr;

		display = nullptr;
		registry = nullptr;
		compositor = nullptr;
		subCompositor = nullptr;
		windowManager = nullptr;
		sharedMemory = nullptr;
		seat = nullptr;
	}

	void flush ()
	{
		if(display && !inDispatch)
			wl_display_flush (display);
	}

	// wl_registry_listener
	static void onGlobal (void* data, wl_registry* registry, uint32_t name, const char* _interfaceName, uint32_t version)
	{
		Impl* This = static_cast<Impl*> (data);

		UTF8String interfaceName (_interfaceName);

		// Compositor
		if(interfaceName == wl_compositor_interface.name && This->compositor == nullptr)
			This->compositor = static_cast<wl_compositor*> (wl_registry_bind (registry, name, &wl_compositor_interface, min<uint32_t> (5, version)));

		// Subcompositor
		else if(interfaceName == wl_subcompositor_interface.name && This->subCompositor == nullptr)
			This->subCompositor = static_cast<wl_subcompositor*> (wl_registry_bind (registry, name, &wl_subcompositor_interface, min<uint32_t> (1, version)));

		// Window Manager
		else if(interfaceName == xdg_wm_base_interface.name && This->windowManager == nullptr)
		{
			This->windowManager = static_cast<xdg_wm_base*> (wl_registry_bind (registry, name, &xdg_wm_base_interface, min<uint32_t> (6, version)));
			if(This->windowManager != nullptr)
				xdg_wm_base_add_listener (This->windowManager, This, This);
		}
		
		// Shared Memory
		else if(interfaceName == wl_shm_interface.name && This->sharedMemory == nullptr)
			This->sharedMemory = static_cast<wl_shm*> (wl_registry_bind (registry, name, &wl_shm_interface, min<uint32_t> (1, version)));

		// Seat
		else if(interfaceName == wl_seat_interface.name && This->seat == nullptr)
		{
			This->seat = static_cast<wl_seat*> (wl_registry_bind (registry, name, &wl_seat_interface, min<uint32_t> (8, version)));
			if(This->seat != nullptr)
				wl_seat_add_listener (This->seat, This, This);
		}
	}

	static void onGlobalRemoved (void* data, wl_registry* registry, uint32_t name)
	{
		// TODO
	}

	// xdg_wm_base_listener
	static void onPing (void* data, xdg_wm_base* windowManager, uint32_t serial)
	{
		xdg_wm_base_pong (windowManager, serial);
	}

	// wl_seat_listener
	static void onSeatCapabilities (void* data, wl_seat* seat, uint32_t capabilities)
	{
		Impl* This = static_cast<Impl*> (data);
		if(seat == This->seat)
			This->seatCapabilities = capabilities;
	}

	static void onSeatName (void* data, wl_seat* seat, const char* name)
	{}

	// IEventHandler
	void onEvent () final
	{
		inDispatch = true;
		if(wl_display_prepare_read (display) == 0)
			wl_display_read_events (display);
		wl_display_dispatch_pending (display);
		inDispatch = false;
		flush ();
	}
};

//------------------------------------------------------------------------
RunLoop& RunLoop::instance ()
{
	static RunLoop gInstance;
	return gInstance;
}

//------------------------------------------------------------------------
void RunLoop::init (const SharedPointer<IRunLoop>& runLoop, const SharedPointer<IWaylandHost>& waylandHost)
{
	instance ().impl->init (runLoop, waylandHost);
}

//------------------------------------------------------------------------
void RunLoop::exit ()
{
	instance ().impl->exit ();
}

//------------------------------------------------------------------------
void RunLoop::flush ()
{
	instance ().impl->flush ();
}

//------------------------------------------------------------------------
const SharedPointer<IRunLoop> RunLoop::get ()
{
	return instance ().impl->runLoop;
}

//------------------------------------------------------------------------
wl_display* RunLoop::getDisplay ()
{
	return instance ().impl->display;
}

//------------------------------------------------------------------------
wl_compositor* RunLoop::getCompositor ()
{
	return instance ().impl->compositor;
}

//------------------------------------------------------------------------
wl_subcompositor* RunLoop::getSubCompositor ()
{
	return instance ().impl->subCompositor;
}

//------------------------------------------------------------------------
xdg_wm_base* RunLoop::getWindowManager ()
{
	return instance ().impl->windowManager;
}

//------------------------------------------------------------------------
wl_shm* RunLoop::getSharedMemory ()
{
	return instance ().impl->sharedMemory;
}

//------------------------------------------------------------------------
wl_seat* RunLoop::getSeat ()
{
	return (instance ().impl->seatCapabilities != 0) ? instance ().impl->seat : nullptr;
}

//------------------------------------------------------------------------
bool RunLoop::hasPointerInput ()
{
	return (instance ().impl->seatCapabilities & WL_SEAT_CAPABILITY_POINTER) != 0;
}

//------------------------------------------------------------------------
RunLoop::RunLoop ()
{
	impl = std::unique_ptr<Impl> (new Impl);
}

//------------------------------------------------------------------------
RunLoop::~RunLoop () noexcept = default;

//------------------------------------------------------------------------
void RunLoop::setDevice (cairo_device_t* device)
{
	if (impl->device != device)
	{
		cairo_device_destroy (impl->device);
		impl->device = cairo_device_reference (device);
	}
}

//------------------------------------------------------------------------
} // Wayland
} // VSTGUI
