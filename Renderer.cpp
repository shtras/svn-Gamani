#include "StdAfx.h"
#include "Gamani.h"
#include "Renderer.h"

int LoadBitmap11(const char *filename)
{
  FILE * file;
  char temp;
  long i;

  unsigned char* infoheader_data;

  BITMAPINFOHEADER infoheader;

  GLuint num_texture;

  if( (file = fopen(filename, "rb"))==NULL) return (-1); // Open the file for reading

  fseek(file, 18, SEEK_CUR);  /* start reading width & height */
  fread(&infoheader.biWidth, sizeof(int), 1, file);

  fread(&infoheader.biHeight, sizeof(int), 1, file);

  fread(&infoheader.biPlanes, sizeof(short int), 1, file);
  if (infoheader.biPlanes != 1) {
    printf("Planes from %s is not 1: %u\n", filename, infoheader.biPlanes);
    return 0;
  }

  // read the bpp
  fread(&infoheader.biBitCount, sizeof(unsigned short int), 1, file);
  if (infoheader.biBitCount != 24) {
    printf("Bpp from %s is not 24: %d\n", filename, infoheader.biBitCount);
    return 0;
  }

  fseek(file, 24, SEEK_CUR);

  // read the data
  if(infoheader.biWidth<0){
    infoheader.biWidth = -infoheader.biWidth;
  }
  if(infoheader.biHeight<0){
    infoheader.biHeight = -infoheader.biHeight;
  }
  infoheader_data = (unsigned char *) malloc(infoheader.biWidth * infoheader.biHeight * 3);
  if (infoheader_data == NULL) {
    printf("Error allocating memory for color-corrected image data\n");
    return 0;
  }

  if ((i = fread(infoheader_data, infoheader.biWidth * infoheader.biHeight * 3, 1, file)) != 1) {
    printf("Error reading image data from %s.\n", filename);
    return 0;
  }

  for (i=0; i<(infoheader.biWidth * infoheader.biHeight * 3); i+=3) { // reverse all of the colors. (bgr -> rgb)
    temp = infoheader_data[i];
    infoheader_data[i] = infoheader_data[i+2];
    infoheader_data[i+2] = temp;
  }


  fclose(file); // Closes the file stream

  glGenTextures(1, &num_texture);
  glBindTexture(GL_TEXTURE_2D, num_texture); // Bind the ID texture specified by the 2nd parameter

  // The next commands sets the texture parameters
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // If the u,v coordinates overflow the range 0,1 the image is repeated
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // The magnification function ("linear" produces better results)
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); //The minifying function

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  // Finally we define the 2d texture
  glTexImage2D(GL_TEXTURE_2D, 0, 3, infoheader.biWidth, infoheader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, infoheader_data);

  // And create 2d mipmaps for the minifying function
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, infoheader.biWidth, infoheader.biHeight, GL_RGB, GL_UNSIGNED_BYTE, infoheader_data);

  free(infoheader_data); // Free the memory we used to load the texture

  return (num_texture); // Returns the current texture OpenGL ID
}


Renderer::Renderer():init_(false),hInstance_(NULL),camera_(new Camera()), renderList_(NULL)
{
}

Renderer::~Renderer()
{
  delete camera_;
  delete skyBox_;
}

Renderer& Renderer::getInstance()
{
  static Renderer instance;
  return instance;
}

LRESULT CALLBACK MainLoop(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  }
  Gamani::getInstance().handleMessage(message, wParam, lParam);
  return DefWindowProc(hWnd, message, wParam, lParam);
}

bool Renderer::init(HINSTANCE& hInstance)
{
  width_ = 1024;
  height_ = 768;
  hWnd_ = 0;
  hInstance_ = &hInstance;
  bool res = initWindow();
  if (!res) {
    return false;
  }
  res = initOpenGL();
  if (!res) {
    return false;
  }
  init_ = true;
  skyBox_ = new SkyBox();
  skyBox_->initTextures();
  return true;
}

void Renderer::changeRenderList(vector<AstralBody*>* newList)
{
  renderList_ = newList;
}

