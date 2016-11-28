// CGWorkView.cpp : implementation of the CCGWorkView class
//
#include "stdafx.h"
#include "CGWork.h"

#include "CGWorkDoc.h"
#include "CGWorkView.h"

#include <iostream>
#include <functional>
using std::cout;
using std::endl;
#include "MaterialDlg.h"
#include "LightDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "PngWrapper.h"
#include "iritSkel.h"
#include "MouseSensetiveDialog.h"
#include "mat4.h"
#include "ColorSelectionDialog.h"
#include <math.h>
#include "line.h"
#include <unordered_map>
// For Status Bar access
#include "MainFrm.h"



// Use this macro to display text messages in the status bar.
#define STATUS_BAR_TEXT(str) (((CMainFrame*)GetParentFrame())->getStatusBar().SetWindowText(str))

#define IN_RANGE(x, y) ((1 <= x) && (x < (m_WindowHeight - 1)) && (1 <= y) && (y < (m_WindowWidth - 1)))


/////////////////////////////////////////////////////////////////////////////
// CCGWorkView

IMPLEMENT_DYNCREATE(CCGWorkView, CView)

BEGIN_MESSAGE_MAP(CCGWorkView, CView)
	//{{AFX_MSG_MAP(CCGWorkView)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_MESSAGE(WM_MOUSEMOVE, OnMouseMovement)
	ON_COMMAND(ID_FILE_LOAD, OnFileLoad)
	ON_COMMAND(ID_WIREFRAME_COLOUR, OnWriteframeColor)
	ON_COMMAND(ID_VIEW_ORTHOGRAPHIC, OnViewOrthographic)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ORTHOGRAPHIC, OnUpdateViewOrthographic)
	ON_COMMAND(ID_VIEW_PERSPECTIVE, OnViewPerspective)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PERSPECTIVE, OnUpdateViewPerspective)
	ON_COMMAND(ID_ACTION_ROTATE, OnActionRotate)
	ON_UPDATE_COMMAND_UI(ID_ACTION_ROTATE, OnUpdateActionRotate)
	ON_COMMAND(ID_ACTION_SCALE, OnActionScale)
	ON_UPDATE_COMMAND_UI(ID_ACTION_SCALE, OnUpdateActionScale)
	ON_COMMAND(ID_ACTION_TRANSLATE, OnActionTranslate)
	ON_UPDATE_COMMAND_UI(ID_ACTION_TRANSLATE, OnUpdateActionTranslate)
	ON_COMMAND(ID_ACTION_RESETVIEW, OnActionResetView)
	ON_COMMAND(ID_AXIS_X, OnAxisX)
	ON_UPDATE_COMMAND_UI(ID_AXIS_X, OnUpdateAxisX)
	ON_COMMAND(ID_AXIS_Y, OnAxisY)
	ON_UPDATE_COMMAND_UI(ID_AXIS_Y, OnUpdateAxisY)
	ON_COMMAND(ID_AXIS_Z, OnAxisZ)
	ON_UPDATE_COMMAND_UI(ID_AXIS_Z, OnUpdateAxisZ)
	ON_COMMAND(ID_AXIS_XY, OnAxisXY)
	ON_UPDATE_COMMAND_UI(ID_AXIS_XY, OnUpdateAxisXY)
	ON_COMMAND(ID_BOUNDBOX, OnBoundBox)
	ON_UPDATE_COMMAND_UI(ID_BOUNDBOX, OnUpdateBoundBox)
	ON_COMMAND(ID_LIGHT_SHADING_FLAT, OnLightShadingFlat)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_FLAT, OnUpdateLightShadingFlat)
	ON_COMMAND(ID_LIGHT_SHADING_GOURAUD, OnLightShadingGouraud)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_GOURAUD, OnUpdateLightShadingGouraud)
	ON_COMMAND(ID_LIGHT_CONSTANTS, OnLightConstants)
	ON_COMMAND(ID_BUTTON_TRANS_TOGGLE, OnActionToggleView)
	ON_COMMAND(ID_OPTIONS_MOUSESENSITIVITY, OnOptionMouseSensetivity)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TRANS_TOGGLE, OnUpdateActionToggleView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// A patch to fix GLaux disappearance from VS2005 to VS2008
