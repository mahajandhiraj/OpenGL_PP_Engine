//COMMON HEADER FILES
#include<windows.h>		//for win32API
#include<stdio.h>		//for file IO
#include<stdlib.h>		//for exit

//OpenGL HEADER FILES
#include<gl/glew.h>//This must be before gl/GL.h
#include<gl/GL.h>//from include directory, from 'gl' subdirectory inlcude 'GL.h', located at "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22000.0\um\gl"
#include "vmath.h"
#include "EngineMain.h"


using namespace vmath;
//MACROS
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//Link with OpenGL library
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"OpenGL32.lib")


// GLOBAL FUNCTIONS DECLARATION
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//GLOBAL VARIABLE DECLARATIONS
FILE* gpFile = NULL;                                                            //gpFIle: global pointer File

HWND ghwnd = NULL;                                                              //ghwnd : global handle
BOOL gbActive = FALSE;                                                          //by default window is not active
DWORD dwStyle = 0;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };                           //initializing struct WINDOWPLACEMENT //wpPrev: window pointer previous
BOOL gbFullscreen = FALSE;                                                      //gb: global bool

//OpenGL related global variables
HDC ghdc = NULL;
HGLRC ghrc = NULL;                                                              //give me rendering context(rc)



GLuint shaderProgramObject = 0;
enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR
};

GLfloat anglePyramid = 0.0f;
GLfloat angleCube = 0.0f;

//Pyramid
GLuint vao_Pyramid = 0;
GLuint vbo_position_Pyramid = 0;
GLuint vbo_color_Pyramid = 0;

//Cube
GLuint vao_Cube = 0;
GLuint vbo_position_Cube = 0;
GLuint vbo_color_Cube = 0;

GLuint mvpMatrixUniform = 0;

mat4 perspectiveProjectionMatrix;

// ENTRY POINT FUNCTION
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//FUNCTION DECLARATIONS
	int initialize(void);
	void uninitialize(void);
	void display(void);
	void update(void);

	// LOCAL VARIABLE DECLARATIONS
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("DAMWindow");
	int iResult = 0;                                                            //this is added for basecode
	BOOL bDone = FALSE;                                                         //this is added for for game loop

	//Variables to to draw window at center
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int windowWidth = 800;
	int windowHeight = 600;

    //code
	if(fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL,
            TEXT("Log.txt file cannot be opened"),
            TEXT("ERROR"),
            MB_OK | MB_ICONERROR);                                              //MB_OK|MB_ICONERROR -> there will be error symbol on left side of messagebox
		exit(0);                                                                //0 because this is not program error, system is not given file thats why used 0
	}
	fprintf(gpFile, "Program started successfully\n");

	// WNDCLASSEX INITIALIZATION
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW |CS_OWNDC;                         //CS_OWNDC : dont le
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// REGISTER WNDCLASSEX
	RegisterClassEx(&wndclass);

	// CREATE WINDOW
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,szAppName,
		TEXT("DHIRAJ A MAHAJAN"),
		WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE,
		screenWidth / 2 - windowWidth / 2,
		screenHeight / 2 - windowHeight / 2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);


	ghwnd = hwnd;

	//initilition
	iResult = initialize();
	if (iResult != 0)
	{
		MessageBox(hwnd,
            TEXT("initilize() failed"),
            TEXT("ERROR"),
            MB_OK | MB_ICONERROR);
		DestroyWindow(hwnd);
	}

	// SHOW THE WINDOW
	ShowWindow(hwnd, iCmdShow);                                                 //if we put SW_MAXIMIZE instead of iCmdShow, window will be opened maximized (not fullscreen)

	SetForegroundWindow(hwnd);                                                  //keep window to foreground if S_CLIPCHILDREN|WS_CLIPSIBLINGS caused some confusion to os wihch brought window to backside
	SetFocus(hwnd);                                                             //keep window highligted, it will send WM_SETFOXUS msg to wndproc internally

	//game loop
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActive == TRUE)                                               //render only when window is active
			{
				//to render write code here
				display();

				//update
				update();
			}

		}
	}

	//UNITILAZATION
	uninitialize();

	return((int)msg.wParam);
}