bool Renderer::initWindow()
{
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_DBLCLKS;
  wcex.lpfnWndProc = (WNDPROC)MainLoop;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = *hInstance_;
  wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(NULL,IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = TEXT("Gamani soft presents");
  wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  RegisterClassEx(&wcex);
  RECT rect = {0,0,width_,height_};
  AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
  hWnd_ = CreateWindowEx(NULL, wcex.lpszClassName, wcex.lpszClassName, 
    WS_OVERLAPPEDWINDOW|WS_VISIBLE, 0, 0, width_, height_, NULL, NULL, *hInstance_, NULL);
  DWORD Error = GetLastError();
  if(!hWnd_) {
    return false;
  }
  ShowWindow(hWnd_,SW_SHOW);
  UpdateWindow(hWnd_);

  RECT actualRect;
  GetClientRect(hWnd_,&actualRect);

  width_ = actualRect.right;
  height_ = actualRect.bottom;

  return true;
}

bool Renderer::initOpenGL()
{
  HDC hDC = GetDC(hWnd_);

  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory( &pfd, sizeof( pfd ) );
  pfd.nSize = sizeof( pfd );
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
    PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  pfd.iLayerType = PFD_MAIN_PLANE;
  int iFormat = ChoosePixelFormat( hDC, &pfd );
  SetPixelFormat( hDC, iFormat, &pfd );
  HGLRC hRC = wglCreateContext(hDC);
  if (!hRC) {
    return false;
  }

  if (!wglMakeCurrent(hDC, hRC)) {
    return false;
  }

  GLenum res = glewInit();
  const GLubyte* bbb = glewGetErrorString(res);
  if (res != GLEW_OK) {
    return false;
  }


  glViewport(0, 0, width_, height_);
  camera_->setAspect((double)width_/(double)height_);

  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glClearDepth( 1.0f );
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glShadeModel (GL_SMOOTH);
  glEnable(GL_VERTEX_ARRAY);
  glEnable(GL_DITHER);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);
  GLfloat light_position[] = { 0, 1, 0, 1};
  GLfloat light_color[] = { 1, 1, 1, 1.0f };
  GLfloat ambient_light_color[] = { 0, 0, 0, 1.0f };
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_color);
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light_color);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light_color);

  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   //glMatrixMode(GL_PROJECTION);
   ////gluPerspective(45, (double)width_ / (double)height_, 0.1, 10);
   //double aspect = (double)width_ / (double)height_;
   //glOrtho(-aspect,aspect,-1,1,0.5,20);
   //gluLookAt(0,0,5,0,0,0,0,1,0);
  glMatrixMode(GL_MODELVIEW);

  GLenum err = glGetError();
  if (err != 0) {
    return false;
  }
  return true;
}

void Renderer::resize(int width, int height)
{
  width_ = width;
  height_ = height;
  glViewport(0, 0, width_, height_);
  camera_->setAspect((double)width_/(double)height_);
}

void Renderer::requestViewPort(int left, int top, int width, int height)
{
  glViewport(left, top-height, width, top);
}

void Renderer::resetViewPort()
{
  glViewport(0, 0, width_, height_);
}

GLvoid *font_style = GLUT_BITMAP_HELVETICA_12;

void Renderer::textOutNoMove(double x, double y, double z, char* format, ...)
{
  va_list args;   //  Variable argument list
  int len;        // String length
  int i;          //  Iterator
  char * text;    // Text
  va_start(args, format);
  len = _vscprintf(format, args) + 1;
  text = (char*)malloc(len * sizeof(char));
  vsprintf(text, format, args);
  va_end(args);

  glDisable(GL_LIGHTING);


  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(x,y,z);

  glRasterPos3f (0, 0, 0);
  for (i = 0; text[i] != '\0'; i++) {
    glutBitmapCharacter(font_style, text[i]);
  }
  free(text);

  glPopMatrix();

  glEnable(GL_LIGHTING);
}

void Renderer::textOut(double x, double y, double z, char* format, ...)
{
  va_list args;   //  Variable argument list
  int len;        // String length
  int i;          //  Iterator
  char * text;    // Text
  va_start(args, format);
  len = _vscprintf(format, args) + 1;
  text = (char*)malloc(len * sizeof(char));
  vsprintf(text, format, args);
  va_end(args);
 
  glDisable(GL_LIGHTING);
 
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(x,y,z);
 
  glColor3f(1,1,1);
  glRasterPos3f (0, 0, 0);
  for (i = 0; text[i] != '\0'; i++) {
    glutBitmapCharacter(font_style, text[i]);
  }
  free(text);
 
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glEnable(GL_LIGHTING);
}

void Renderer::render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  camera_->position();
  skyBox_->draw();
  //camera_->applyZoom();
  //testCase();
  if (renderList_) {
    //list<Renderable*>::iterator itr = renderList_->begin();
    //for (; itr != renderList_->end(); ++itr) {
    for (unsigned int i=0; i<renderList_->size(); ++i) {
      Renderable* object = /**itr*/(*renderList_)[i];
      object->render();
    }
  }
  camera_->test();

}

void Renderer::renderEnd()
{
  glFlush();
  SwapBuffers(GetDC(hWnd_));
}

void Renderer::testCase()
{
  glRotatef(90, 1, 0, 0);
  glColor3d(1,1,1);
  glutSolidCone(1,0.5,10,2);
}

CString Renderer::formatDistance(double dist)
{
  double km = dist * 1000.0;
  double AU = km / 149598000.0;
  double ly = km / 9.4605284e15;
  double m = km * 1000.0;
  if (ly > 0.1) {
    return CString(ly) + " LY";
  }
  if (AU > 0.1) {
    return CString(AU) + " AU";
  }
  if (km > 1) {
    return CString(km) + " km";
  }
  return CString(m) + " m";
}