void auxSolidCone(GLdouble radius, GLdouble height) {
        GLUquadric *quad = gluNewQuadric();
        gluQuadricDrawStyle(quad, GLU_FILL);
        gluCylinder(quad, radius, 0.0, height, 20, 20);
        gluDeleteQuadric(quad);
}

/////////////////////////////////////////////////////////////////////////////
// CCGWorkView construction/destruction

CCGWorkView::CCGWorkView()
{
	// Set default values
	m_nAxis = ID_AXIS_X;
	m_nAction = ID_ACTION_ROTATE;
	m_nView = ID_VIEW_ORTHOGRAPHIC;	
	m_screen = NULL;

	m_object_space_trans = false;
	m_bound_box = false;
	m_mouse_sensetivity = 1;

	m_bIsPerspective = false;
	m_tarnsform[0][0] = 1;
	m_tarnsform[1][1] = 1;
	m_tarnsform[2][2] = 1;
	m_tarnsform[3][3] = 1;

	m_color_wireframe = RGB(0, 0, 0);
	m_background_color = RGB(255, 255, 255);
	m_boundbox_color = RGB(0, 0, 0);

	m_nLightShading = ID_LIGHT_SHADING_FLAT;

	m_lMaterialAmbient = 0.2;
	m_lMaterialDiffuse = 0.8;
	m_lMaterialSpecular = 1.0;
	m_nMaterialCosineFactor = 32;


	//init the first light to be enabled
	m_lights[LIGHT_ID_1].enabled=true;
}

CCGWorkView::~CCGWorkView()
{
}


/////////////////////////////////////////////////////////////////////////////
// CCGWorkView diagnostics

#ifdef _DEBUG
void CCGWorkView::AssertValid() const
{
	CView::AssertValid();
}

void CCGWorkView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCGWorkDoc* CCGWorkView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCGWorkDoc)));
	return (CCGWorkDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CCGWorkView Window Creation - Linkage of windows to CGWork

BOOL CCGWorkView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	// An CGWork window must be created with the following
	// flags and must NOT include CS_PARENTDC for the
	// class style.

	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	return CView::PreCreateWindow(cs);
}



int CCGWorkView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	InitializeCGWork();

	return 0;
}


