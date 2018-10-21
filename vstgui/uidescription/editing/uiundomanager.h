// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uiundomanager__
#define __uiundomanager__

#include "../../lib/vstguibase.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/dispatchlist.h"
#include <list>
#include <deque>

namespace VSTGUI {
class IAction;
class UIGroupAction;

//----------------------------------------------------------------------------------------------------
class UIUndoManager : public CBaseObject,
                      public GenericChangeT<UIUndoManager>,
                      protected std::list<IAction*>
{
public:
	UIUndoManager ();
	~UIUndoManager () override;

	void pushAndPerform (IAction* action);

	UTF8StringPtr getUndoName ();
	UTF8StringPtr getRedoName ();
	
	void performUndo ();
	void performRedo ();
	bool canUndo ();
	bool canRedo ();

	void startGroupAction (UTF8StringPtr name);
	void endGroupAction ();
	void cancelGroupAction ();

	void clear ();

	void markSavePosition ();
	bool isSavePosition () const;
protected:
	iterator position;
	iterator savePosition;
	using GroupActionDeque = std::deque<UIGroupAction*>;
	GroupActionDeque groupQueue;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiundomanager__
