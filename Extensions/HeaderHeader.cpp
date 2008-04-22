/*
 * HeaderHeader is an Extension.
 */

#include "PeAddOn.h"
#include <ctype.h>
#include <time.h>
#include <FilePanel.h>
#include <MenuItem.h>
#include <PopUpMenu.h>

static const char *sTrackerSig = "application/x-vnd.Be-TRAK";

static const char *sHaikuHeaderTemplate = "%COMMS%
%COMMC% Copyright %YEAR%, Haiku.
%COMMC% Distributed under the terms of the MIT License.
%COMMC%
%COMMC% Authors:
%COMMC%		%AUTHOR% <%AUTHORMAIL%>
%COMME%
";

static const char *sHaikuAddMeHeaderMatch = \
" * Authors:
";
static const char *sHaikuAddMeHeaderTemplate = \
" *	 	%AUTHOR% <%AUTHORMAIL%>
";


static const char *sHaikuMeHeaderTemplate = "%COMMS%
%COMMC% Copyright %YEAR%, %AUTHOR%, <%AUTHORMAIL%>.
%COMMC% Distributed under the terms of the MIT License.
%COMMC%	
%COMME%
";

/*
static const char *sHaikuMeAddMeHeaderTemplate = \
" * Copyright %YEAR%, %AUTHOR%, <%AUTHORMAIL%>.
";
*/

static const char *sIdMeMITCreatedHeaderTemplate = \
"%COMMS%	$Id: %FILENAME% $
%COMML%
%COMML%	Copyright %YEAR% %AUTHOR%
%COMML%
%COMML%	Distributed under the MIT License
%COMML%
%COMML%	Created: %DATE%
%COMME%
";

static const char *sIdMeFullMITCreatedHeaderTemplate = \
"%COMMS%	$Id: %FILENAME% $
%COMML%	
%COMML%	Copyright %YEAR%
%COMML%	        %AUTHOR%  All rights reserved.
%COMML%	
%COMML%	Redistribution and use in source and binary forms, with or without
%COMML%	modification, are permitted provided that the following conditions are met:
%COMML%	1. Redistributions of source code must retain the above copyright notice,
%COMML%	   this list of conditions and the following disclaimer.
%COMML%	2. Redistributions in binary form must reproduce the above copyright notice,
%COMML%	   this list of conditions and the following disclaimer in the documentation
%COMML%	   and/or other materials provided with the distribution.
%COMML%	3. All advertising materials mentioning features or use of this software
%COMML%	   must display the following acknowledgement:
%COMML%	   
%COMML%	    This product includes software developed by %AUTHOR%.
%COMML%	
%COMML%	4. The name of %AUTHOR% may not be used to endorse or
%COMML%	   promote products derived from this software without specific prior
%COMML%	   written permission.
%COMML%	
%COMML%	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
%COMML%	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
%COMML%	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
%COMML%	AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
%COMML%	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
%COMML%	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
%COMML%	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
%COMML%	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
%COMML%	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
%COMML%	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 	
%COMML%
%COMML%	Created: %DATE% %TIME%
%COMME%
";

static const char *sIdMeFullMITRevisedCreatedHeaderTemplate = \
"%COMMS%	$Id: %FILENAME% $
%COMML%	
%COMML%	Copyright %YEAR%
%COMML%	        %AUTHOR%  All rights reserved.
%COMML%	
%COMML%	Redistribution and use in source and binary forms, with or without
%COMML%	modification, are permitted provided that the following conditions are met:
%COMML%	1. Redistributions of source code must retain the above copyright notice,
%COMML%	   this list of conditions and the following disclaimer.
%COMML%	2. Redistributions in binary form must reproduce the above copyright notice,
%COMML%	   this list of conditions and the following disclaimer in the documentation
%COMML%	   and/or other materials provided with the distribution.
%COMML%	4. The name of %AUTHOR% may not be used to endorse or
%COMML%	   promote products derived from this software without specific prior
%COMML%	   written permission.
%COMML%	
%COMML%	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
%COMML%	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
%COMML%	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
%COMML%	AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
%COMML%	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
%COMML%	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
%COMML%	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
%COMML%	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
%COMML%	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
%COMML%	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 	
%COMML%
%COMML%	Created: %DATE% %TIME%
%COMME%
";