// CALLBACK FUNCTION
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//FUNCTION DECLARATION
	void ToggleFullscreen(void);
	void resize(int, int);//(width, height)

	// CODE
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActive = TRUE;
		break;

	case WM_KILLFOCUS:
		gbActive = FALSE;
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));                                 // lParam of WM_SIZE has width(bit 0 to 15) and height(bit 16 to 32)
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_ESCAPE:                                                         //VK_ESCAPE: virtual key escape (esc key , ASKII 27, we can use aski or hex value instead of VK_ESCAPE)
			DestroyWindow(hwnd);                                                //this function send WM_DESTROY message to hwnd window
			break;
		}
		break;

	case WM_CHAR:
		switch (LOWORD(wParam))
		{
		case 'F':
		case 'f':
			if (gbFullscreen == FALSE)
			{
				ToggleFullscreen();
				gbFullscreen = TRUE;
			}
			else
			{
				ToggleFullscreen();
				gbFullscreen = FALSE;
			}

			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:

		PostQuitMessage(0);
		break;

	default:
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
	//Local variable declarations
	MONITORINFO mi = { sizeof(MONITORINFO) };                                   //initializing struct MONITORINFO

	//code
	//1
	if (gbFullscreen == FALSE)
	{
		//A
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);                              // this funct give window's long type variable
												                                //GWL_STYLE : get window style from ghwnd
		if (dwStyle & WS_OVERLAPPEDWINDOW)                                      // to check if dwStyle contains WS_OVERLAPPEDWINDOW
		{
			//a, b, c
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
                                                                                //GetWindowPlacement(ghwnd, &wpPrev) : give windoow placement of ghwnd handle's window and give it back in wpPrev
                                                                                    //GetMonitorInfo(MonitorFromWindow()): MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY): to get handle of monitor to window ghwnd, MONITORINFOF_PRIMARY: this micro gives primary monitors info
                                                                                    //this call give handle to monitors hMonitor
                                                                                //&mi is where monitor info is stored and sent back
                                                                                //GetWindowPlacement & GetMonitorInfo have BOOL return value
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                                                                                //ghwnd: which window (style) we need to set
                                                                                //GWL_STYLE : what to set: style
                                                                                //dwStyle & ~WS_OVERLAPPEDWINDOW : how to set style (take dwStyle, find WS_OVERLAPPEDWINDOW in it and remove it)

				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
                                                                                // 7 paramters
                                                                                // ghwnd : which window position
                                                                                // HWND_TOP: after WS_OVERLAPPEDWINDOW is removed, this adds WS_OVERLAPPED
                                                                                // mi.rcMonitor: Monitor's left (rc here is RECT)
                                                                                // mi.rcMonitor.top: Monitor's top,
                                                                                // (mi.rcMonitor.right - mi.rcMonitor.left) :Monitor's width
                                                                                // (mi.rcMonitor.bottom - mi.rcMonitor.top) : Monitor's height
                                                                                // SWP_NOZORDER | SWP_FRAMECHANGED : swp: set window position
                                                                                    //SWP_NOZORDER: parent's z-order should not have impact on zorder of window
                                                                                    //SWP_FRAMECHANGED: this forces to reclaculate non-clinent area size :internally send message WM_NCCALCSIZE (window message non_client area calculate size)
			}
		}

		//B
		//hide cursor
		ShowCursor(FALSE);
	}
	else//2
	{
		//A
		SetWindowPlacement(ghwnd, &wpPrev);
		//wpPrev: prev state is saved in wpPrev
		//B
		SetWindowLong(ghwnd, GWL_STYLE , dwStyle| WS_OVERLAPPEDWINDOW);         //to insert WS_OVERLAPPEDWINDOW in dwStyle we use |


		//C: set window on top, actually it is on top because WS_OVERLAPPEDWINDOW but we are making sure that its on top (Z-order), beacuse SetWindowPos has higher priority than SetWindowPlacementb and SetWindowStyle
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_FRAMECHANGED);
                                                                                //0, 0, 0, 0  -> already window is placed at position SetWindowPlacement, because of position SetWindowPlacement ha shigher priority than SetWindowPos
                                                                                //SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_FRAMECHANGED : to
                                                                                //SWP_NOMOVE: while bringing window, dont move window
                                                                                //SWP_NOSIZE: while bringing window, dont change size
                                                                                //SWP_NOOWNERZORDER: even if parent Z order has been chnage dont chnage zorder os this window
                                                                                //SWP_NOZORDER: dont change windows zorder
                                                                                //SWP_FRAMECHANGED: this forces to reclaculate non-clinent area size :internally send message WM_NCCALCSIZE (window message non_client area calculate size)

		//D
		ShowCursor(TRUE);
	}

}