// This method initialized the CGWork system.
BOOL CCGWorkView::InitializeCGWork()
{
	m_pDC = new CClientDC(this);

	m_hDC = CreateCompatibleDC(m_pDC->GetSafeHdc());

	if (NULL == m_hDC) { // failure to get DC
		::AfxMessageBox(CString("Couldn't get a valid DC."));
		return FALSE;
	}

	if ( NULL == m_pDC ) { // failure to get DC
		::AfxMessageBox(CString("Couldn't get a valid DC."));
		return FALSE;
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CCGWorkView message handlers


void CCGWorkView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	if ( 0 >= cx || 0 >= cy ) {
		return;
	}

	// save the width and height of the current window
	m_WindowWidth = cx;
	m_WindowHeight = cy;

	// compute the aspect ratio
	// this will keep all dimension scales equal
	m_AspectRatio = (GLdouble)m_WindowWidth/(GLdouble)m_WindowHeight;
}


BOOL CCGWorkView::SetupViewingFrustum(void)
{
    return TRUE;
}


// This viewing projection gives us a constant aspect ration. This is done by
// increasing the corresponding size of the ortho cube.
BOOL CCGWorkView::SetupViewingOrthoConstAspect(void)
{
	return TRUE;
}





BOOL CCGWorkView::OnEraseBkgnd(CDC* pDC) 
{
	// Windows will clear the window with the background color every time your window 
	// is redrawn, and then CGWork will clear the viewport with its own background color.

	// return CView::OnEraseBkgnd(pDC);
	return true;
}



/////////////////////////////////////////////////////////////////////////////
// CCGWorkView drawing
/////////////////////////////////////////////////////////////////////////////

void CCGWorkView::OnDraw(CDC* pDC)
{
	CCGWorkDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	//delete m_screen;
	if (m_screen == NULL)
		m_screen = (COLORREF*)calloc(m_WindowWidth * m_WindowHeight, sizeof(COLORREF));

	mat4 screen_space_scale;
	mat4 screen_space_translate;


	int min_axis = min(m_WindowHeight, m_WindowWidth);

	screen_space_scale[0][0] = (double)min_axis*0.6;
	screen_space_scale[1][1] = (double)min_axis*0.6;
	screen_space_scale[2][2] = (double)min_axis*0.6;
	screen_space_scale[3][3] = 1;

	screen_space_translate[0][0] = 1;
	screen_space_translate[3][0] = 0.5 * m_WindowHeight;

	screen_space_translate[1][1] = 1;
	screen_space_translate[3][1] = 0.5 * m_WindowWidth;

	screen_space_translate[2][2] = 1;
	screen_space_translate[3][3] = 1;

	m_screen_space_trans = screen_space_scale * screen_space_translate;

	RenderScene();

	if (!pDoc)
	    return;
}


/////////////////////////////////////////////////////////////////////////////
// CCGWorkView CGWork Finishing and clearing...

void CCGWorkView::OnDestroy() 
{
	CView::OnDestroy();
	if (m_screen){
		delete m_screen;
	}

	if (m_hDC){
		DeleteDC(m_hDC);
	}
	// delete the DC
	if ( m_pDC ) {
		delete m_pDC;
	}
}



/////////////////////////////////////////////////////////////////////////////
// User Defined Functions
bool CCGWorkView::InRange(int x, int y, int width, int height){
	return (0 <= x && x < height && 0 <= y && y < width);
};

bool past_pressed;
LRESULT CCGWorkView::OnMouseMovement(WPARAM wparam, LPARAM lparam){
	int xPos = GET_X_LPARAM(lparam);
	int yPos = GET_Y_LPARAM(lparam);

	if (wparam == MK_LBUTTON && past_pressed)
	{
		mat4 temp_transform;
		mat4 temp_transform_xy;

		// x mouse movement params
		double diff_x = m_mouse_sensetivity * (xPos - m_mouse_xpos);
		
		double teta_x = 10 * asin((double)diff_x / m_WindowWidth);

		double sinx = sin(teta_x);
		
		double cosx = cos(teta_x); // sin(x) ~= x movements are very small

		// x mouse movement params
		double diff_y = m_mouse_sensetivity * (yPos - m_mouse_ypos);
		
		double teta_y = 10 * asin((double)diff_y / m_WindowHeight);  //TODO scales...
		double siny = sin(teta_y);
		
		double cosy = cos(teta_y); // sin(y) ~= y movements are very small

		if (m_nAction == ID_ACTION_ROTATE){
			if (m_nAxis == ID_AXIS_X){
				temp_transform[0][0] = 1;

				temp_transform[1][1] = cosx;
				temp_transform[1][2] = sinx;

				temp_transform[2][1] = -sinx;
				temp_transform[2][2] = cosx;

				temp_transform[3][3] = 1;

			}
			if (m_nAxis == ID_AXIS_Y){
				temp_transform[0][0] = cosy;
				temp_transform[0][2] = siny;

				temp_transform[1][1] = 1;

				temp_transform[2][0] = -siny;
				temp_transform[2][2] = cosy;

				temp_transform[3][3] = 1;
			}
			if (m_nAxis == ID_AXIS_Z){
				temp_transform[0][0] = cosx;
				temp_transform[0][1] = sinx;

				temp_transform[1][0] = -sinx;
				temp_transform[1][1] = cosx;

				temp_transform[2][2] = 1;

				temp_transform[3][3] = 1;
			}
			if (m_nAxis == ID_AXIS_XY){
				temp_transform[0][0] = 1;

				temp_transform[1][1] = cosx;
				temp_transform[1][2] = sinx;

				temp_transform[2][1] = -sinx;
				temp_transform[2][2] = cosx;

				temp_transform[3][3] = 1;

				temp_transform_xy[0][0] = cosy;
				temp_transform_xy[0][2] = siny;

				temp_transform_xy[1][1] = 1;

				temp_transform_xy[2][0] = -siny;
				temp_transform_xy[2][2] = cosy;

				temp_transform_xy[3][3] = 1;

				temp_transform = temp_transform_xy * temp_transform;
			}
		}
		else if (m_nAction == ID_ACTION_TRANSLATE){
			if (m_nAxis == ID_AXIS_X){
				temp_transform[0][0] = 1;
				temp_transform[3][0] = (double)diff_x / m_WindowWidth;

				temp_transform[1][1] = 1;

				temp_transform[2][2] = 1;

				temp_transform[3][3] = 1;
			}
			if (m_nAxis == ID_AXIS_Y){
				temp_transform[0][0] = 1;

				temp_transform[1][1] = 1;
				temp_transform[3][1] = (double)diff_y / m_WindowHeight;

				temp_transform[2][2] = 1;

				temp_transform[3][3] = 1;
			}
			if (m_nAxis == ID_AXIS_Z){
				temp_transform[0][0] = 1;

				temp_transform[1][1] = 1;

				temp_transform[2][2] = 1;
				temp_transform[3][2] = (double)diff_x / m_WindowWidth;

				temp_transform[3][3] = 1;
			}
			if (m_nAxis == ID_AXIS_XY){
				temp_transform[0][0] = 1;
				temp_transform[3][0] = (double)diff_x / m_WindowWidth;

				temp_transform[1][1] = 1;
				temp_transform[3][1] = (double)diff_y / m_WindowHeight;

				temp_transform[2][2] = 1;

				temp_transform[3][3] = 1;
			}
		}
		else if (m_nAction == ID_ACTION_SCALE){
			if (m_nAxis == ID_AXIS_X){
				temp_transform[0][0] = 1 + (double)diff_x / m_WindowWidth;

				temp_transform[1][1] = 1;

				temp_transform[2][2] = 1;

				temp_transform[3][3] = 1;
			}
			if (m_nAxis == ID_AXIS_Y){
				temp_transform[0][0] = 1;

				temp_transform[1][1] = 1 + (double)diff_y / m_WindowHeight;

				temp_transform[2][2] = 1;

				temp_transform[3][3] = 1;
			}
			if (m_nAxis == ID_AXIS_Z){
				temp_transform[0][0] = 1;

				temp_transform[1][1] = 1;

				temp_transform[2][2] = 1 + (double)diff_x / m_WindowWidth;

				temp_transform[3][3] = 1;
			}
			if (m_nAxis == ID_AXIS_XY){
				temp_transform[0][0] = 1 + (double)diff_x / m_WindowWidth;

				temp_transform[1][1] = 1 + (double)diff_y / m_WindowHeight;

				temp_transform[2][2] = 1;

				temp_transform[3][3] = 1;
			}
		}
		else {
			temp_transform[0][0] = 1;

			temp_transform[1][1] = 1;

			temp_transform[2][2] = 1;

			temp_transform[3][3] = 1;
		}
		m_tarnsform = temp_transform;

		for (unsigned int m = 0; m < models.size(); m++){
			if (m_object_space_trans)
				models[m].obj_coord_trans = m_tarnsform * models[m].obj_coord_trans;
			else
				models[m].obj_coord_trans = models[m].obj_coord_trans * m_tarnsform;
		}

		RenderScene();

		m_mouse_xpos = xPos;
		m_mouse_ypos = yPos;
	}
	else if (wparam == MK_LBUTTON){
		past_pressed = true;
		m_mouse_xpos = xPos;
		m_mouse_ypos = yPos;
	}
	else{
		past_pressed = false;
	}

	return 0;
};

void CCGWorkView::DrawLine(COLORREF *arr, vec4 &p1, vec4 &p2, COLORREF color){
	// TODO:
	// use default line color

	// algorithm vars
	int x1, x2, y1, y2, dx, dy, d;
	int north_er, north_west_er, west_er, south_west_er, south_er, south_east_er, east_er, north_east_er;

	// draw location variables
	int x, y;

	// midpoint algorithm

	x1 = static_cast<int>(p1.x < p2.x ? p1.x / p1.p : p2.x / p2.p);
	x2 = static_cast<int>(p1.x < p2.x ? p2.x / p2.p : p1.x / p1.p);

	if (x1 != x2){
		y1 = static_cast<int>(p1.x < p2.x ? p1.y / p1.p : p2.y / p2.p);
		y2 = static_cast<int>(p1.x < p2.x ? p2.y / p2.p : p1.y / p1.p);
	}
	else{
		y1 = static_cast<int>(min(p1.y, p2.y));
		y2 = static_cast<int>(max(p1.y, p2.y));
	}


	dx = x2 - x1;
	dy = y2 - y1;

	x = x1;
	y = y1;

	east_er = 2 * dy;
	north_east_er = 2 * (dy - dx);
	north_er = -2 * dx;
	north_west_er = -2 * (dy - dx);
	west_er = -2 * dy;
	south_west_er = 2 * dx - 2 * dy;
	south_er = 2 * dx;
	south_east_er = 2 * dx + 2 * dy;
		

	if (IN_RANGE(x, y))
		arr[y + m_WindowWidth * x] = color;
	// select the correct midpoint algorithm (direction and incline)
	if (dx == 0){ // horizontal y line or line in z direction only
		//move in positive y direction only
			
		while (y < y2){
			y = y + 1;	
			if (IN_RANGE(x, y))
				arr[y + m_WindowWidth * x] = color;
		}
		return;
	}

	double incline = (double)dy / (double)dx;

	if (incline > 1){
		d = dy - 2 * dx; // try to move in positive y direction only
		while (y < y2){
			if (d > 0){
				d = d + north_er;
				y = y + 1;
			}
			else{
				d = d + north_east_er;
				x = x + 1;
				y = y + 1;
			}
			if (IN_RANGE(x, y))
				arr[y + m_WindowWidth * x] = color;
		}
	}
	else if (0 < incline && incline <= 1)
	{
		d = 2 * dy - dx; // try to move in positive x direction only, possibly positive y
		while (x < x2){
			if (d < 0){
				d = d + east_er;
				x = x + 1;
			}
			else{
				d = d + north_east_er;
				x = x + 1;
				y = y + 1;
			}
			if (IN_RANGE(x, y))
				arr[y + m_WindowWidth * x] = color;
		}
	}
	else if (-1 < incline && incline <= 0){
		d = dx + 2 * dy; // try to move in positive x direction only, possibly negative y
		while (x < x2){
			if (d > 0){
				d = d + east_er;
				x = x + 1;
			}
			else{
				d = d + south_east_er;
				x = x + 1;
				y = y - 1;
			}
			if (IN_RANGE(x, y))
				arr[y + m_WindowWidth * x] = color;
		}
	}
	else if (incline <= -1){ // condition unneccessary, exists to make conditions clear
		d = 2 * dx + dy; // try to move in negative y direction only
		while (y > y2){
			if (d < 0){
				d = d + south_er;
				y = y - 1;
			}
			else{
				d = d + south_east_er;
				x = x + 1;
				y = y - 1;
			}
			if (IN_RANGE(x, y))
				arr[y + m_WindowWidth * x] = color;
		}
	}

	return;
}

void CCGWorkView::DrawBoundBox(COLORREF *arr, model &model, COLORREF color){

	mat4 cur_transform = model.obj_coord_trans * model.view_space_trans * m_screen_space_trans;

	double minx = model.min_vec.x;
	double miny = model.min_vec.y;
	double minz = model.min_vec.z;

	double maxx = model.max_vec.x;
	double maxy = model.max_vec.y;
	double maxz = model.max_vec.z;

	vec4 xmin_ymin_zmin(minx, miny, minz, 1.0);
	vec4 xmin_ymin_zmax(minx, miny, maxz, 1.0);

	vec4 xmin_ymax_zmin(minx, maxy, minz, 1.0);
	vec4 xmin_ymax_zmax(minx, maxy, maxz, 1.0);

	vec4 xmax_ymin_zmin(maxx, miny, minz, 1.0);
	vec4 xmax_ymin_zmax(maxx, miny, maxz, 1.0);

	vec4 xmax_ymax_zmin(maxx, maxy, minz, 1.0);
	vec4 xmax_ymax_zmax(maxx, maxy, maxz, 1.0);

	// zmin rectangle first
	DrawLine(arr, xmin_ymin_zmin * cur_transform, xmin_ymax_zmin * cur_transform, color);
	DrawLine(arr, xmin_ymax_zmin * cur_transform, xmax_ymax_zmin * cur_transform, color);
	DrawLine(arr, xmax_ymax_zmin * cur_transform, xmax_ymin_zmin * cur_transform, color);
	DrawLine(arr, xmax_ymin_zmin * cur_transform, xmin_ymin_zmin * cur_transform, color);

	// zmax rectangle second
	DrawLine(arr, xmin_ymin_zmax * cur_transform, xmin_ymax_zmax * cur_transform, color);
	DrawLine(arr, xmin_ymax_zmax * cur_transform, xmax_ymax_zmax * cur_transform, color);
	DrawLine(arr, xmax_ymax_zmax * cur_transform, xmax_ymin_zmax * cur_transform, color);
	DrawLine(arr, xmax_ymin_zmax * cur_transform, xmin_ymin_zmax * cur_transform, color);

	// connect the two rectangles next
	DrawLine(arr, xmin_ymin_zmin * cur_transform, xmin_ymin_zmax * cur_transform, color);
	DrawLine(arr, xmin_ymax_zmin * cur_transform, xmin_ymax_zmax * cur_transform, color);
	DrawLine(arr, xmax_ymin_zmin * cur_transform, xmax_ymin_zmax * cur_transform, color);
	DrawLine(arr, xmax_ymax_zmin * cur_transform, xmax_ymax_zmax * cur_transform, color);
}

void CCGWorkView::RenderScene() {
	
	std::fill_n(m_screen, m_WindowWidth * m_WindowHeight, m_background_color); //TODO background color select
	vec4 p1, p2;
	line cur_line;
	polygon cur_polygon;
	mat4 cur_transform;
	for (unsigned int m = 0; m < models.size(); m++){
		cur_transform = models[m].obj_coord_trans * models[m].view_space_trans * m_screen_space_trans;
		for (unsigned int pnt = 0; pnt < models[m].points_list.size(); pnt++){
			p1 = models[m].points_list[pnt].p_a * cur_transform;
			p2 = models[m].points_list[pnt].p_b * cur_transform;
			//if (models[m].color == m_background_color )
			//	DrawLine(m_screen, p1, p2, models[m].color ^ 0x00ffffff); //inverse the color of the object if it is the same as the screen
			//else
			DrawLine(m_screen, p1, p2, models[m].color);
		}
		if (m_bound_box){
			DrawBoundBox(m_screen, models[m], m_boundbox_color);
		}
	}


	m_map = CreateBitmap(m_WindowWidth,		 // width
		m_WindowHeight,		 // height
		1,			 // Color Planes, unfortanutelly don't know what is it actually. Let it be 1
		8 * 4,		 // Size of memory for one pixel in bits (in win32 4 bytes = 4*8 bits)
		(void*)m_screen); // pointer to array

	
	//HDC src = CreateCompatibleDC(m_pDC->GetSafeHdc()); // hdc - Device context for window
	SelectObject(m_hDC, m_map); // Inserting picture into our temp HDC

	// Copy image from temp HDC to window
	BitBlt(m_pDC->GetSafeHdc(), // Destination
		0,  // x and
		0,  // y - upper-left corner of place, where we'd like to copy
		m_WindowWidth, // width of the region
		m_WindowHeight, // height
		m_hDC, // source
		0,   // x and
		0,   // y of upper left corner  of part of the source, from where we'd like to copy
		SRCCOPY); // Defined DWORD to juct copy pixels. Watch more on msdn;

	DeleteObject(m_map);

	return;
}


void CCGWorkView::OnFileLoad() 
{
	TCHAR szFilters[] = _T ("IRIT Data Files (*.itd)|*.itd|All Files (*.*)|*.*||");

	CFileDialog dlg(TRUE, _T("itd"), _T("*.itd"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY ,szFilters);

	if (dlg.DoModal () == IDOK) {
		m_strItdFileName = dlg.GetPathName();		// Full path and filename
		PngWrapper p;
		CGSkelProcessIritDataFiles(m_strItdFileName, 1);
		// Open the file and read it.
		// Your code here...


		Invalidate();	// force a WM_PAINT for drawing.
	} 

}





// VIEW HANDLERS ///////////////////////////////////////////

// Note: that all the following Message Handlers act in a similar way.
// Each control or command has two functions associated with it.

void CCGWorkView::OnViewOrthographic() 
{
	m_nView = ID_VIEW_ORTHOGRAPHIC;
	m_bIsPerspective = false;
	Invalidate();		// redraw using the new view.
}

void CCGWorkView::OnUpdateViewOrthographic(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nView == ID_VIEW_ORTHOGRAPHIC);
}

void CCGWorkView::OnViewPerspective() 
{
	m_nView = ID_VIEW_PERSPECTIVE;
	m_bIsPerspective = true;
	Invalidate();
}

void CCGWorkView::OnUpdateViewPerspective(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nView == ID_VIEW_PERSPECTIVE);
}