//------------------------------------------------------------------------------
//	#pragma mark - implementation


status_t
GetSettingsDir(BDirectory &dir, BPath &path)
{
	//BPath path;
	status_t err;
	// TODO: build list from text files
	err = find_directory(B_COMMON_SETTINGS_DIRECTORY, &path, true);
	if (err < B_OK)
		return err;
	dir.SetTo(path.Path());
	if (!dir.Contains("pe"))
		dir.CreateDirectory("pe", NULL);
	path.Append("pe");
	dir.SetTo(path.Path());
	if (!dir.Contains("HeaderTemplates"))
		dir.CreateDirectory("HeaderTemplates", NULL);
	path.Append("HeaderTemplates");
	dir.SetTo(path.Path());
	return B_OK;
}

void
AddTemplateItem(BPopUpMenu *menu, const char *name, const char *tmpl, const char *match=NULL)
{
	BMessage *msg;
	BMenuItem *item;
	msg = new BMessage('head');
	msg->AddString("template", tmpl);
	if (match)
		msg->AddString("match", match);
	item = new BMenuItem(name, msg);
	menu->AddItem(item);
}


BPopUpMenu *
BuildPopUp(BDirectory &dir)
{
	BPopUpMenu *menu = new BPopUpMenu("menu", false);
	//BMenuItem *item;
	//BMessage *msg;
	status_t err;

	//menu->SetFont(be_plain_font);

	AddTemplateItem(menu, B_UTF8_COPYRIGHT " Haiku", 
		sHaikuHeaderTemplate);

/*
	AddTemplateItem(menu, B_UTF8_COPYRIGHT " Haiku (Add me)", 
		sHaikuAddMeHeaderTemplate, sHaikuAddMeHeaderMatch);
*/

	AddTemplateItem(menu, B_UTF8_COPYRIGHT " Me (Haiku)", 
		sHaikuMeHeaderTemplate);

	AddTemplateItem(menu, "Id + " B_UTF8_COPYRIGHT "Me + MIT + Created", 
		sIdMeMITCreatedHeaderTemplate);

	AddTemplateItem(menu, "Id + " B_UTF8_COPYRIGHT "Me + Full MIT + Created", 
		sIdMeFullMITCreatedHeaderTemplate);

	AddTemplateItem(menu, "Id + " B_UTF8_COPYRIGHT "Me + Full MIT Revised + Created", 
		sIdMeFullMITRevisedCreatedHeaderTemplate);

	if (dir.InitCheck() < B_OK)
		return menu;

	entry_ref ref;
	while (dir.GetNextRef(&ref) == B_OK) {
		BFile file(&ref, B_READ_ONLY);
		if (file.InitCheck() < B_OK)
			continue;
		BString str;
		char *p = str.LockBuffer(1024);
		memset(p, 0, 1024);
		if (file.Read(p, 1024) <= 0)
			continue;
		str.UnlockBuffer();
		
		AddTemplateItem(menu, ref.name, str.String());
	}

	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Open Template Folder", new BMessage('optf')));
	menu->AddItem(new BMenuItem("Set Author", new BMessage('seta')));
	return menu;
}