int initialize(void)
{
	//function declaration
	void resize(int, int);
	void printGLInfo(void);
	void uninitialize(void);

	//code
	PIXELFORMATDESCRIPTOR pfd;                                                  //to show pixel screen, how to show that , for that we need struct PIXELFORMATDESCRIPTOR
	int iPixelFormatIndex = 0;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));//make all members of struct pfd initilaize to 0

	//initiliazation of PIXELFORMATDESCRIPTOR
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);//size of struct
	pfd.nVersion = 1;                                                           //version of this struct
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
                                                                                //PFD_DRAW_TO_WINDOW://draw over window (we can draw on bitmap, printer, but we want to draw on window
                                                                                //PFD_SUPPORT_OPENGL://this tells OS to support OpenGL to do imemdiate mode rendering, which OS was doing earlier by retained mdoe rendering
                                                                                //PFD_DOUBLEBUFFER: use double buffer (concept will be explaine din future)
	pfd.iPixelType = PFD_TYPE_RGBA;                                             // which type to use to show pixel
	pfd.cColorBits = 32;                                                        //count of color bits i.e for RGBA total 32 , for each color 8 bits
	pfd.cRedBits = 8;                                                           //count of red bits = 8
	pfd.cGreenBits = 8;                                                         //count of green bits = 8
	pfd.cBlueBits = 8;                                                          //count of green bits = 8
	pfd.cAlphaBits = 8;                                                         //count of green bits = 8
	                                                                            //above 4 lines means, we tell OS to use pixel format type split as above four lines
	pfd.cDepthBits = 32;

	//STEP 2:get DC : DEVICE CONTEXT
	ghdc = GetDC(ghwnd);                                                        //we need DC which can paint using other methods other than WM_PAINT, earlier we use to get DC from beginPaint but this DC was only cabapble to draw using WM_PAINT
	if (ghdc == NULL)
	{
		fprintf(gpFile, "GetDC() failed !!!\n");
		return(-1);                                                             //this is failure thats why -1
	}

	//STEP 3:Tell OS, collect my pixel format descriptor(ghdc), try to match it closest pixelformatdescriptor OS has give its index to us in &pfd
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)                                                 // we should get iPixelFormatIndex as nonzero positive if succeeded
	{

		fprintf(gpFile, "ChoosePixelFormat() failed !!!\n");
		return(-2);                                                             //this is failure thats why (-)minus
	}

	//set obtained pixelFormat now
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) ==  FALSE)//SetPixelFormat return true on SUCEESS
                                                                                //ghdc:
                                                                                //iPixelFormatIndex:
                                                                                //pfd:
	{
		fprintf(gpFile, "SetPixelFormat() failed !!!\n");
		return(-3);                                                             //this is failure thats why (-)minus
	}

	//STEP 4: Tell windows graphics library(WGL) give me OpenGL compatible DC from this ghdc  (WGL is a briding API where drawing is moved from window to opengl)
	//create opngl context from device context
	ghrc = wglCreateContext(ghdc);                                              //function string from wgl are bridging apis, function starting from gl are opengl api
	if (ghrc == NULL)
	{
		fprintf(gpFile, "wglCreateContext() failed !!!\n");
		return(-4);
	}

	//STEP5: now ghdc will end its role and give control to ghrc to further drawing
	//Make rendering context xurrent
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		fprintf(gpFile, "wglMakeCurrent() failed !!!\n");
		return(-5);
	}

	//Initialize GLEW
	if (glewInit() != GLEW_OK)
	{
		fprintf(gpFile, "glewInit() failed to initialize !!!\n");
		return(-6);
	}

	//Print GL INFO
	printGLInfo();

	//----------------------------------------------------
	//Vertex shader
	const GLchar* vertexShaderSourceCode =
	"#version 460 core"                     \
	"\n"                                    \
	"in vec4 aPosition;"                    \
	"in vec4 aColor;"                       \
    "uniform mat4 uMVPMatrix;"              \
	"out vec4 oColor;"                      \
	"void main(void)"                       \
	"{"                                     \
	"gl_Position = aPosition;"              \
	"oColor = aColor;"                      \
    "gl_Position = uMVPMatrix* aPosition;"  \
	"}";

	//Create vertexShaderObject
	GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//give Vertex shader source code to openGL
	glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

	//Let openGL compile vertex shader
	glCompileShader(vertexShaderObject);

	//Check for compilation errors if any
	GLint status = 0;
	GLint infoLogLength= 0;
	GLchar* szInfoLog = NULL;

	//a: Get compile status
	glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
                                                                                //vertexShaderObject -> which shader
                                                                                // GL_COMPILE_STATUS  -> what is needed
                                                                                // &status  -> get status filled in status variable

	//b: If compilation error, get length of the compilation error info log
	if (status == GL_FALSE)
	{
		glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
                                                                                //vertexShaderObject -> which shader
                                                                                // GL_INFO_LOG_LENGTH  -> what is needed
                                                                                // &infoLogLength  -> get length filled in infoLogLength variable

		//c :Declare a string variable and allocate memory equal to above length
		if (infoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(infoLogLength);

			//d: Get compilation error info log into this error string
			if (szInfoLog != NULL)
			{
				glGetShaderInfoLog(vertexShaderObject, infoLogLength, NULL, szInfoLog);
                                                                                //vertexShaderObject : which shader
                                                                                //infoLogLength : max length of info log which we want to get
                                                                                //NULL: actual how much is the length of infolog we got, we dont want it so null
                                                                                //szInfoLog :where we want to fill the info log


				//e: Print the compilation error info log
				fprintf(gpFile, "Vertex Shader compilation error log : %s \n", szInfoLog);

				//f: Free the allocated memory
				free(szInfoLog);
				szInfoLog = NULL;

			}
		}

        //g: Uninitialize and exit
		uninitialize();
	}

	//Fragment Shader
	const GLchar* fragmentShaderSourceCode =
		"#version 460 core"     \
		"\n"                    \
		"in vec4 oColor;"       \
		"out vec4 FragColor;"   \
		"void main(void)"       \
		"{"                     \
		"FragColor = oColor;"   \
		"}";

	//Create fragmentShaderObject
	GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//give fragment shader source code to openGL
	glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

	//compile shader
	glCompileShader(fragmentShaderObject);

	//Check for compilation errors if any
	status = 0;
	infoLogLength = 0;
	szInfoLog = NULL;

	//a: Get compile status
	glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);

	//b: If compilation error, get length of the compilation error info log
	if (status == GL_FALSE)
	{
		glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);

		//c :Declare a string variable and allocate memory equal to above length
		if (infoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(infoLogLength);

			//d: Get compilation error info log into this error string
			if (szInfoLog != NULL)
			{
				glGetShaderInfoLog(fragmentShaderObject, infoLogLength, NULL, szInfoLog);

				//e: Print the compilation error info log
				fprintf(gpFile, "Fragment Shader compilation error log : %s \n", szInfoLog);

				//f: Free the allocated memory
				free(szInfoLog);
				szInfoLog = NULL;
			}
		}
        //g: Uninitialize and exit
		uninitialize();

	}



	//Shader program
	shaderProgramObject = glCreateProgram();

	//Attach both shader to this program
	glAttachShader(shaderProgramObject, vertexShaderObject);
	glAttachShader(shaderProgramObject, fragmentShaderObject);

	//Bind attribute location with the shader program object
	glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
	glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_COLOR, "aColor");

	//Link the shader program
	glLinkProgram(shaderProgramObject);

	//Do above shader compilation related steps for linking error
	//Check for compilation errors if any
	status = 0;
	infoLogLength = 0;
	szInfoLog = NULL;

	//a: Get compile status
	glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);

	//b: If compilation error, get length of the compilation error info log
	if (status == GL_FALSE)
	{
		glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);

		//c :Declare a string variable and allocate memory equal to above length
		if (infoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(infoLogLength);

			//d: Get compilation error info log into this error string
			if (szInfoLog != NULL)
			{
				glGetProgramInfoLog(shaderProgramObject, infoLogLength, NULL, szInfoLog);

				//e: Print the compilation error info log
				fprintf(gpFile, "Shader Program linking error log : %s \n", szInfoLog);

				//f: Free the allocated memory
				free(szInfoLog);
				szInfoLog = NULL;

			}
		}
        //g: Uninitialize and exit
		uninitialize();
	}

    //Get shader uniform location
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");


	//Declare and Position and color arrays
	const GLfloat pyramid_position[] =
	{
        // front
        0.0f,  1.0f,  0.0f, // front-top
        -1.0f, -1.0f,  1.0f, // front-left
        1.0f, -1.0f,  1.0f, // front-right

        // right
        0.0f,  1.0f,  0.0f, // right-top
        1.0f, -1.0f,  1.0f, // right-left
        1.0f, -1.0f, -1.0f, // right-right

        // back
        0.0f,  1.0f,  0.0f, // back-top
        1.0f, -1.0f, -1.0f, // back-left
        -1.0f, -1.0f, -1.0f, // back-right

        // left
        0.0f,  1.0f,  0.0f, // left-top
        -1.0f, -1.0f, -1.0f, // left-left
        -1.0f, -1.0f,  1.0f, // left-right
	};

	const GLfloat pyramid_color[] =
	{
        // front
        1.0f, 0.0f, 0.0f, // front-top
        0.0f, 1.0f, 0.0f, // front-left
        0.0f, 0.0f, 1.0f, // front-right

        // right
        1.0f, 0.0f, 0.0f, // right-top
        0.0f, 0.0f, 1.0f, // right-left
        0.0f, 1.0f, 0.0f, // right-right

        // back
        1.0f, 0.0f, 0.0f, // back-top
        0.0f, 1.0f, 0.0f, // back-left
        0.0f, 0.0f, 1.0f, // back-right

        // left
        1.0f, 0.0f, 0.0f, // left-top
        0.0f, 0.0f, 1.0f, // left-left
        0.0f, 1.0f, 0.0f, // left-right
	};

	const GLfloat cube_position[] =
	{
        // front
        1.0f,  1.0f,  1.0f, // top-right of front
        -1.0f,  1.0f,  1.0f, // top-left of front
        -1.0f, -1.0f,  1.0f, // bottom-left of front
        1.0f, -1.0f,  1.0f, // bottom-right of front

        // right
        1.0f,  1.0f, -1.0f, // top-right of right
        1.0f,  1.0f,  1.0f, // top-left of right
        1.0f, -1.0f,  1.0f, // bottom-left of right
        1.0f, -1.0f, -1.0f, // bottom-right of right

        // back
        1.0f,  1.0f, -1.0f, // top-right of back
        -1.0f,  1.0f, -1.0f, // top-left of back
        -1.0f, -1.0f, -1.0f, // bottom-left of back
        1.0f, -1.0f, -1.0f, // bottom-right of back

        // left
        -1.0f,  1.0f,  1.0f, // top-right of left
        -1.0f,  1.0f, -1.0f, // top-left of left
        -1.0f, -1.0f, -1.0f, // bottom-left of left
        -1.0f, -1.0f,  1.0f, // bottom-right of left

        // top
        1.0f,  1.0f, -1.0f, // top-right of top
        -1.0f,  1.0f, -1.0f, // top-left of top
        -1.0f,  1.0f,  1.0f, // bottom-left of top
        1.0f,  1.0f,  1.0f, // bottom-right of top

        // bottom
        1.0f, -1.0f,  1.0f, // top-right of bottom
        -1.0f, -1.0f,  1.0f, // top-left of bottom
        -1.0f, -1.0f, -1.0f, // bottom-left of bottom
        1.0f, -1.0f, -1.0f, // bottom-right of bottom
	};

    const GLfloat cube_color[] =
    {
        // front
        1.0f, 0.0f, 0.0f, // top-right of front
        1.0f, 0.0f, 0.0f, // top-left of front
        1.0f, 0.0f, 0.0f, // bottom-left of front
        1.0f, 0.0f, 0.0f, // bottom-right of front

        // right
        0.0f, 0.0f, 1.0f, // top-right of right
        0.0f, 0.0f, 1.0f, // top-left of right
        0.0f, 0.0f, 1.0f, // bottom-left of right
        0.0f, 0.0f, 1.0f, // bottom-right of right

        // back
        1.0f, 1.0f, 0.0f, // top-right of back
        1.0f, 1.0f, 0.0f, // top-left of back
        1.0f, 1.0f, 0.0f, // bottom-left of back
        1.0f, 1.0f, 0.0f, // bottom-right of back

        // left
        1.0f, 0.0f, 1.0f, // top-right of left
        1.0f, 0.0f, 1.0f, // top-left of left
        1.0f, 0.0f, 1.0f, // bottom-left of left
        1.0f, 0.0f, 1.0f, // bottom-right of left

        // top
        0.0f, 1.0f, 0.0f, // top-right of top
        0.0f, 1.0f, 0.0f, // top-left of top
        0.0f, 1.0f, 0.0f, // bottom-left of top
        0.0f, 1.0f, 0.0f, // bottom-right of top

        // bottom
        1.0f, 0.5f, 0.0f, // top-right of bottom
        1.0f, 0.5f, 0.0f, // top-left of bottom
        1.0f, 0.5f, 0.0f, // bottom-left of bottom
        1.0f, 0.5f, 0.0f, // bottom-right of bottom
    };


    //Pyramid----------------------
	//Create Vertex array Object: VAO
	glGenVertexArrays(1, &vao_Pyramid);

	//Bind with vao
	glBindVertexArray(vao_Pyramid);

	//VBO for position
	glGenBuffers(1, &vbo_position_Pyramid);

	//Bind with VBO of position
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_Pyramid);

	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_position), pyramid_position, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//VBO for color
	glGenBuffers(1, &vbo_color_Pyramid);

	//Bind with VBO of color
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color_Pyramid);

	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_color), pyramid_color, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Unbind with VAO
	glBindVertexArray(0);


    //Cube--------------------
	//Create Vertex array Object: VAO
	glGenVertexArrays(1, &vao_Cube);

	//Bind with vao
	glBindVertexArray(vao_Cube);

	//VBO for position
	glGenBuffers(1, &vbo_position_Cube);

	//Bind with VBO of position
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_Cube);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_position), cube_position, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

    //VBO for color
	glGenBuffers(1, &vbo_color_Cube);

	//Bind with VBO of color
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color_Cube);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_color), cube_color, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Unbind with VAO
	glBindVertexArray(0);

	//----------------------------------------------------

	//ENABLING DEPTH

	//below 3 lines are compulsary
	glClearDepth(1.0f);                                                         //use 1.0f to clear depth buffer, ie to set all bits in depth buffer to 1.0f
	glEnable(GL_DEPTH_TEST);                                                    //enable depth test out of 8 tests which rasterizer do to decide actual pixel from potential pixel
	glDepthFunc(GL_LEQUAL);
                                                                                // glDepthFunc means which function to use to test Depth.
                                                                                // GL_LEQUAL: Less than or equal. PASS those pixel whose z-cordinate value is less than or equal to 1.0f (1.0f value is given in glClearDepth(())


	//Set the clear color of window to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                                       //here every colro can be between 0.0 to 1.0, in total we can have 2^32 colors
                                                                                //alpha is 1.0  means non transparent, 0.0 is tranparent
                                                                                //This is first OpenGl function we have used

    //initialze perspectiveProjectionMatrix : good habit
    perspectiveProjectionMatrix = vmath::mat4::identity();
	resize(WIN_WIDTH, WIN_HEIGHT);
	return(0);
}

