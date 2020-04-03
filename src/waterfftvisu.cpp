#include "waterfftvisu.h"

#include "ui_waterfftvisu.h"
#include "ui_DialogParameters.h"
#include <iostream>
#include <random>

waterFFTvisu::waterFFTvisu(QWidget *parent)
  : QGLWidget(parent,0)
{
  ui.setupUi(this);
  this->resize(1280,720);
  setMouseTracking(true);
  freeCamera = true;

  t = 0.0;

  //parameters dialog box stuff
  parameterBox = new QWidget();

  dialog_parameters.setupUi(parameterBox);
  parameterBox->setVisible(true);

  //default parameters for the heightfield
  HFIELD_SIZE = 100.0; //cell size
  WINDX = 30.0;
  WINDZ = 0.0;
  WAVEHEIGHTFACTOR = 0.0002; // A in the formula
  GRAVIT_CST = 9.81;
  DISPLACE_LAMBDA = 0.8;

  surf.PrecomputeFields();

  connect(this->dialog_parameters.pushButton,SIGNAL(pressed()),this,SLOT(update_parameters()));
  connect(this->dialog_parameters.HFieldSpinBox,SIGNAL(valueChanged(double)),this,SLOT(update_parameters()));
  connect(this->dialog_parameters.WindXSpinBox,SIGNAL(valueChanged(double)),this,SLOT(update_parameters()));
  connect(this->dialog_parameters.WindZSpinBox,SIGNAL(valueChanged(double)),this,SLOT(update_parameters()));
  connect(this->dialog_parameters.WaveHFFactorSpinBox,SIGNAL(valueChanged(double)),this,SLOT(update_parameters()));
  connect(this->dialog_parameters.GravitCstSpinBox,SIGNAL(valueChanged(double)),this,SLOT(update_parameters()));
  connect(this->dialog_parameters.LmbdaDispSpinBox,SIGNAL(valueChanged(double)),this,SLOT(update_parameters()));
  cam = new Camera();
  theTime.start();
}

waterFFTvisu::~waterFFTvisu()
{
  delete cam;
}


