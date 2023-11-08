// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Originally written and contributed to VSTGUI by PreSonus Software Ltd.

#include "waylandutils.h"
#include "../../cstring.h"

#include <wayland-client.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <linux/input.h>

namespace VSTGUI {
namespace Wayland {

class InputHandler: public wl_pointer_listener
{
public:
	static InputHandler& instance ()
	{
		static InputHandler inputHandler;
		return inputHandler;
	}

	void addWindow (ChildWindow* window);
	void removeWindow (ChildWindow* window);

	// wl_pointer_listener
	static void onPointerEnter (void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t x, wl_fixed_t y);
	static void onPointerLeave (void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface);
	static void onPointerMotion (void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y);
	static void onPointerButton (void* data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
	static void onPointerAxis (void* data, wl_pointer* wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
	static void onPointerAxisSource (void* data, wl_pointer* pointer, uint32_t axisSource);
	static void onPointerAxisStop (void* data, wl_pointer* pointer, uint32_t time, uint32_t axis);
	static void onPointerAxisDiscrete (void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete);
	static void onPointerAxis120 (void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete);
	static void onPointerFrame (void* data, wl_pointer* pointer);
	
private:
	struct PointerEvent
	{
		uint32_t eventMask {0};
		wl_fixed_t x {0};
		wl_fixed_t y {0};
		uint32_t button {0};
		uint32_t state {0};
		uint32_t time {0};
		uint32_t serial {0};
		struct Axis
		{
			wl_fixed_t value {0};
			int32_t discrete {0};
			bool valid {false};
		} axes[2];
		uint32_t axisSource {0};
		uint32_t buttonState {0};
		wl_surface* focus {nullptr};
		wl_surface* previousFocus {nullptr};
	};
	
	enum PointerEventMask
	{
		kPointerEnter = 1 << 0,
		kPointerLeave = 1 << 1,
		kPointerMotion = 1 << 2,
		kPointerButton = 1 << 3,
		kPointerAxis = 1 << 4,
		kPointerAxisSource = 1 << 5,
		kPointerAxisStop = 1 << 6,
		kPointerAxisDiscrete = 1 << 7,
	};

	wl_pointer* pointer;
	PointerEvent pointerEvent;
	bool initialized;
	std::vector<ChildWindow*> windows;

	InputHandler ();
	void initialize ();
	void terminate ();
	void dispatch (const PointerEvent& event);
};

} // namespace Wayland
} // namespace VSTGUI

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Wayland {

//------------------------------------------------------------------------
ChildWindow::ChildWindow (IWaylandFrame* waylandFrame, CPoint size)
: size (size),
  waylandFrame (shared (waylandFrame)),
  data (MAP_FAILED),
  buffer (nullptr),
  pool (nullptr),
  surface (nullptr),
  subSurface (nullptr),
  allocatedSize (0),
  byteSize (0),
  fd (-1)
{}

//------------------------------------------------------------------------
ChildWindow::~ChildWindow () noexcept
{
	destroyBuffer ();
	terminate ();
}

//------------------------------------------------------------------------
void ChildWindow::setFrame (IPlatformFrameCallback* frame)
{
	frameCallback = frame;
}

//------------------------------------------------------------------------
IPlatformFrameCallback* ChildWindow::getFrame () const
{
	return frameCallback;
}

//------------------------------------------------------------------------
void ChildWindow::commit (const CRect& rect)
{
	if(surface == nullptr)
		return;

	wl_surface_damage_buffer (surface, rect.left, rect.top, rect.getWidth (), rect.getHeight ());
	wl_surface_attach (surface, buffer, 0, 0);
	wl_surface_commit (surface);

	RunLoop::flush ();
}

//------------------------------------------------------------------------
void ChildWindow::setSize (const CRect& rect)
{
	if(size != rect.getSize ())
	{
		if(buffer != nullptr)
			wl_buffer_destroy (buffer);
		buffer = nullptr;
		data = MAP_FAILED;
	}
	size = rect.getSize ();
}

//------------------------------------------------------------------------
const CPoint& ChildWindow::getSize () const
{
	return size;
}

//------------------------------------------------------------------------
void* ChildWindow::getBuffer () const
{
	if(buffer == nullptr)
		const_cast<ChildWindow*> (this)->createBuffer ();
	return (data != MAP_FAILED) ? data : nullptr;
}

//------------------------------------------------------------------------
int ChildWindow::getBufferStride () const
{
	return cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, size.x);
}

//------------------------------------------------------------------------
wl_surface* ChildWindow::getSurface () const
{
	return surface;
}

//------------------------------------------------------------------------
void ChildWindow::createBuffer ()
{
	if(!initialized)
		initialize ();
	if(!initialized)
		return;

	wl_shm* shm = RunLoop::getSharedMemory ();
	if(shm == nullptr)
		return;
	
	int stride = getBufferStride ();
	int byteSize = stride * size.y;

	if(byteSize > allocatedSize)
	{
		int newSize = byteSize * 1.5;
		
		if(pool)
			wl_shm_pool_destroy (pool);
		pool = nullptr;
		
		if(fd >= 0)
			::close (fd);
		fd = -1;
		
		if(data != MAP_FAILED)
		{
			::munmap (data, allocatedSize);
			data = MAP_FAILED;
		}
		
		for(int i = 0; i < 100 && fd < 0; i++)
		{
			UTF8String name = "/vstgui_wl_buffer-" + std::to_string (::rand ());
			fd = ::shm_open (name, O_RDWR | O_CREAT | O_EXCL, 0600);
			if(fd >= 0)
			{
				::shm_unlink (name);
				break;
			}
		}
		
		int result = 0;
		do
		{
			result = ::ftruncate (fd, newSize);
		} while (result < 0 && errno == EINTR);
		
		if(result < 0)
		{
			::close (fd);
			return;
		}
		
		data = ::mmap (NULL, newSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if(data == MAP_FAILED)
		{
			::close (fd);
			return;
		}
		
		allocatedSize = newSize;
		
		pool = wl_shm_create_pool (shm, fd, allocatedSize);
	}
	
	buffer = wl_shm_pool_create_buffer (pool, 0, size.x, size.y, stride, WL_SHM_FORMAT_ARGB8888);
	if(surface && buffer)
		wl_surface_attach (surface, buffer, 0, 0);

	// TODO add a buffer listener, check if a buffer has been released before rendering into the same buffer again
	// use multiple buffers as a swapchain
}

//------------------------------------------------------------------------
void ChildWindow::destroyBuffer ()
{
	if(buffer)
		wl_buffer_destroy (buffer);
	buffer = nullptr;

	if(pool)
		wl_shm_pool_destroy (pool);
	pool = nullptr;

	if(data != MAP_FAILED)
		::munmap (data, byteSize);
	data = MAP_FAILED;
}

//------------------------------------------------------------------------
void ChildWindow::initialize ()
{
	wl_display* display = RunLoop::getDisplay ();
	wl_compositor* compositor = RunLoop::getCompositor ();
	wl_subcompositor* subCompositor = RunLoop::getSubCompositor ();
	wl_seat* seat = RunLoop::getSeat ();

	// FIXME: better wait for all required globals to be available before creating any ChildWindow's
	if(display == nullptr || compositor == nullptr || subCompositor == nullptr || seat == nullptr)
		return;

	wl_surface* parent = waylandFrame->getWaylandSurface (display);
	if(parent == nullptr)
		return;

	surface = wl_compositor_create_surface (compositor);
	if(surface == nullptr)
		return;

	subSurface = wl_subcompositor_get_subsurface (subCompositor, surface, parent);
	if(subSurface == nullptr)
	{
		wl_surface_destroy (surface);
		surface = nullptr;
		return;
	}

	wl_subsurface_set_desync (subSurface);

	InputHandler::instance ().addWindow (this);

	initialized = true;
}

//------------------------------------------------------------------------
void ChildWindow::terminate ()
{
	if(!initialized)
		return;

	InputHandler::instance ().removeWindow (this);

	if(subSurface)
		wl_subsurface_destroy (subSurface);
	subSurface = nullptr;

	if(surface)
		wl_surface_destroy (surface);
	surface = nullptr;

	RunLoop::flush ();

	initialized = false;
}

//------------------------------------------------------------------------
InputHandler::InputHandler ()
: pointer (nullptr),
  initialized (false)
{
	enter = onPointerEnter;
	leave = onPointerLeave;
	motion = onPointerMotion;
	button = onPointerButton;
	axis = onPointerAxis;
	axis_source = onPointerAxisSource;
	axis_stop = onPointerAxisStop;
	axis_discrete = onPointerAxisDiscrete;
#ifdef WL_POINTER_AXIS_VALUE120_SINCE_VERSION
	axis_value120 = onPointerAxis120;
#endif
	frame = onPointerFrame;
}

//------------------------------------------------------------------------
void InputHandler::initialize ()
{
	wl_seat* seat = RunLoop::getSeat ();
	if(seat == nullptr)
		return;

	// FIXME: this property might change when seat capabilities change
	if(RunLoop::hasPointerInput ())
	{
		pointer = wl_seat_get_pointer (seat);
		if(pointer)
			wl_pointer_add_listener (pointer, this, this);
	}

	initialized = true;
}

//------------------------------------------------------------------------
void InputHandler::terminate ()
{
	if(pointer)
		wl_pointer_destroy (pointer);
	pointer = nullptr;

	initialized = false;
}

//------------------------------------------------------------------------
void InputHandler::addWindow (ChildWindow* window)
{
	windows.push_back (window);
	if(!initialized)
		initialize ();
}

//------------------------------------------------------------------------
void InputHandler::removeWindow (ChildWindow* window)
{
	windows.erase (std::remove (windows.begin (), windows.end (), window));
	if(windows.empty ())
		terminate ();
}

//------------------------------------------------------------------------
void InputHandler::onPointerEnter (void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t x, wl_fixed_t y)
{
	InputHandler* This = static_cast<InputHandler*> (data);

	This->pointerEvent.eventMask |= kPointerEnter;
	This->pointerEvent.x = x;
	This->pointerEvent.y = y;
	This->pointerEvent.focus = surface;
	
	// TODO: remember the serial
}

//------------------------------------------------------------------------
void InputHandler::onPointerLeave (void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface)
{
	InputHandler* This = static_cast<InputHandler*> (data);

	This->pointerEvent.eventMask |= kPointerLeave;
	This->pointerEvent.previousFocus = surface;
	This->pointerEvent.focus = nullptr;
	This->pointerEvent.buttonState = 0;
}

//------------------------------------------------------------------------
void InputHandler::onPointerMotion (void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
	InputHandler* This = static_cast<InputHandler*> (data);
	
	This->pointerEvent.eventMask |= kPointerMotion;
	This->pointerEvent.time = time;
	This->pointerEvent.x = x;
	This->pointerEvent.y = y;
}

//------------------------------------------------------------------------
void InputHandler::onPointerButton (void* data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	InputHandler* This = static_cast<InputHandler*> (data);
	
	This->pointerEvent.eventMask |= kPointerButton;
	This->pointerEvent.time = time;
	This->pointerEvent.button = button;
	This->pointerEvent.state = state;
	
	MouseButton flag = MouseButton::None;
	switch(button)
	{
	case BTN_LEFT:
		flag = MouseButton::Left;
		break;
	case BTN_MIDDLE:
		flag = MouseButton::Middle;
		break;
	case BTN_RIGHT:
		flag = MouseButton::Right;
		break;
	}
	if(state == WL_POINTER_BUTTON_STATE_PRESSED)
	{
		This->pointerEvent.serial = serial;
		This->pointerEvent.buttonState |= (uint32_t)flag;
	}
	else
		This->pointerEvent.buttonState &= ~(uint32_t)flag;
}

//------------------------------------------------------------------------
void InputHandler::onPointerAxis (void* data, wl_pointer* wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	InputHandler* This = static_cast<InputHandler*> (data);
		
	if(axis >= 2)
		return;
	
	This->pointerEvent.eventMask |= kPointerAxis;
	This->pointerEvent.time = time;
	This->pointerEvent.axes[axis].value += value;
	This->pointerEvent.axes[axis].valid = true;
}

//------------------------------------------------------------------------
void InputHandler::onPointerAxisSource (void* data, wl_pointer* pointer, uint32_t axisSource)
{
	InputHandler* This = static_cast<InputHandler*> (data);
		
	This->pointerEvent.eventMask |= kPointerAxisSource;
	This->pointerEvent.axisSource = axisSource;
}

//------------------------------------------------------------------------
void InputHandler::onPointerAxisStop (void* data, wl_pointer* pointer, uint32_t time, uint32_t axis)
{
	InputHandler* This = static_cast<InputHandler*> (data);
		
	This->pointerEvent.eventMask |= kPointerAxisStop;
	This->pointerEvent.time = time;
	This->pointerEvent.axes[axis].valid = true;
}

//------------------------------------------------------------------------
void InputHandler::onPointerAxisDiscrete (void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete)
{
	InputHandler* This = static_cast<InputHandler*> (data);
		
	onPointerAxis120 (data, pointer, axis, discrete * 120);
}

//------------------------------------------------------------------------
void InputHandler::onPointerAxis120 (void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete)
{
	InputHandler* This = static_cast<InputHandler*> (data);
		
	if(axis >= 2)
		return;
	
	This->pointerEvent.eventMask |= kPointerAxisDiscrete;
	This->pointerEvent.axes[axis].discrete += discrete;
	This->pointerEvent.axes[axis].valid = true;
}

//------------------------------------------------------------------------
void InputHandler::onPointerFrame (void* data, wl_pointer* pointer)
{
	// TODO: maybe collect events first and dispatch them later to avoid delays inside Wayland dispatch
	// maybe move input handling to a separate queue
	// see https://gitlab.freedesktop.org/wayland/wayland/-/issues/159
	//     https://bugreports.qt.io/browse/QTBUG-66997
	//     https://bugs.kde.org/show_bug.cgi?id=392376
	//     https://gitlab.gnome.org/GNOME/mutter/-/issues/2234

	InputHandler* This = static_cast<InputHandler*> (data);
		
	PointerEvent event (This->pointerEvent);
	
	This->pointerEvent.eventMask = 0;
	This->pointerEvent.previousFocus = nullptr;
	for(int i = 0; i < 2; i++)
	{
		This->pointerEvent.axes[i].valid = false;
		This->pointerEvent.axes[i].value = 0;
		This->pointerEvent.axes[i].discrete = 0;
	}
	This->pointerEvent.time = 0;

	This->dispatch (event);
}

//------------------------------------------------------------------------
void InputHandler::dispatch (const PointerEvent& event)
{
	CPoint position (wl_fixed_to_int (event.x), wl_fixed_to_int (event.y));

	for(ChildWindow* window : windows)
	{
		IPlatformFrameCallback* frame = window->getFrame ();
		if(frame == nullptr)
			continue;

		// TODO: wheel, axis, enter, leave, ...

		// FIXME: when dragging / manipulating a control, we might also need to forward mouse move events to a surface that is not currently in focus
		if((event.eventMask & InputHandler::kPointerButton) && (window->getSurface () == event.focus || window->getSurface () == event.previousFocus))
		{
			if(event.state == WL_POINTER_BUTTON_STATE_RELEASED)
			{
				MouseUpEvent upEvent;
				upEvent.mousePosition = position;
				upEvent.buttonState.add ((MouseButton)event.buttonState);
				upEvent.timestamp = event.time;
				frame->platformOnEvent (upEvent);
			}
			else
			{
				MouseDownEvent downEvent;
				downEvent.mousePosition = position;
				downEvent.buttonState.add ((MouseButton)event.buttonState);
				downEvent.timestamp = event.time;
				frame->platformOnEvent (downEvent);
			}
		}

		if((event.eventMask & InputHandler::kPointerMotion) && event.focus == window->getSurface ())
		{
			MouseMoveEvent moveEvent;
			moveEvent.mousePosition = position;
			moveEvent.buttonState.add ((MouseButton)event.buttonState);
			moveEvent.timestamp = event.time;
			frame->platformOnEvent (moveEvent);
		}
	}
}

//------------------------------------------------------------------------
} // Wayland
} // VSTGUI