void printGLInfo(void)
{
	//variable declarations
	GLint num_extensions;
	GLint i;

	//code
    fprintf(gpFile, "************************************************************************\n");
	fprintf(gpFile, "OpenGL Vendor : %s\n", glGetString(GL_VENDOR));            //which company gfx card we have
	fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));        //driver version number
	fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));          //openGL version number
	fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));//Graphics library Shading language version
    fprintf(gpFile, "************************************************************************\n");

	//Listing of all supported extensions
	glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

	for (i = 0; i < num_extensions; i++)
	{
		fprintf(gpFile, "%s\n", glGetStringi(GL_EXTENSIONS, i));                //glGetStringi  -> give string based on interger provided in paramter
	}

     fprintf(gpFile, "************************************************************************\n");

}

void resize(int width, int height)
{
	//code
	if (height <= 0)
		height = 1;



	glViewport(0, 0, (GLsizei)width, (GLsizei)height);//where to focus on window to view. for now show entire window (viewport is nothing but binoculor)
	//type cast int to GLsizei. GLsizei is openGl's int
	//all this 3 line are concept explaine din future

    //set perpective projection matrix
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
            //45.0f  -> eye angle : field of view  -> this is fovY
	        //(GLfloat)width / (GLfloat)height  --> aspect ratio
	        //0.1f -->near
	        //100.0f -->far
}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//Use shaderProgamObject
	glUseProgram(shaderProgramObject);

    //Pyramid--------------
    //Transformations
    mat4 modelViewMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(-1.5f, 0.0f, -6.0f);
    mat4 rotationMatrix = mat4::identity();
    rotationMatrix = vmath::rotate(anglePyramid, 0.0f, 1.0f, 0.0f);
    modelViewMatrix = translationMatrix * rotationMatrix;
    mat4 modelViewProjectionMatrix = mat4::identity();
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;  //multiplication order is vvmimp

    //push above mvp into vertex shader's mvp uniform
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	//bind with VAO
	glBindVertexArray(vao_Pyramid);

	//Draw the geometry/Shapes/Models/Scene
	glDrawArrays(GL_TRIANGLES, 0, 12);

	//Unbind with VAO
	glBindVertexArray(0);

    //Cube--------------
    //Transformations
    modelViewMatrix = mat4::identity();
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(1.5f, 0.0f, -6.0f);
    mat4 scaleMatrix = mat4::identity();
    scaleMatrix = vmath::scale(0.75f, 0.75f, 0.75f);
    mat4 rotationMatrix1 = mat4::identity();
    rotationMatrix1 = vmath::rotate(angleCube, 1.0f, 0.0f, 0.0f);
    mat4 rotationMatrix2 = mat4::identity();
    rotationMatrix2 = vmath::rotate(angleCube, 0.0f, 1.0f, 0.0f);
    mat4 rotationMatrix3 = mat4::identity();
    rotationMatrix3 = vmath::rotate(angleCube, 0.0f, 0.0f, 1.0f);
    rotationMatrix = rotationMatrix1 * rotationMatrix2 * rotationMatrix3;
    modelViewMatrix = translationMatrix * scaleMatrix * rotationMatrix;
    modelViewProjectionMatrix = mat4::identity();
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;  //multiplication order is vvmimp

    //push above mvp into vertex shader's mvp uniform
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	//bind with VAO
	glBindVertexArray(vao_Cube);

	//Draw the geometry/Shapes/Models/Scene
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);                                        //from 0th line 4 lines
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);                                        //from 4rth line 4 lines
    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
                                                                                //above one line represents one surface

	//Unbind with VAO
	glBindVertexArray(0);

	//Unuse shaderprogramobject
	glUseProgram(0);


	SwapBuffers(ghdc);
}

