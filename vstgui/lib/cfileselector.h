// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cstring.h"
#include <functional>
#include <memory>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CFileExtension Declaration
//! @brief file extension description
//-----------------------------------------------------------------------------
class CFileExtension
{
public:
	CFileExtension (const UTF8String& description, const UTF8String& extension,
					const UTF8String& mimeType = "", int32_t macType = 0,
					const UTF8String& uti = "");
	CFileExtension (const CFileExtension& ext);
	~CFileExtension () noexcept;

	const UTF8String& getDescription () const;
	const UTF8String& getExtension () const;
	const UTF8String& getMimeType () const;
	const UTF8String& getUTI () const;
	int32_t getMacType () const;

	bool operator== (const CFileExtension& ext) const;
//-----------------------------------------------------------------------------
	CFileExtension (CFileExtension&& ext) noexcept;
	CFileExtension& operator= (CFileExtension&& ext) noexcept;

	CFileExtension (const PlatformFileExtension&);
	const PlatformFileExtension& getPlatformFileExtension () const;

protected:
	CFileExtension ();
	void init (const UTF8String& description, const UTF8String& extension,
			   const UTF8String& mimeType, const UTF8String& uti);

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//-----------------------------------------------------------------------------
// CNewFileSelector Declaration
//! @brief New file selector class
/*! @class CNewFileSelector
@section usage Usage
Running the file selector
@code
void MyClass::runFileSelector ()
{
	CNewFileSelector* selector = CNewFileSelector::create (getFrame (), CNewFileSelector::kSelectFile);
	if (selector)
	{
		selector->addFileExtension (CFileExtension ("AIFF", "aif", "audio/aiff"));
		selector->setDefaultExtension (CFileExtension ("WAVE", "wav"));
		selector->setTitle("Choose An Audio File");
		selector->run (this);
		selector->forget ();
	}
}
@endcode
Getting results
@code
CMessageResult MyClass::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CNewFileSelector::kSelectEndMessage)
	{
		CNewFileSelector* sel = dynamic_cast<CNewFileSelector*>(sender);
		if (sel)
		{
			// do anything with the selected files here
			return kMessageNotified;
		}
	}
	return parent::notify (sender, message);
}
@endcode
*/
//-----------------------------------------------------------------------------
class CNewFileSelector final : public CBaseObject
{
public:
	enum Style {
		/** select file(s) selector style */
		kSelectFile,
		/** select save file selector style */
		kSelectSaveFile,
		/** select directory style */
		kSelectDirectory
	};
	
	//-----------------------------------------------------------------------------
	/// @name CFileSelector running
	//-----------------------------------------------------------------------------
	//@{
	/** create a new instance */
	static CNewFileSelector* create (CFrame* parent = nullptr, Style style = kSelectFile);

	CNewFileSelector (PlatformFileSelectorPtr&& platformFileSelector, CFrame* parent);

	using CallbackFunc = std::function<void(CNewFileSelector*)>;
	bool run (CallbackFunc&& callback);
	/** the delegate will get a kSelectEndMessage throu the notify method where the sender is this CNewFileSelector object */
	bool run (CBaseObject* delegate);
	/** cancel running the file selector */
	void cancel ();
	/** run as modal dialog */
	bool runModal ();
	//@}

	//-----------------------------------------------------------------------------
	/// @name CFileSelector setup
	//-----------------------------------------------------------------------------
	//@{
	/** set title of file selector */
	void setTitle (const UTF8String& title);
	/** set initial directory (UTF8 string) */
	void setInitialDirectory (const UTF8String& path);
	/** set initial save name (UTF8 string) */
	void setDefaultSaveName (const UTF8String& name);
	/** set default file extension */
	void setDefaultExtension (const CFileExtension& extension);
	/** set allow multi file selection (only valid for kSelectFile selector style) */
	void setAllowMultiFileSelection (bool state);
	/** add a file extension */
	void addFileExtension (const CFileExtension& extension);
	/** add a file extension */
	void addFileExtension (CFileExtension&& extension);
	//@}

	//-----------------------------------------------------------------------------
	/// @name CFileSelector result
	//-----------------------------------------------------------------------------
	//@{
	/** get number of selected files */
	uint32_t getNumSelectedFiles () const;
	/** get selected file. Result is only valid as long as the instance of CNewFileSelector is valid. */
	UTF8StringPtr getSelectedFile (uint32_t index) const;
	//@}

	/** get the all files extension */
	static const CFileExtension& getAllFilesExtension ();

	static IdStringPtr kSelectEndMessage;
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY (CNewFileSelector, CBaseObject)
protected:
	~CNewFileSelector () noexcept override;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

} // VSTGUI
