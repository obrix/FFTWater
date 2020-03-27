#include "camera.h"
#include <iostream>
#include <qmath.h>

Camera::Camera()
{
	//look to -z by default
	orientation = QQuaternion(0.0,0.0,0.0,1.0);

	pitchAxis = QVector3D(1.0,0.0,0.0);
	yawAxis = QVector3D(0.0,1.0,0.0);
	
	//default Value
	FOV = 60.0f;
	AspectRatio = 4.0/3.0;
	zNear = 1.0f;
	zFar  = 2000.0f;

	ProjectionMatrix.setToIdentity();
	ProjectionMatrix.perspective(FOV,AspectRatio,zNear,zFar);

	position = QVector3D(0.0,0.0,0.0);
	rail = QList<OrientedPoint>();

	camSpeed = 1;

	skyboxModelViewMatrix.setToIdentity();
}

void Camera::calcPitchAxis()
{
	pitchAxis.setX(orientation.z());
	pitchAxis.setZ(-orientation.x());
	pitchAxis.normalize();
}

void Camera::calcYawAxis()
{
	yawAxis = QVector3D::crossProduct(orientation.vector(), pitchAxis).normalized(); 
}

void Camera::calcPitchAndYaw()
{
	calcPitchAxis();
	calcYawAxis();
}

void Camera::translate(float x, float y, float z)
{
    position += QVector3D(x,y,z);
}

void Camera::translate(QVector3D t)
{
	position+=t;
}
void Camera::rotate(float angle, float axeX, float axeY, float axeZ)
{
    QQuaternion q = QQuaternion::fromAxisAndAngle(axeX,axeY,axeZ,angle);
	orientation= q * orientation * q.conjugate();
	calcPitchAndYaw();
	ViewMatrix.rotate(angle, axeX, axeY, axeZ);
	skyboxModelViewMatrix.rotate(-q);
}

void Camera::rotate(QQuaternion q)
{
	q.normalize();
	orientation= q * orientation * q.conjugate();
	calcPitchAndYaw();
	ViewMatrix.rotate(q);
	skyboxModelViewMatrix.rotate(-q);
}

void Camera::rotateX(float angle)
{
	QQuaternion q = QQuaternion::fromAxisAndAngle(1.0, 0.0, 0.0, angle);
	orientation= q * orientation * q.conjugate();
	calcYawAxis();
	skyboxModelViewMatrix.rotate(-angle, 1.0, 0.0, 0.0);
}
void Camera::rotateY(float angle)
{
	QQuaternion q = QQuaternion::fromAxisAndAngle(0.0,1.0,0.0,angle);
	orientation= q * orientation * q.conjugate();
	calcPitchAndYaw();
	skyboxModelViewMatrix.rotate(-angle, 0.0,1.0,0.0);
}
void Camera::rotateZ(float angle)
{
	QQuaternion q = QQuaternion::fromAxisAndAngle(0.0, 0.0, 1.0, angle);
	orientation= q * orientation * q.conjugate();
	calcYawAxis();
	skyboxModelViewMatrix.rotate(-angle, 0.0, 0.0, 1.0);
}

void Camera::rotatePitch(float angle)
{
	QQuaternion q = QQuaternion::fromAxisAndAngle(pitchAxis,angle);
	QQuaternion save = orientation;
	orientation = q * orientation * q.conjugate();
	calcYawAxis();
	if(yawAxis.y() < 0)	// Y U UPSIDE DOWN ?!
	{
		orientation = save;
		calcYawAxis();
		return;
	}
	skyboxModelViewMatrix.rotate(-angle,pitchAxis);
}
void Camera::rotateYaw(float angle)
{
	QQuaternion q = QQuaternion::fromAxisAndAngle(yawAxis,angle);
	orientation= q * orientation * q.conjugate();
	calcPitchAxis();
	skyboxModelViewMatrix.rotate(-angle,yawAxis);
}

const QMatrix4x4 & Camera::getViewMatrix()
{
	buildViewMatrix();
	return ViewMatrix;
}

void Camera::setAspectRatio(float ar)
{
	AspectRatio = ar;
	ProjectionMatrix.setToIdentity();
	ProjectionMatrix.perspective(FOV,AspectRatio,zNear,zFar);

}

void Camera::setPlanes(float near, float far)
{
	zNear = near;
	zFar = far;
	ProjectionMatrix.setToIdentity();
	ProjectionMatrix.perspective(FOV,AspectRatio,zNear,zFar);

}

void Camera::setFOV(float angle)
{
	FOV = angle;
	ProjectionMatrix.setToIdentity();
	ProjectionMatrix.perspective(FOV,AspectRatio,zNear,zFar);
}

const QMatrix4x4 & Camera::getProjectionMatrix()
{
	return ProjectionMatrix;
}

void Camera::buildViewMatrix()
{
	ViewMatrix.setToIdentity();
	ViewMatrix.lookAt(position,position+orientation.vector(), yawAxis);
}

QVector3D Camera::getPosition()
{
	return position;
}

void Camera::setOrientation(QVector3D vec)
{
	orientation.setX(vec.x());
	orientation.setY(vec.y());
	orientation.setZ(vec.z());
	orientation.setScalar(0);
}

QMatrix4x4 Camera::getYMirroredViewMatrix()
{
	QVector3D m(1.0,-1.0,1.0);
	QMatrix4x4 mirroredViewMatrix;
	mirroredViewMatrix.setToIdentity();
	mirroredViewMatrix.lookAt(position*m,(position+orientation.vector())*m,QVector3D(0.0,1.0,0.0));
	return mirroredViewMatrix;
}