void waterFFTvisu::initializeGL()
{
  glEnable(GL_DEPTH_TEST);

  glewExperimental = GL_TRUE;
  GLenum error = glewInit();
  if(error != GLEW_OK)
  {
    QMessageBox::critical(this, trUtf8("Erreur"), trUtf8("Echec de l'initialization de GLEW: %1").arg(reinterpret_cast<const char *>(glewGetErrorString(error))));
    exit(-1);
  }


  QGLFormat format;
  QGLFormat::OpenGLVersionFlags flag = format.openGLVersionFlags();
  if(flag & QGLFormat::OpenGL_Version_1_5)
    qDebug() << "1.5 or higher";
  if(flag & QGLFormat::OpenGL_Version_2_0)
    qDebug() << "2.0 or higher";
  if(flag & QGLFormat::OpenGL_Version_3_1)
    qDebug() << "3.1 or higher";
  if(flag & QGLFormat::OpenGL_Version_3_2)
    qDebug() << "3.2 or higher";
  if(flag & QGLFormat::OpenGL_Version_3_3)
    qDebug() << "3.3 or higher";
  if(flag & QGLFormat::OpenGL_Version_4_0)
    qDebug() << "4.0 or higher";

  displayShader = loadShaders("Resources/display.vsh", "Resources/display.fsh");
  renderShader = loadShaders("Resources/renderGrid.vsh", "Resources/renderGrid.fsh");

  heightmap = createTexture(WIDTH , HEIGHT, 0,GL_R16F,GL_RED,GL_FLOAT,0);
  dispX = createTexture(WIDTH , HEIGHT, 0,GL_R16F,GL_RED,GL_FLOAT,0);
  dispZ = createTexture(WIDTH , HEIGHT, 0,GL_R16F,GL_RED,GL_FLOAT,0);

  nX = createTexture(WIDTH , HEIGHT, 0,GL_R16F,GL_RED,GL_FLOAT,0);
  nZ = createTexture(WIDTH , HEIGHT, 0,GL_R16F,GL_RED,GL_FLOAT,0);

  buildScreenQuad();
  surf.ComputeHeightmap(t);
  updateHeightfield();
  buildGrid();

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  //for wireframe mode
  //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

  buildCubeMap("Resources/skybox3/right.png",
                 "Resources/skybox3/left.png",
                 "Resources/skybox3/up.png",
                 "Resources/skybox3/down.png",
                 "Resources/skybox3/right_r.png",
                 "Resources/skybox3/center.png");
}
void waterFFTvisu::resizeGL(int width, int height)
{
  if(height == 0)
        height = 1;
    glViewport(0, 0, width, height);

}
void waterFFTvisu::paintGL()
{
  winX = QCursor::pos().x();
  winY = QCursor::pos().y();
  t = theTime.elapsed()/1000.0;

  //MV and P Matrix init
  glViewport(0,0,1280,720);

  //Rendering part
  glUseProgram(renderShader->programId());
  glClearColor(0.2,0.4,0.6,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  GLuint vertexLocation = renderShader->attributeLocation("position");
  GLuint texCoordLocation = renderShader->attributeLocation("texCoord");

  renderShader->setUniformValue("cameraPos",cam->getPosition());

  //Send the data textures to the shaders
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,heightmap);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmapEXT(GL_TEXTURE_2D);
  renderShader->setUniformValue("heightmap",0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D,dispX);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmapEXT(GL_TEXTURE_2D);
  renderShader->setUniformValue("dispX",1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D,dispZ);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmapEXT(GL_TEXTURE_2D);
  renderShader->setUniformValue("dispZ",2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D,nX);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmapEXT(GL_TEXTURE_2D);
  renderShader->setUniformValue("nX",3);

  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D,nZ);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmapEXT(GL_TEXTURE_2D);
  renderShader->setUniformValue("nZ",4);

  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
  renderShader->setUniformValue("cubeMap",5);


  renderShader->setUniformValue("Projection",cam->getProjectionMatrix());

  QMatrix4x4 tmp;
  tmp.scale(HFIELD_SIZE/100.0,1.0,HFIELD_SIZE/100.0);
  renderShader->setUniformValue("Model",tmp);
  renderShader->setUniformValue("ModelView",cam->getViewMatrix()*tmp);
  //render the grid
  renderGrid(vertexLocation,texCoordLocation,renderShader);



  //Display the grey heihtmap on the border
  glUseProgram(displayShader->programId());
  glViewport(0.0,0.0,200,200);

  vertexLocation = displayShader->attributeLocation("position");
  texCoordLocation = displayShader->attributeLocation("texCoord");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,heightmap);
  displayShader->setUniformValue("inTexture",0);

  drawScreenQuad(vertexLocation,texCoordLocation);

  surf.ComputeHeightmap(t);

  updateHeightfield();

  //Should not do this
  update();
}

void waterFFTvisu::update_parameters()
{
  HFIELD_SIZE = dialog_parameters.HFieldSpinBox->value();; //cell size
  WINDX = dialog_parameters.WindXSpinBox->value();
  WINDZ = dialog_parameters.WindZSpinBox->value();
  WAVEHEIGHTFACTOR = dialog_parameters.WaveHFFactorSpinBox->value(); // A in the formula
  GRAVIT_CST = dialog_parameters.GravitCstSpinBox->value();
  DISPLACE_LAMBDA = dialog_parameters.LmbdaDispSpinBox->value();

  surf.PrecomputeFields();
}

QGLShaderProgram * waterFFTvisu::loadShaders(QString vertexFile, QString fragmentFile)
{
  //load shaders
  QGLShaderProgram *program = new QGLShaderProgram();
  if(!program->addShaderFromSourceFile(QGLShader::Vertex,vertexFile))
    QMessageBox::critical(this, trUtf8("Erreur vertex"),program->log());
  if(!program->addShaderFromSourceFile(QGLShader::Fragment,fragmentFile))
  {
    QMessageBox::critical(this, trUtf8("Erreur fragment"),program->log());
  }
  if(!program->link())
    QMessageBox::critical(this, trUtf8("Link"),program->log());

  return program;
}