// ACTION HANDLERS ///////////////////////////////////////////

void CCGWorkView::OnActionRotate() 
{
	m_nAction = ID_ACTION_ROTATE;
}

void CCGWorkView::OnUpdateActionRotate(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAction == ID_ACTION_ROTATE);
}

void CCGWorkView::OnActionTranslate() 
{
	m_nAction = ID_ACTION_TRANSLATE;
}

void CCGWorkView::OnUpdateActionTranslate(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAction == ID_ACTION_TRANSLATE);
}

void CCGWorkView::OnActionScale() 
{
	m_nAction = ID_ACTION_SCALE;
}

void CCGWorkView::OnUpdateActionScale(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAction == ID_ACTION_SCALE);
}

void CCGWorkView::OnActionResetView()
{
	mat4 reset_transform;
	reset_transform[0][0] = 1;
	reset_transform[1][1] = 1;
	reset_transform[2][2] = 1;
	reset_transform[3][3] = 1;

	m_tarnsform = reset_transform;
	for (unsigned int m = 0; m < models.size(); m++)
		models[m].obj_coord_trans = reset_transform;
	
	RenderScene();
}


// OBJ/CAMERA VIEW TOGGLE //////////////////////////////////

void CCGWorkView::OnActionToggleView()
{
	m_object_space_trans = !m_object_space_trans;
}



