//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __animation__
#define __animation__

#include "animator.h"
#include "../crect.h"
#include "../vstguibase.h"

namespace VSTGUI {
class CView;

namespace Animation {

/// @brief animates the alpha value of the view
/// @ingroup AnimationTargets
//-----------------------------------------------------------------------------
class AlphaValueAnimation : public IAnimationTarget
{
public:
	AlphaValueAnimation (float endValue, bool forceEndValueOnFinish = false);

	void animationStart (CView* view, IdStringPtr name);
	void animationTick (CView* view, IdStringPtr name, float pos);
	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled);
protected:
	float startValue;
	float endValue;
	bool forceEndValueOnFinish;
};

/// @brief animates the view size of the view
/// @ingroup AnimationTargets
//-----------------------------------------------------------------------------
class ViewSizeAnimation : public IAnimationTarget
{
public:
	ViewSizeAnimation (const CRect& newRect, bool forceEndValueOnFinish = false);

	void animationStart (CView* view, IdStringPtr name);
	void animationTick (CView* view, IdStringPtr name, float pos);
	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled);
protected:
	CRect startRect;
	CRect newRect;
	bool forceEndValueOnFinish;
};

/// @brief exchange a view by another view with an animation
/// @ingroup AnimationTargets
//-----------------------------------------------------------------------------
class ExchangeViewAnimation : public IAnimationTarget
{
public:
	enum AnimationStyle {
		kAlphaValueFade = 0,
		kPushInFromLeft,
		kPushInFromRight,
		kPushInFromTop,
		kPushInFromBottom
	};

	/** oldView must be a subview of the animation view */
	ExchangeViewAnimation (CView* oldView, CView* newView, AnimationStyle style = kAlphaValueFade);
	~ExchangeViewAnimation ();
	void animationStart (CView* view, IdStringPtr name);
	void animationTick (CView* view, IdStringPtr name, float pos);
	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled);

protected:
	CView* newView;
	CView* viewToRemove;
	AnimationStyle style;
	float newViewValueEnd;
	float oldViewValueStart;
};

}} // namespaces

#endif // __animation__