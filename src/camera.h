#ifndef CAMERA_H
#define CAMERA_H
#include <qquaternion.h>
#include <QVector3D>
#include <QMatrix4x4>
#include <qlist.h>
#include <qquaternion.h>
class Camera
{
	typedef struct _OrientedPoint_
	{
		 QVector3D pos;
		 QQuaternion dir;

		 _OrientedPoint_(QVector3D _p, QQuaternion _q)
		 {
			 pos = _p;
			 dir = _q;
		 }
	}OrientedPoint;

	typedef struct _TimedPoint_
	{
		 QVector3D pos;
		 QQuaternion dir;
		 float timeToGo;

		 _TimedPoint_(QVector3D _p, QQuaternion _q, float _timeToGo)
		 {
			 pos = _p;
			 dir = _q;
			 timeToGo = _timeToGo;
		 }
	}TimedPoint;

    QVector3D position;
    QQuaternion orientation;
    QMatrix4x4 ViewMatrix;
    QMatrix4x4 ProjectionMatrix;

	QVector3D pitchAxis;
	QVector3D yawAxis;

	float AspectRatio;
	float FOV;
	float zNear;
	float zFar;

	QList<OrientedPoint> rail;
	int railIndex;
	bool isRail;
	int camSpeed;

	static QQuaternion slerp(const QQuaternion& q1, const QQuaternion& q2, qreal t);
	void buildRailPath(QList<TimedPoint> customRail);

	QMatrix4x4 skyboxModelViewMatrix;


public:
    Camera();

	void calcPitchAxis();
	void calcYawAxis();
	void calcPitchAndYaw();
    void translate(float x, float y, float z);
	void translate(QVector3D t);

	void rotate(float angle, float axeX, float axeY, float axeZ);
	void rotate(QQuaternion q);

	void rotateX(float angle);
	void rotateY(float angle);
	void rotateZ(float angle);

	void rotatePitch(float angle);
	void rotateYaw(float angle);

    const QMatrix4x4 & getViewMatrix();
    void setAspectRatio(float ar);
    void setPlanes(float near, float far);
    void setFOV(float angle);
    const QMatrix4x4 & getProjectionMatrix();
	QVector3D getPosition();
	QMatrix4x4 getYMirroredViewMatrix();

	int getCameraSpeed();
	void setCameraSpeed(int cameraSpeed);
	void upCameraSpeed();
	void downCameraSpeed();

	void setOrientation(QVector3D vec);
	QVector3D getOrientation();
	QVector3D getSideVector();
	QMatrix4x4 getSkyboxMV();

	void update();

	void move(QVector3D destination, QQuaternion finalOrientation);
	void buildRailPath();
	void followRail();
protected:
    void buildViewMatrix();
	void buildSkyboxMV();
    void projectionMatrix();

};

#endif // CAMERA_H
