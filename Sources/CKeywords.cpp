/*	$Id$
	
	Copyright 1996, 1997, 1998, 2002
	        Hekkelman Programmatuur B.V.  All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.
	3. All advertising materials mentioning features or use of this software
	   must display the following acknowledgement:
	   
	    This product includes software developed by Hekkelman Programmatuur B.V.
	
	4. The name of Hekkelman Programmatuur B.V. may not be used to endorse or
	   promote products derived from this software without specific prior
	   written permission.
	
	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
	AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 	

	Created: 09/18/97 20:36:58
*/

#include "pe.h"

#include <String.h>

#include "CKeywords.h"
#include "CLanguageAddOn.h"
#include "HError.h"


void GenerateKWMap(const char *file, const char *ext, map<BString,int>& kwMap)
{
	try
	{
		BPath settings;
		bool isNew = false;
		
		FailOSErr(find_directory(B_USER_SETTINGS_DIRECTORY, &settings, true));
	
		BString p;
		p << settings.Path() << "/pe/" << file;
	
		BEntry e;
		FailOSErrMsg(e.SetTo(p.String(), B_FILE_NODE), 
						 "Settings directory was not found?");

		BString keywords;
		if (!e.Exists())
		{
			// copy resources into separate file in settings-folder, such that
			// the user can edit that in order to modify the keywords for that
			// specific language:
			isNew = true;
			
			BFile rf;
			FailOSErr(rf.SetTo(ext, B_READ_ONLY));
			BResources res;
			FailOSErr(res.SetTo(&rf));
			
			size_t s;
			const char *r = (const char*)res.LoadResource('KeyW', file, &s);
			
			if (!r) THROW(("Missing resource"));
			
			BFile txtfile(p.String(), B_CREATE_FILE | B_READ_WRITE);
			CheckedWrite(txtfile, r, s);
			keywords.SetTo(r, s);
		} else {
			BFile txtfile(p.String(), B_READ_ONLY);
			off_t size;
			FailOSErr(txtfile.GetSize(&size));
			char* kw = keywords.LockBuffer(size+1);
			if (kw) {
				CheckedRead(txtfile, kw, size);
				keywords.UnlockBuffer(size);
			}
		}

		const char* kw = keywords.String();
		const char* white = " \n\r\t";
		const char* start = kw + strspn(kw, white);
		const char* end = start + strcspn(start, white);
		BString word;
		int currType = kKeywordLanguage;
		while(start < end) {
			word.SetTo(start, end-start);
			if (!word.Compare("//", 2)) {
				// a comment, so we skip to end of line:
				start += strcspn(start, "\n");
				start += strspn(start, white);
				end = start + strcspn(start, white);
			} else {
				if(word[0] == '-') {
					// it's a keyword-class specifier, we check which one:
					if (!word.ICompare("-Pe-Keywords-Language-"))
						currType = kKeywordLanguage;
					else if (!word.ICompare("-Pe-Keywords-User1-"))
						currType = kKeywordUser1;
					else if (!word.ICompare("-Pe-Keywords-User2-"))
						currType = kKeywordUser2;
					else if (!word.ICompare("-Pe-Keywords-User3-"))
						currType = kKeywordUser3;
					else if (!word.ICompare("-Pe-Keywords-User4-"))
						currType = kKeywordUser4;
					else {
						// be compatible with old style, meaning that an unknown
						// '-' entry bumps the type...
						currType++;
						// ...and skips to end of line:
						start += strcspn(start, "\n");
						start += strspn(start, white);
						end = start + strcspn(start, white);
					}
				} else {
					kwMap[word] = currType;
				}
				start = end + strspn(end, white);
				end = start + strcspn(start, white);
			}
		}
	}
	catch (HErr& err)
	{
		err.DoError();
	}
} /* GenerateKWTables */