void waterFFTvisu::buildScreenQuad()
{
  //Quad pour l'affichage finale
  static const GLfloat g_quad_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
  };
  static const GLfloat g_quad_TexCoord_buffer_data[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f,  1.0f,
    0.0f,  1.0f,
    1.0f, 0.0f,
    1.0f,  1.0f,
  };

  glGenBuffers(1, &ScAquad_vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, ScAquad_vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

  glGenBuffers(1, &ScAquad_TexCoordBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, ScAquad_TexCoordBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_TexCoord_buffer_data), g_quad_TexCoord_buffer_data, GL_STATIC_DRAW);
}

void waterFFTvisu::drawScreenQuad(GLuint vertexLocation, GLuint texCoordLocation)
{
  glBindBuffer(GL_ARRAY_BUFFER,ScAquad_vertexbuffer);
  glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLocation);

  glBindBuffer(GL_ARRAY_BUFFER,ScAquad_TexCoordBuffer);
  glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(texCoordLocation);

  glDrawArrays(GL_TRIANGLES,0,6);
}

GLuint waterFFTvisu::createTexture(GLint width, GLint height, void *texture, GLint internalFormat, GLenum format, GLenum type, bool clamp)
{
  GLuint localTextureID;
  glGenTextures(1, &localTextureID); //Get texture's unique ID
  glBindTexture(GL_TEXTURE_2D, localTextureID);  //Bind texture

  if(internalFormat == GL_DEPTH_COMPONENT)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  if(clamp)
  {
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
  }
  else
  {
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  }

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, texture); //Create texture (for active texture: textureID)
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmapEXT(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  return localTextureID;
}

//Update the heigtfield by generating a new Heightfield with FFT
void waterFFTvisu::updateHeightfield()
{
  //Update the textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,heightmap);
  glTexSubImage2D(GL_TEXTURE_2D,0,0,0,WIDTH,HEIGHT,GL_RED,GL_FLOAT,surf.m_heightmapData);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D,dispX);
  glTexSubImage2D(GL_TEXTURE_2D,0,0,0,WIDTH,HEIGHT,GL_RED,GL_FLOAT,surf.m_displacementX);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D,dispZ);
  glTexSubImage2D(GL_TEXTURE_2D,0,0,0,WIDTH,HEIGHT,GL_RED,GL_FLOAT,surf.m_displacementZ);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D,nX);
  glTexSubImage2D(GL_TEXTURE_2D,0,0,0,WIDTH,HEIGHT,GL_RED,GL_FLOAT,surf.m_normalX);

  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D,nZ);
  glTexSubImage2D(GL_TEXTURE_2D,0,0,0,WIDTH,HEIGHT,GL_RED,GL_FLOAT,surf.m_normalZ);

}

void waterFFTvisu::loadTexture(QString textureName,GLuint *textureUnit, bool wrap)
{
    QImage qim_TempTexture;

    qim_TempTexture.load(textureName);
  if(qim_TempTexture.isNull())
    qDebug()<<"Not initiated";
    QImage *qim_Texture = new QImage(QGLWidget::convertToGLFormat( qim_TempTexture ));
    glGenTextures( 1, textureUnit );
    glBindTexture( GL_TEXTURE_2D, *textureUnit );
    glTexImage2D( GL_TEXTURE_2D, 0, 3, qim_Texture->width(), qim_Texture->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, qim_Texture->bits() );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  if(wrap)
  {
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  }
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmapEXT(GL_TEXTURE_2D);
  delete qim_Texture;
}