void CCGWorkView::OnUpdateActionToggleView(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_object_space_trans);
}

// AXIS HANDLERS ///////////////////////////////////////////


// Gets calles when the X button is pressed or when the Axis->X menu is selected.
// The only thing we do here is set the ChildView member variable m_nAxis to the 
// selected axis.
void CCGWorkView::OnAxisX() 
{
	m_nAxis = ID_AXIS_X;
}

// Gets called when windows has to repaint either the X button or the Axis pop up menu.
// The control is responsible for its redrawing.
// It sets itself disabled when the action is a Scale action.
// It sets itself Checked if the current axis is the X axis.
void CCGWorkView::OnUpdateAxisX(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_X);
}


void CCGWorkView::OnAxisY() 
{
	m_nAxis = ID_AXIS_Y;
}

void CCGWorkView::OnUpdateAxisY(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_Y);
}


void CCGWorkView::OnAxisZ() 
{
	m_nAxis = ID_AXIS_Z;
}

void CCGWorkView::OnUpdateAxisZ(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_Z);
}

void CCGWorkView::OnAxisXY()
{
	m_nAxis = ID_AXIS_XY;
}

void CCGWorkView::OnUpdateAxisXY(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_XY);
}



// OPTIONS HANDLERS ///////////////////////////////////////////