status_t
RunPopUpMenu(BPoint where, BString &header, BString &fileName)
{
	status_t err;
	BPath path;
	BDirectory dir;
	err = GetSettingsDir(dir, path);
	err = B_ERROR;
	BPopUpMenu *menu = BuildPopUp(dir);
	if (menu == NULL)
		return B_ERROR;
	
	BMenuItem *item = menu->Go(where, false, true);
	//if (item && item->Message())
	//	item->Message()->PrintToStream();

	switch ((item && item->Message()) ? item->Message()->what : 0) {
		case 'head':
		{
			if (item->Message()->FindString("template", &header) < B_OK)
				break;
			BString tmp;
			time_t now = time(NULL);
			struct tm *tim = localtime(&now);
			// date
			char *p;
			p = tmp.LockBuffer(100);
			memset(p, 0, 100);
			strftime(p, 100, "%F", tim);
			tmp.UnlockBuffer();
			header.ReplaceAll("%DATE%", tmp.String());
			tmp.Truncate(0);
			
			p = tmp.LockBuffer(100);
			memset(p, 0, 100);
			strftime(p, 100, "%T", tim);
			tmp.UnlockBuffer();
			header.ReplaceAll("%TIME%", tmp.String());
			tmp.Truncate(0);

			// year
			p = tmp.LockBuffer(10);
			memset(p, 0, 10);
			strftime(p, 10, "%Y", tim);
			tmp.UnlockBuffer();
			header.ReplaceAll("%YEAR%", tmp.String());
			tmp.Truncate(0);

			// TODO fetch from query on META:email==**
			// or specific people file
			tmp << "François Revol";
			header.ReplaceAll("%AUTHOR%", tmp.String());
			tmp.Truncate(0);
			tmp << "revol@free.fr";
			header.ReplaceAll("%AUTHORMAIL%", tmp.String());
			tmp.Truncate(0);

			BString fileNameNoExt(fileName);
			if (fileNameNoExt.FindLast('.') > -1)
				fileNameNoExt.Truncate(fileNameNoExt.FindLast('.'));
			header.ReplaceAll("%FILENAMENOEXT%", fileNameNoExt.String());
			header.ReplaceAll("%FILENAME%", fileName.String());
			/*
			tmp << "Haiku";
			header.ReplaceAll("%PROJECT%", tmp.String());
			tmp.Truncate(0);
			*/

			// TODO: depend on language
			// comment start
			header.ReplaceAll("%COMMS%", "/*");
			// comment cont'd
			header.ReplaceAll("%COMMC%", " *");
			// comment cont'd lazy (blank if possible)
			header.ReplaceAll("%COMML%", "");
			// comment end
			header.ReplaceAll("%COMME%", " */");


			err = B_OK;
			break;
		}
		case 'optf':
		{
			const char *args[] = {path.Path(), NULL};
			err = be_roster->Launch(sTrackerSig, 1, (char **)args);
			//printf("err %s\n", strerror(err));
			err = B_OK;
			break;
		}
		case 'seta':
			// TODO
			//BFilePanel *panel = new ;
			break;
		case 0:
			err = B_CANCELED;
			break;
		default:
			break;
	}
	delete menu;
	return err;
}

//------------------------------------------------------------------------------
//	#pragma mark - 


#if __INTEL__
	extern "C" _EXPORT long perform_edit(MTextAddOn *addon);
#else
#	pragma export on
	extern "C"
		long perform_edit(MTextAddOn *addon);
#	pragma export reset
#endif

long perform_edit(MTextAddOn *addon)
{
	long result = B_OK;
	entry_ref headerFile;
	BPoint where(0,0);

	if (addon->Window() && addon->Window()->Lock()) {
		uint32 buttons;
		addon->Window()->ChildAt(0)->GetMouse(&where, &buttons);
		addon->Window()->ChildAt(0)->ConvertToScreen(&where);
		addon->Window()->Unlock();
		where += BPoint(-3,-3);
	}

	result = addon->GetRef(headerFile);
	BString fileName;
	if (result >= B_OK)
		fileName = headerFile.name;

	BString header;
	result = RunPopUpMenu(where, header, fileName);
	//printf("result %s\n", strerror(result));
	if (result == B_CANCELED)
		return B_OK;
	if (result < B_OK)
		return result;

#if 0
	// Do not change the case if a shift key was pressed
	if ((modifiers() & B_SHIFT_KEY) == 0)
		fileName.ToUpper();
#endif

	addon->Select(0, 0);

	addon->Insert(header.String());
	
	return result;
}

