#ifndef WATERFFTVISU_H
#define WATERFFTVISU_H

#include <GL/glew.h>
#include <QWidget>
#include <QGLShaderProgram>
#include <qmessagebox.h>
#include <QTime>
#include <QKeyEvent>
#include <math.h>
#include "functions.h"
#include <fftw3.h>
#include <complex>
#include "camera.h"
#include "OceanSurface.h"
#include <vector>

#include "ui_waterfftvisu.h"
#include "ui_DialogParameters.h"

using namespace std;

#define M_PI       3.14159265358979323846
#define WIDTH 128
#define HEIGHT 128

//Visualization of an implementation of the FFT Ocean water method by Jerry Tessendorf
// author : Bertrand Rix
//http://graphics.ucsd.edu/courses/rendering/2005/jdewall/tessendorf.pdf

class waterFFTvisu : public QGLWidget
{
  Q_OBJECT

public:
  waterFFTvisu(QWidget *parent);
  ~waterFFTvisu();
  QGLShaderProgram * loadShaders(QString vertexFile, QString fragmentFile);
  void buildScreenQuad();
  void drawScreenQuad(GLuint vertexLocation, GLuint texCoordLocation);
  GLuint createTexture(GLint width, GLint height, void *texture, GLint internalFormat, GLenum format, GLenum type, bool clamp);
  void loadTexture(QString textureName,GLuint *textureUnit, bool wrap) ;

private:
  Ui::waterFFTvisuClass ui;
  Ui_Form dialog_parameters;

  QWidget *parameterBox;

  //HeightField Parameters

  float HFIELD_SIZE; //cell size
  float WINDX;
  float WINDZ;
  float WAVEHEIGHTFACTOR; // A in the formula
  float GRAVIT_CST;
  float DISPLACE_LAMBDA;

  //Screen Aligned quad related stuff
  GLuint ScAquad_vertexbuffer;
  GLuint ScAquad_TexCoordBuffer;
  int nbVertex;
  GLuint vboPlan;
  GLuint vboIndexPlan;
  GLuint vboTexCoord;

  float t;
  QTime theTime;

  Camera *cam;
  int winX, winY;
  bool freeCamera;
  GLuint fbo;

  //bases vectors to compute h(X,t)

  float *waveVectorLength;
  float *waveFrequency;
  QVector2D *waveVector;

  // h~0(k) amplitude and phase computed with phillips spectrum
  complex<float> *h0_tilde;

  // h~0(-k) amplitude and phase computed with phillips spectrum
  complex<float>* h0_tilde2;

  //field of frequency amplitude
  //h(k,t) = h~0(k) * exp(i*w(k)*t) + h~0(-k) * exp(-i*w(k)*t)
  complex<float> *h;

  GLfloat *heightmapData;
  GLuint heightmap;

  GLfloat *displacementX;
  GLuint dispX;

  GLfloat *displacementZ;
  GLuint dispZ;

  GLfloat *normalX;
  GLuint nX;

  GLfloat *normalZ;
  GLuint nZ;

  QGLShaderProgram *displayShader;
  QGLShaderProgram *renderShader;

  GLuint cubeMap;

  void updateHeightfield();

  void buildGrid();
  void renderGrid(GLuint positionLocation,GLuint texCoordLocation, QGLShaderProgram *program);
  GLuint createFrameBufferObject(GLuint nbTextures, GLuint * textureToAttachID, GLint textureWidth, GLint textureHeight, GLenum * attachment);
  void buildCubeMap(QString xpos,QString xneg,QString ypos,QString yneg,QString zpos,QString zneg);

  rx::OceanSurface surf;

protected:
  void initializeGL();
  void resizeGL(int width, int height);
  void paintGL();
  void keyPressEvent( QKeyEvent *k );
  void mouseMoveEvent ( QMouseEvent * eve );
  void wheelEvent( QWheelEvent * eve);

  protected slots:

  void update_parameters();

};

#endif // WATERFFTVISU_H