void CCGWorkView::OnWriteframeColor()
{
	ColorSelectionDialog dlg(m_color_wireframe, m_boundbox_color, m_background_color);
	if (dlg.DoModal() == IDOK){

		m_color_wireframe = RGB(GetBValue(dlg.wireframe_color), GetGValue(dlg.wireframe_color), GetRValue(dlg.wireframe_color));
		m_boundbox_color = RGB(GetBValue(dlg.boundbox_color), GetGValue(dlg.boundbox_color), GetRValue(dlg.boundbox_color));
		m_background_color = RGB(GetBValue(dlg.background_color), GetGValue(dlg.background_color), GetRValue(dlg.background_color));

		for (unsigned int m = 0; m < models.size(); m++){
			models[m].color = m_color_wireframe;
		}
		Invalidate();
	}
}

void CCGWorkView::OnBoundBox()
{
	m_bound_box = !m_bound_box;
	Invalidate();
}

void CCGWorkView::OnUpdateBoundBox(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bound_box);
}


void CCGWorkView::OnOptionMouseSensetivity(){
	MouseSensetiveDialog dlg;
	dlg.DoModal();

	m_mouse_sensetivity = dlg.m_mouse_sensetivity;
}

// LIGHT SHADING HANDLERS ///////////////////////////////////////////