//build the support grid
void waterFFTvisu::buildGrid()
{
  GLfloat * mesh = new GLfloat[3*WIDTH*HEIGHT];
  GLfloat * textureCoord = new GLfloat[2*WIDTH*HEIGHT];

  float taillePlan = HFIELD_SIZE;

  int k=0;
  int l=0;
  for(int i =0 ; i <WIDTH; i++)
  {
    for(int j = 0; j <HEIGHT; j++)
    {
      mesh[k+0] = ((GLfloat)j / (GLfloat)(WIDTH-1)) * taillePlan;
      mesh[k+1] =0.0;
      mesh[k+2] = ((GLfloat)i / (GLfloat)(HEIGHT-1)) * taillePlan;
      textureCoord[l+0] = ((GLfloat)j / (GLfloat)(WIDTH-1));
      textureCoord[l+1] = ((GLfloat)i / (GLfloat)(HEIGHT-1));
      k+=3;
      l+=2;
    }
  }

  GLuint *indice = new GLuint[2*(WIDTH-1)*(HEIGHT-1)*3];

  k = 0;
  for(int i =0 ; i <WIDTH-1; i++)
  {
    for(int j = 0; j <HEIGHT-1; j++)
    {
      indice[k+0] = i*WIDTH+j;
      indice[k+1] = i*WIDTH+j+WIDTH;
      indice[k+2] = i*WIDTH+j+1;
      indice[k+3] = i*WIDTH+j+WIDTH;
      indice[k+4] = i*WIDTH+j+WIDTH+1;
      indice[k+5] = i*WIDTH+j+1;
      k+=6;
    }
  }

  nbVertex = 2*(WIDTH-1)*(HEIGHT-1)*3;


  glGenBuffers(1,&vboPlan);
  glBindBuffer(GL_ARRAY_BUFFER,vboPlan);
  glBufferData(GL_ARRAY_BUFFER,  3*WIDTH*WIDTH*sizeof(GLfloat), mesh,  GL_STATIC_DRAW);

  glGenBuffers(1,&vboIndexPlan);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vboIndexPlan);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*(WIDTH-1)*(WIDTH-1)*3*sizeof(GLuint), indice,  GL_STATIC_DRAW);

  glGenBuffers(1,&vboTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER,vboTexCoord);
    glBufferData(GL_ARRAY_BUFFER,2*WIDTH*WIDTH*sizeof(GLfloat),textureCoord,GL_STATIC_DRAW);

  delete mesh;
  delete textureCoord;
  delete indice;
}

//render the support grid
void waterFFTvisu::renderGrid(GLuint positionLocation,GLuint texCoordLocation, QGLShaderProgram *program)
{
  glBindBuffer(GL_ARRAY_BUFFER,vboPlan);
  glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(positionLocation);

  glBindBuffer(GL_ARRAY_BUFFER,vboTexCoord);
    glVertexAttribPointer(texCoordLocation,2,GL_FLOAT,GL_FALSE,0,0);
    glEnableVertexAttribArray(texCoordLocation);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vboIndexPlan);
  glDrawElements(GL_TRIANGLES, nbVertex,GL_UNSIGNED_INT,0);
}

GLuint waterFFTvisu::createFrameBufferObject(GLuint nbTextures, GLuint * textureToAttachID, GLint textureWidth, GLint textureHeight, GLenum * attachment)
{
  //Create frame buffer object (active FBO)
  GLuint frameBufferID;
  glGenFramebuffers(1, &frameBufferID);
  glBindFramebuffer(GL_FRAMEBUFFER_EXT, frameBufferID);//Bind to make active

  //Attach texture to FBO as color attachment
  for(int i = 0; i<nbTextures; i++)
    glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, attachment[i], GL_TEXTURE_2D, textureToAttachID[i], 0);

  //Unbind frame buffer object
  glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

  return frameBufferID;
}

