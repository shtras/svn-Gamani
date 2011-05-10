#pragma once
#include "Camera.h"
#include "AstralBody.h"
#include "SkyBox.h"

using namespace std;

int LoadBitmap11(const char *filename);

class Renderer
{
public:
  static Renderer& getInstance();
  bool init(HINSTANCE& hInstance);
  void render();
  void renderEnd();
  HWND& getHwnd() {return hWnd_;}
  int getWidth() {return width_;}
  int getHeight() {return height_;}
  Camera& getCamera() {return *camera_;}
  void changeRenderList(vector<AstralBody*>* newList);
  void textOutNoMove(double x, double y, double z, char* format, ...);

  void textOut(double x, double y, double z, char* format, ...);
  void requestViewPort(int left, int top, int width, int height);
  void resetViewPort();
  CString formatDistance (double dist);
  void resize (int width, int height);
private:
  Renderer();
  ~Renderer();

  bool initWindow();
  bool initOpenGL();

  void testCase();

  Camera* camera_;
  bool init_;
  HWND hWnd_;
  HINSTANCE* hInstance_;
  int height_;
  int width_;

  SkyBox* skyBox_;

  vector<AstralBody*>* renderList_;
};