void update(void)
{
	//code
	if (anglePyramid >= 360.0f)
		anglePyramid = anglePyramid - 360.0f;
	else
		anglePyramid = anglePyramid + 1.0f;

	if (angleCube >= 360.0f)
		angleCube = angleCube - 360.0f;
	else
		angleCube = angleCube + 1.0f;
}

void uninitialize(void)
{
	//function declarations
	void ToggleFullscreen(void);

	//code

	if (shaderProgramObject)
	{
		//Use shader program object
		glUseProgram(shaderProgramObject);

		//Get number of attached shaders
		GLint num_shaders = 0;
		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &num_shaders);

		//Create and Allocate array of shader objects according to number of attached shaders
		if (num_shaders > 0)
		{
			GLuint* pShaders = (GLuint*)malloc(num_shaders * sizeof(GLuint));

			if (pShaders != NULL)
			{
				//Get shader objects into this allocated array
				glGetAttachedShaders(shaderProgramObject, num_shaders, NULL, pShaders);

				//Detach and delete shader Objects iteratively
				for (GLint i = 0; i < num_shaders; i++)
				{
					glDetachShader(shaderProgramObject, pShaders[i]);
					glDeleteShader(pShaders[i]);
					pShaders[i] = 0;
				}


				free(pShaders);
				pShaders = NULL;
			}
		}

		//Unuse shaderProgramObject
		glUseProgram(0);

		//Delete shaderProgramObject
		glDeleteProgram(shaderProgramObject);
		shaderProgramObject = 0;
	}

    //Cube----------
    //Delete VBO of color
	if (vbo_color_Cube)
	{
		glDeleteBuffers(1, &vbo_color_Cube);
		vbo_color_Cube = 0;
	}
	//Delete VBO of Position
	if (vbo_position_Cube)
	{
		glDeleteBuffers(1, &vbo_position_Cube);
		vbo_position_Cube = 0;
	}

	//delete VAO
	if (vao_Cube)
	{
		glDeleteVertexArrays(1, &vao_Cube);
		vao_Cube = 0;
	}

    //tPyramid----------
	//Delete VBO of color
	if (vbo_color_Pyramid)
	{
		glDeleteBuffers(1, &vbo_color_Pyramid);
		vbo_color_Pyramid = 0;
	}

	//Delete VBO of Position
	if (vbo_position_Pyramid)
	{
		glDeleteBuffers(1, &vbo_position_Pyramid);
		vbo_position_Pyramid = 0;
	}

	//delete VAO
	if (vao_Pyramid)
	{
		glDeleteVertexArrays(1, &vao_Pyramid);
		vao_Pyramid = 0;
	}


	//if application is exiting in fullscreen
	if (gbFullscreen == TRUE)
	{
		ToggleFullscreen();
		gbFullscreen = FALSE;
	}

	//Make the OS's context hdc as current context
	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);                                             // if current context is ghrc, then make opengl context null that means tell os make ogl context null and move back to OS's context: hdc
	}

	//delete rendering context
	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	//release the hdc also
	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);//ghwnd: which windows context to release
		ghdc = NULL;
	}

	//destroy window
	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}

	//close the log file
	if (gpFile)
	{
        fprintf(gpFile, "************************************************************************\n");
		fprintf(gpFile, "Program Ended SuccessFully\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}