void waterFFTvisu::keyPressEvent( QKeyEvent *k )
{
  if(k->key() == Qt::Key_Escape)
    QApplication::exit();

  QVector3D direction = cam->getOrientation();
  direction.normalize();
  QVector3D sideVector = cam->getSideVector();
  sideVector.normalize();
  if(freeCamera)
  {
    switch ( k->key() )
    {
      case Qt::Key_Z: //
        cam->rotatePitch(-10.0);
        break;
      case Qt::Key_S: //
        cam->rotatePitch(10.0);
        break;
      case Qt::Key_Q: //
        cam->rotateY(10.0);
        break;
      case Qt::Key_D: //
        cam->rotateY(-10.0);
        break;
      case Qt::Key_Up ://
        cam->translate(direction * 10);
        break;
      case Qt::Key_Down: //
        cam->translate(-direction * 10);
        break;
      case Qt::Key_Right: //
        cam->translate(-sideVector * 3);
        break;
      case Qt::Key_Left: //
        cam->translate(sideVector * 3);
        break;
      case Qt::Key_PageUp: //
        cam->translate(0.0,0.2,0.0);
        break;
      case Qt::Key_PageDown: //
        cam->translate(0.0,-0.2,0.0);
        break;
      case Qt::Key_Space:
        freeCamera = false;
        break;
      default:
        ;
    }
  }
  else
  {
    if(k->key() == Qt::Key_Space)
      freeCamera = true;
  }
}
void waterFFTvisu::mouseMoveEvent ( QMouseEvent * eve )
{
  if(freeCamera)
  {
    int winXDelta = winX - QCursor::pos().x();
    int winYDelta = QCursor::pos().y() - winY;


    QVector3D sideVector = cam->getSideVector();
    sideVector.normalize();
    float angle = 0;

    cam->rotatePitch(winYDelta);

    cam->rotateY(winXDelta);
  }
}
void waterFFTvisu::wheelEvent( QWheelEvent * eve)
{
}

//build the Cubre Map
void waterFFTvisu::buildCubeMap(QString xpos,QString xneg,QString ypos,QString yneg,QString zpos,QString zneg)
{
  glActiveTexture(GL_TEXTURE2);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glGenTextures(2, &cubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  QImage qim_TempTexture;
  QImage qim_TempTexture2;

    qim_TempTexture.load(xpos);
    QImage *qim_Texture = new QImage(QGLWidget::convertToGLFormat( qim_TempTexture ));
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, qim_Texture->width(), qim_Texture->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, qim_Texture->bits());
  qim_Texture->~QImage();

  qim_TempTexture.detach();
  qim_TempTexture.load(xneg);
    qim_Texture = new QImage(QGLWidget::convertToGLFormat( qim_TempTexture ));

    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X,  0, GL_RGBA, qim_Texture->width(), qim_Texture->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, qim_Texture->bits());
  qim_Texture->~QImage();

  qim_TempTexture.load(ypos);
    qim_Texture = new QImage(QGLWidget::convertToGLFormat( qim_TempTexture ));
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y,  0, GL_RGBA, qim_Texture->width(), qim_Texture->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, qim_Texture->bits());
  qim_Texture->~QImage();

  qim_TempTexture.load(yneg);
    qim_Texture = new QImage(QGLWidget::convertToGLFormat( qim_TempTexture ));
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,  0, GL_RGBA, qim_Texture->width(), qim_Texture->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, qim_Texture->bits());
  qim_Texture->~QImage();

  qim_TempTexture.load(zpos);
    qim_Texture = new QImage(QGLWidget::convertToGLFormat( qim_TempTexture ));
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, qim_Texture->width(), qim_Texture->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, qim_Texture->bits());
  qim_Texture->~QImage();

  qim_TempTexture.load(zneg);
    qim_Texture = new QImage(QGLWidget::convertToGLFormat( qim_TempTexture ));
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,  0, GL_RGBA, qim_Texture->width(), qim_Texture->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, qim_Texture->bits());
  qim_Texture->~QImage();

  //glDisable(GL_TEXTURE_CUBE_MAP);
}

#include "moc_waterfftvisu.cpp"