void CCGWorkView::OnLightShadingFlat() 
{
	m_nLightShading = ID_LIGHT_SHADING_FLAT;
}

void CCGWorkView::OnUpdateLightShadingFlat(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLightShading == ID_LIGHT_SHADING_FLAT);
}


void CCGWorkView::OnLightShadingGouraud() 
{
	m_nLightShading = ID_LIGHT_SHADING_GOURAUD;
}

void CCGWorkView::OnUpdateLightShadingGouraud(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLightShading == ID_LIGHT_SHADING_GOURAUD);
}

// LIGHT SETUP HANDLER ///////////////////////////////////////////

void CCGWorkView::OnLightConstants() 
{
	CLightDialog dlg;

	for (int id=LIGHT_ID_1;id<MAX_LIGHT;id++)
	{	    
	    dlg.SetDialogData((LightID)id,m_lights[id]);
	}
	dlg.SetDialogData(LIGHT_ID_AMBIENT,m_ambientLight);

	if (dlg.DoModal() == IDOK) 
	{
	    for (int id=LIGHT_ID_1;id<MAX_LIGHT;id++)
	    {
		m_lights[id] = dlg.GetDialogData((LightID)id);
	    }
	    m_ambientLight = dlg.GetDialogData(LIGHT_ID_AMBIENT);
	}	
	Invalidate();
}