QVector3D Camera::getOrientation()
{
	return orientation.vector();
}

QVector3D Camera::getSideVector()
{
	return pitchAxis;
}

void Camera::move(QVector3D destination, QQuaternion finalOrientation)
{
	position = destination;
	orientation = finalOrientation.normalized();

	//yawAxis = (orientation * QQuaternion(0.0, 0.0, 1.0, 0.0) * orientation.conjugate()).vector();
	//yawAxis.normalize();
	calcPitchAndYaw();
	buildSkyboxMV();
}

void Camera::buildRailPath(QList<TimedPoint> customRail)
{
	/*** Interpolation between the points of customRail ***/

	int imax; //number of interpolated points between the timedPoints
	double t; //interpolation factor : Pi = P0 * (1 - t) + Pimax * t
	int it; //iterator on customRail

	//Interpolate the path segments from the first to the last point
	for(it = 0; it < customRail.size()-1; ++it)
	{
		rail.append(OrientedPoint(customRail[it].pos, customRail[it].dir));
		imax = (int)customRail[it+1].timeToGo * 30;
		for(int i = 0; i < imax; ++i)
		{
			t = (double)i / imax;
			rail.append(OrientedPoint(	customRail[it+1].pos * t + customRail[it].pos * (1 - t),
										/*QQuaternion::*/slerp(customRail[it].dir, customRail[it+1].dir, t)));
		}
	}
	//Interpolate the path between the last and the first point to generate a cycled path
	rail.append(OrientedPoint(customRail[it].pos, customRail[it].dir));
	imax = (int)customRail[0].timeToGo * 30;
	for(int i = 0; i < imax; ++i)
	{
		t = (double)i / imax;
		rail.append(OrientedPoint(	customRail[0].pos * t + customRail[it].pos * (1 - t),
									/*QQuaternion::*/slerp(customRail[it].dir, customRail[0].dir, t)));
	}

	isRail = true;
	railIndex = 0;

}

void Camera::buildRailPath()
{
	/*** Path Initialization with fixed oriented points ***/

	//TODO PLUS DE POIIIIIINTS !

	QList<TimedPoint> initList;
	QVector3D vec = QVector3D(500,250,500);
	QQuaternion q = QQuaternion(0, -1,-0.5,-1).normalized();
	initList.append(TimedPoint(vec,q, 12.0f));
	vec = QVector3D(500,110,-500);
	q = QQuaternion(0, -1,-0.3,1).normalized();
	initList.append(TimedPoint(vec,q, 12.0f));
	vec = QVector3D(-500,380,-500);
	q = QQuaternion(0, 1,-0.6,1).normalized();
	initList.append(TimedPoint(vec,q, 6.0f));
	vec = QVector3D(-500,110,500);
	q = QQuaternion(0, 1,-0.3,-1).normalized();
	initList.append(TimedPoint(vec,q, 12.0f));

	buildRailPath(initList);
	move(rail[0].pos, rail[0].dir);
}

void Camera::followRail()
{
	move(rail[railIndex].pos, rail[railIndex].dir);
}

void Camera::update()
{
	if(isRail)
	{
		railIndex += camSpeed; 
		if(railIndex >= rail.size())
			railIndex = 0;
		followRail();
	}

	// TODO Moche, trouver un système qui évite de les reset à chaque update
	//pitchAxis = QVector3D(1.0,0.0,0.0);
	//yawAxis = QVector3D(0.0,1.0,0.0);

}

QQuaternion Camera::slerp(const QQuaternion& q1, const QQuaternion& q2, qreal t)
{
    // Handle the easy cases first.
    if (t <= 0.0f)
        return q1;
    else if (t >= 1.0f)
        return q2;

    // Determine the angle between the two quaternions.
    QQuaternion q2b;
    qreal dot;
    dot = q1.x() * q2.x() + q1.y() * q2.y() + q1.z() * q2.z() + q1.scalar() * q2.scalar();
    if (dot >= 0.0f) {
        //q2b = q2;
    } else {
        //q2b = -q2;
        dot = -dot;
    }

    // Get the scale factors.  If they are too small,
    // then revert to simple linear interpolation.
    qreal factor1 = 1.0f - t;
    qreal factor2 = t;
    if ((1.0f - dot) > 0.0000001) {
        qreal angle = qreal(qAcos(dot));
        qreal sinOfAngle = qreal(qSin(angle));
        if (sinOfAngle > 0.0000001) {
            factor1 = qreal(qSin((1.0f - t) * angle)) / sinOfAngle;
            factor2 = qreal(qSin(t * angle)) / sinOfAngle;
        }
    }

    // Construct the result quaternion.
    return q1 * factor1 + q2 * factor2;
}

QMatrix4x4 Camera::getSkyboxMV()
{
	return skyboxModelViewMatrix;
}

void Camera::buildSkyboxMV()
{
	skyboxModelViewMatrix.setToIdentity();
	skyboxModelViewMatrix.lookAt(QVector3D(0.0,0.0,0.0), orientation.vector(), QVector3D(0.0,1.0,0.0) );
}

int Camera::getCameraSpeed()
{
	return camSpeed;
}

void Camera::setCameraSpeed(int cameraSpeed)
{
	camSpeed = cameraSpeed;
}

void Camera::upCameraSpeed()
{
	if(camSpeed < 10)
		++camSpeed;
}

void Camera::downCameraSpeed()
{
	if(camSpeed>1)
		--camSpeed;
}