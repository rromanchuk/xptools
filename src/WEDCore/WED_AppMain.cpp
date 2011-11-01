/*
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

 #define __DEBUGGING__

#include "WED_AboutBox.h"
// Stuff we need to init
//#include "XESInit.h"
#include "WED_Document.h"
#include "WED_Assert.h"
//#include "DEMTables.h"
#include "WED_AboutBox.h"
#include "WED_StartWindow.h"
//#include "ObjTables.h"
#include <CGAL/assertions_behaviour.h>
#include "GUI_Clipboard.h"
#include "WED_Package.h"
#include "GUI_Resources.h"
#include "WED_Application.h"
#include "WED_PackageMgr.h"
#include "GUI_Pane.h"
#include "GUI_Fonts.h"
#include "GUI_Window.h"
#include "GUI_Prefs.h"

#include "WED_Menus.h"

#include "GUI_ScrollerPane.h"
#include "GUI_TextField.h"
#include "GUI_Splitter.h"

#define	REGISTER_LIST	\
	_R(WED_Airport) \
	_R(WED_AirportBeacon) \
	_R(WED_AirportBoundary) \
	_R(WED_AirportChain) \
	_R(WED_Ring) \
	_R(WED_AirportNode) \
	_R(WED_AirportSign) \
	_R(WED_Group) \
	_R(WED_Helipad) \
	_R(WED_KeyObjects) \
	_R(WED_LightFixture) \
	_R(WED_ObjPlacement) \
	_R(WED_RampPosition) \
	_R(WED_Root) \
	_R(WED_Runway) \
	_R(WED_RunwayNode) \
	_R(WED_Sealane) \
	_R(WED_Select) \
	_R(WED_Taxiway) \
	_R(WED_TowerViewpoint) \
	_R(WED_Windsock) \
	_R(WED_ATCFrequency) \
	_R(WED_TextureNode) \
	_R(WED_TextureBezierNode) \
	_R(WED_OverlayImage) \
	_R(WED_SimpleBoundaryNode) \
	_R(WED_SimpleBezierBoundaryNode) \
	_R(WED_LinePlacement) \
	_R(WED_StringPlacement) \
	_R(WED_ForestPlacement) \
	_R(WED_FacadePlacement) \
	_R(WED_PolygonPlacement) \
	_R(WED_DrapedOrthophoto) \
	_R(WED_ExclusionZone) \
	_R(WED_ForestRing) \
	_R(WED_FacadeRing) \
	_R(WED_FacadeNode)

#define REGISTER_LIST_ATC \
	_R(WED_TaxiRoute) \
	_R(WED_ATCFlow) \
	_R(WED_ATCRunwayUse) \
	_R(WED_RoadEdge)

#define _R(x)	extern void x##_Register();
REGISTER_LIST
#if AIRPORT_ROUTING
REGISTER_LIST_ATC
#endif
#undef _R

#include "WED_EnumSystem.h"

#if IBM
HINSTANCE gInstance = NULL;
#endif
#if LIN
#include "initializer.h"
#endif


CGAL::Failure_function	gFailure = NULL;
void	cgal_failure(const char* a, const char* b, const char* c, int d, const char* e)
{
#if IBM
	char str[65536];
	snprintf(str, 65536, "%s: %s\n(%s: %d)\n%s\n",a,b,c,d,e);
	MessageBox(0, str, "CGAL error", MB_OK);
#else
	printf("%s: %s\n(%s: %d)\n%s\n",a,b,c,d,e);
#endif
	if (gFailure)
		(*gFailure)(a, b, c, d, e);
	throw a;
}

void	cgal_warning(const char* a, const char* b, const char* c, int d, const char* e)
{
#if IBM
	char str[65536];
	snprintf(str, 65536, "%s: %s\n(%s: %d)\n%s\n",a,b,c,d,e);
	MessageBox(0, str, "CGAL warning", MB_OK);
#else
	printf("%s: %s\n(%s: %d)\n%s\n",a,b,c,d,e);
#endif
}

#if IBM
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, char * argv[])
#endif
{
#if IBM
	gInstance = hInstance;
	SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
#endif
	GUI_MemoryHog::InstallNewHandler();
	GUI_InitClipboard();
#if LIN
	Initializer linit(&argc, &argv, false);
	WED_Application	app(argc, argv);
#else
	WED_Application	app;
#endif
	WED_PackageMgr	pMgr(NULL);

	// Ben says: the about box is actually integral to WED's operation.  WED uses a series of shared OGL contexts to hold
	// our textures, and WED cannot handle the textures being thrown away and needing a reload.  So logically we must have
	// at least one shared context so that the textures are not purged.
	// This means one window must always be in existence.  That window is the about box...which stays hidden but allocated to
	// sustain OpenGL.

	WED_AboutBox * about = new WED_AboutBox(&app);
	WED_MakeMenus(&app);
	WED_StartWindow * start = new WED_StartWindow(&app);

	start->Show();

	gFailure = CGAL::set_error_handler(cgal_failure);
#if DEV
	CGAL::set_warning_handler(cgal_warning);
#endif

	start->ShowMessage("Initializing...");
//	XESInit();

	start->ShowMessage("Reading Prefs...");
	GUI_Prefs_Read("WED");
	WED_Document::ReadGlobalPrefs();

	start->ShowMessage("Scanning X-System Folder...");
	pMgr.SetXPlaneFolder(GUI_GetPrefString("packages","xsystem",""));

//	start->ShowMessage("Loading DEM tables...");
//	LoadDEMTables();
//	start->ShowMessage("Loading OBJ tables...");
//	LoadObjTables();

	start->ShowMessage("Loading ENUM system...");
	WED_AssertInit();
	ENUM_Init();

	start->ShowMessage("Registering classes...");
	#define _R(x)	x##_Register();
	REGISTER_LIST
	#if AIRPORT_ROUTING
	REGISTER_LIST_ATC
	#endif
	#undef _R

	for (int y = 0; y < 2; ++y)
	for (int x = 0; x < 4; ++x)
	{
		char	fname[30];
		sprintf(fname,"earth_%d%d.jpg",x+1,y+1);
		int	tex_id = GUI_GetTextureResource(fname, 0, NULL);
		sprintf(fname,"Loading earth_%d%d.jpg",x+1,y+1);
		start->ShowMessage(fname);
	}

	app.SetAbout(about);

	start->ShowMessage(string());
	setlocale(LC_ALL,"C");
	app.Run();
    // we're out of eventloop here, deleting windows on linux implies sending messages to them,
    // so this would fail.
	#if !LIN
	    delete about;
	#endif

	GUI_MemoryHog::RemoveNewHandler();

	string xsys;
	pMgr.GetXPlaneFolder(xsys);
	GUI_SetPrefString("packages","xsystem",xsys.c_str());

	WED_Document::WriteGlobalPrefs();
	GUI_Prefs_Write("WED");

	return 0;
}