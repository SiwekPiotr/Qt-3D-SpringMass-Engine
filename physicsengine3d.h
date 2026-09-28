#ifndef PHYSICSENGINE3D_H
#define PHYSICSENGINE3D_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>
#include "springmasssystem3d.h"

class PhysicsEngine3D : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit PhysicsEngine3D(QWidget *parent = nullptr);
    ~PhysicsEngine3D();

    void updateSimulation(float deltaTime);
    void createCubeGrid(int pointsPerSide, float spacing);
    void resetSystem();

    void createCloth(int widthPoints, int heightPoints, float spacing);

    void setSimulationMode(bool isCloth) { clothMode = isCloth; }
    bool getSimulationMode() const { return clothMode; }

    GLuint sphereList = 0;
    GLuint groundList = 0;

public slots:
    void setSpringStiffness(float stiffness);
    void setSpringDamping(float damping);
    void setSpringRestLength(float restLength);
    void setPointsPerSide(int count);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void drawPoint(const PointMass3D& point);
    void drawSpring(const Spring3D& spring);
    void drawGround();
    void drawSolidSphere(float radius, int slices, int stacks);
    int findPointNearMouse(const QPoint& mousePos, float maxDistance = 20.0f);
    void handleCollisions();
    QVector3D screenToWorld(const QPoint& screenPos);
    QVector3D worldToScreen(const QVector3D& worldPos);
    QVector3D heldPosition;

    SpringMassSystem3D system;
    int selectedPoint = -1;
    float groundLevel;

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QVector3D cameraTarget;
    QVector3D cameraUp;
    float cameraDistance;
    QPoint lastMousePos;
    QQuaternion cameraRotation;

    QVector3D mouseOffset;
    bool isDragging = false;
    bool isRotating = false;

    float currentStiffness;
    float currentDamping;
    float currentRestLength;
    int currentPointsPerSide;

    void updateSpringParameters();

    bool clothMode;

    void createClothSprings(int widthPoints, int heightPoints, float spacing);
};

#endif
