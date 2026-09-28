#include "physicsengine3d.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <cmath>
#include <algorithm>

PhysicsEngine3D::PhysicsEngine3D(QWidget *parent) : QOpenGLWidget(parent),
    groundLevel(-2.0f),
    cameraTarget(0, 0, 0),
    cameraUp(0, 1, 0),
    cameraDistance(25.0f),
    cameraRotation(1, 0, 0, 0),
    currentStiffness(600.0f),
    currentDamping(8.0f),
    currentRestLength(0.3f),
    currentPointsPerSide(4)
{
    setFocusPolicy(Qt::StrongFocus);
    createCubeGrid(currentPointsPerSide, currentRestLength);
}

PhysicsEngine3D::~PhysicsEngine3D(){
    makeCurrent();

    if(sphereList != 0){
        glDeleteLists(sphereList, 1);
        sphereList = 0;
    }

    if(groundList != 0){
        glDeleteLists(groundList, 1);
        groundList = 0;
    }

    doneCurrent();
}

void PhysicsEngine3D::initializeGL(){
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    sphereList = glGenLists(1);
    glNewList(sphereList, GL_COMPILE);
    drawSolidSphere(0.06f, 16, 16);
    glEndList();

    groundList = glGenLists(1);
    glNewList(groundList, GL_COMPILE);
    drawGround();
    glEndList();
}

void PhysicsEngine3D::resizeGL(int w, int h){
    glViewport(0, 0, w, h);
    projection.setToIdentity();
    projection.perspective(45.0f, float(w) / float(h), 0.1f, 100.0f);
}

void PhysicsEngine3D::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view.setToIdentity();
    QVector3D cameraPos = cameraRotation.rotatedVector(QVector3D(0, 0, cameraDistance));
    view.lookAt(cameraPos, cameraTarget, cameraUp);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection.constData());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view.constData());

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat lightPos[] = {5.0f, 5.0f, 5.0f, 1.0f};
    GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);

    glCallList(groundList);

    const auto& springs = system.getSprings();
    for(const auto& spring : springs){
        drawSpring(spring);
    }

    const auto& points = system.getPoints();
    for(int i = 0; i < points.size(); ++i){
        drawPoint(points[i]);
    }

    glDisable(GL_LIGHTING);
}

void PhysicsEngine3D::drawPoint(const PointMass3D& point){
    glPushMatrix();
    glTranslatef(point.position.x(), point.position.y(), point.position.z());

    if(selectedPoint != -1 && &point == &system.getPoints()[selectedPoint]){
        glColor3f(1.0f, 1.0f, 0.0f);
    } else if(point.position.y() <= groundLevel + 0.1f){
        glColor3f(1.0f, 0.5f, 0.0f);
    } else if(point.fixed){
        glColor3f(1.0f, 0.0f, 0.0f);
    } else{
        glColor3f(0.2f, 0.6f, 1.0f);
    }

    glCallList(sphereList);

    glPopMatrix();
}

void PhysicsEngine3D::drawSolidSphere(float radius, int slices, int stacks){
    for(int i = 0; i < stacks; ++i){
        float phi1 = M_PI * float(i) / float(stacks);
        float phi2 = M_PI * float(i + 1) / float(stacks);

        glBegin(GL_TRIANGLE_STRIP);
        for(int j = 0; j <= slices; ++j){
            float theta = 2.0f * M_PI * float(j) / float(slices);

            auto vertex = [radius](float phi, float theta){
                float x = radius * sin(phi) * cos(theta);
                float y = radius * cos(phi);
                float z = radius * sin(phi) * sin(theta);
                glNormal3f(x/radius, y/radius, z/radius);
                glVertex3f(x, y, z);
            };

            vertex(phi1, theta);
            vertex(phi2, theta);
        }
        glEnd();
    }
}

void PhysicsEngine3D::drawSpring(const Spring3D& spring){
    const auto& points = system.getPoints();
    if(spring.pointA < points.size() && spring.pointB < points.size()){
        const QVector3D& posA = points[spring.pointA].position;
        const QVector3D& posB = points[spring.pointB].position;

        float currentLength = posA.distanceToPoint(posB);

        float stretch = currentLength / spring.restLength;

        if(stretch > 1.0f){
            float t = std::min(1.0f, (stretch - 1.0f) * 3.0f);
            glColor3f(1.0f, 1.0f - t, 0.0f);
        } else{
            float t = std::min(1.0f, (1.0f - stretch) * 3.0f);
            glColor3f(0.0f, t, 1.0f);
        }

        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex3f(posA.x(), posA.y(), posA.z());
        glVertex3f(posB.x(), posB.y(), posB.z());
        glEnd();
    }
}

void PhysicsEngine3D::drawGround(){
    glColor3f(0.3f, 0.6f, 0.3f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    float size = 25.0f;
    glVertex3f(-size, groundLevel, -size);
    glVertex3f( size, groundLevel, -size);
    glVertex3f( size, groundLevel,  size);
    glVertex3f(-size, groundLevel,  size);
    glEnd();

    glColor3f(0.5f, 0.8f, 0.5f);
    glBegin(GL_LINES);
    for(int i = -size; i <= size; i++){
        glVertex3f(i, groundLevel + 0.01f, -size);
        glVertex3f(i, groundLevel + 0.01f, size);
        glVertex3f(-size, groundLevel + 0.01f, i);
        glVertex3f(size, groundLevel + 0.01f, i);
    }
    glEnd();
}

void PhysicsEngine3D::createCubeGrid(int pointsPerSide, float spacing){
    system = SpringMassSystem3D();
    currentRestLength = spacing;

    for(int z = 0; z < pointsPerSide; ++z){
        for(int y = 0; y < pointsPerSide; ++y){
            for(int x = 0; x < pointsPerSide; ++x){
                QVector3D position(
                    (x - (pointsPerSide-1)/2.0f) * spacing,
                    (y - (pointsPerSide-1)/2.0f) * spacing + 3.0f,
                    (z - (pointsPerSide-1)/2.0f) * spacing
                    );
                float mass = 1.0f;
                bool fixed = false;

                system.addPointMass(PointMass3D(position, mass, fixed));
            }
        }
    }

    int pointsPerLayer = pointsPerSide * pointsPerSide;

    for(int z = 0; z < pointsPerSide; ++z){
        for(int y = 0; y < pointsPerSide; ++y){
            for(int x = 0; x < pointsPerSide; ++x){
                int currentIndex = z * pointsPerLayer + y * pointsPerSide + x;

                if(x < pointsPerSide - 1){
                    int rightIndex = z * pointsPerLayer + y * pointsPerSide + (x + 1);
                    system.addSpring(Spring3D(currentIndex, rightIndex,
                                              spacing, currentStiffness, currentDamping));
                }
                if(y < pointsPerSide - 1){
                    int upIndex = z * pointsPerLayer + (y + 1) * pointsPerSide + x;
                    system.addSpring(Spring3D(currentIndex, upIndex,
                                              spacing, currentStiffness, currentDamping));
                }
                if(z < pointsPerSide - 1){
                    int forwardIndex = (z + 1) * pointsPerLayer + y * pointsPerSide + x;
                    system.addSpring(Spring3D(currentIndex, forwardIndex,
                                              spacing, currentStiffness, currentDamping));
                }

                if(x < pointsPerSide - 1 && y < pointsPerSide - 1){
                    int diagIndex = z * pointsPerLayer + (y + 1) * pointsPerSide + (x + 1);
                    float diagLength = spacing * std::sqrt(2.0f);
                    system.addSpring(Spring3D(currentIndex, diagIndex, diagLength,
                                              currentStiffness * 0.8f, currentDamping * 0.75f));
                }
                if(x > 0 && y < pointsPerSide - 1){
                    int diagIndex = z * pointsPerLayer + (y + 1) * pointsPerSide + (x - 1);
                    float diagLength = spacing * std::sqrt(2.0f);
                    system.addSpring(Spring3D(currentIndex, diagIndex, diagLength,
                                              currentStiffness * 0.8f, currentDamping * 0.75f));
                }

                if(x < pointsPerSide - 1 && z < pointsPerSide - 1){
                    int diagIndex = (z + 1) * pointsPerLayer + y * pointsPerSide + (x + 1);
                    float diagLength = spacing * std::sqrt(2.0f);
                    system.addSpring(Spring3D(currentIndex, diagIndex, diagLength,
                                              currentStiffness * 0.7f, currentDamping * 0.625f));
                }
                if(y < pointsPerSide - 1 && z < pointsPerSide - 1){
                    int diagIndex = (z + 1) * pointsPerLayer + (y + 1) * pointsPerSide + x;
                    float diagLength = spacing * std::sqrt(2.0f);
                    system.addSpring(Spring3D(currentIndex, diagIndex, diagLength,
                                              currentStiffness * 0.7f, currentDamping * 0.625f));
                }

                if(x < pointsPerSide - 1 && y < pointsPerSide - 1 && z < pointsPerSide - 1){
                    int diagIndex = (z + 1) * pointsPerLayer + (y + 1) * pointsPerSide + (x + 1);
                    float diagLength = spacing * std::sqrt(3.0f);
                    system.addSpring(Spring3D(currentIndex, diagIndex, diagLength,
                                              currentStiffness * 0.6f, currentDamping * 0.5f));
                }
            }
        }
    }

    for(int z = 0; z < pointsPerSide; ++z){
        for(int y = 0; y < pointsPerSide; ++y){
            for(int x = 0; x < pointsPerSide; ++x){
                int currentIndex = z * pointsPerLayer + y * pointsPerSide + x;

                if(x < pointsPerSide - 2){
                    int farIndex = z * pointsPerLayer + y * pointsPerSide + (x + 2);
                    system.addSpring(Spring3D(currentIndex, farIndex, spacing * 2.0f,
                                              currentStiffness * 0.4f, currentDamping * 0.375f));
                }
                if(y < pointsPerSide - 2){
                    int farIndex = z * pointsPerLayer + (y + 2) * pointsPerSide + x;
                    system.addSpring(Spring3D(currentIndex, farIndex, spacing * 2.0f,
                                              currentStiffness * 0.4f, currentDamping * 0.375f));
                }
                if(z < pointsPerSide - 2){
                    int farIndex = (z + 2) * pointsPerLayer + y * pointsPerSide + x;
                    system.addSpring(Spring3D(currentIndex, farIndex, spacing * 2.0f,
                                              currentStiffness * 0.4f, currentDamping * 0.375f));
                }
            }
        }
    }
}

void PhysicsEngine3D::setPointsPerSide(int count){
    currentPointsPerSide = count;
    resetSystem();
}

void PhysicsEngine3D::resetSystem(){
    if(clothMode){
        createCloth(20, 20, 0.2f);
    } else{
        createCubeGrid(currentPointsPerSide, currentRestLength);
    }

    selectedPoint = -1;
    isDragging = false;
}

void PhysicsEngine3D::handleCollisions(){
    auto& points = system.getPoints();
    const float groundRestitution = 0.4f;
    const float groundFriction = 0.7f;

    const float roomSize = 25.0f;
    const float ceilingHeight = 25.0f;
    const float wallRestitution = 0.6f;
    const float wallFriction = 0.4f;

    for(auto& point : points){
        if(selectedPoint != -1 && &point == &system.getPoints()[selectedPoint]){
            continue;
        }

        if(point.position.y() <= groundLevel){
            point.position.setY(groundLevel);
            point.velocity.setY(-point.velocity.y() * groundRestitution);
            point.velocity.setX(point.velocity.x() * groundFriction);
            point.velocity.setZ(point.velocity.z() * groundFriction);
        }

        if(point.position.y() >= ceilingHeight){
            point.position.setY(ceilingHeight);
            point.velocity.setY(-point.velocity.y() * wallRestitution);
            point.velocity.setX(point.velocity.x() * wallFriction);
            point.velocity.setZ(point.velocity.z() * wallFriction);
        }

        if(point.position.x() <= -roomSize){
            point.position.setX(-roomSize);
            point.velocity.setX(-point.velocity.x() * wallRestitution);
            point.velocity.setY(point.velocity.y() * wallFriction);
            point.velocity.setZ(point.velocity.z() * wallFriction);
        }

        if(point.position.x() >= roomSize){
            point.position.setX(roomSize);
            point.velocity.setX(-point.velocity.x() * wallRestitution);
            point.velocity.setY(point.velocity.y() * wallFriction);
            point.velocity.setZ(point.velocity.z() * wallFriction);
        }

        if(point.position.z() <= -roomSize){
            point.position.setZ(-roomSize);
            point.velocity.setZ(-point.velocity.z() * wallRestitution);
            point.velocity.setX(point.velocity.x() * wallFriction);
            point.velocity.setY(point.velocity.y() * wallFriction);
        }

        if(point.position.z() >= roomSize){
            point.position.setZ(roomSize);
            point.velocity.setZ(-point.velocity.z() * wallRestitution);
            point.velocity.setX(point.velocity.x() * wallFriction);
            point.velocity.setY(point.velocity.y() * wallFriction);
        }

        if(std::abs(point.velocity.y()) < 0.1f) point.velocity.setY(0);
        if(std::abs(point.velocity.x()) < 0.05f) point.velocity.setX(0);
        if(std::abs(point.velocity.z()) < 0.05f) point.velocity.setZ(0);
    }
}

void PhysicsEngine3D::mousePressEvent(QMouseEvent *event){
    lastMousePos = event->pos();

    if(event->button() == Qt::LeftButton){
        selectedPoint = findPointNearMouse(event->pos());
        if(selectedPoint != -1){
            isDragging = true;
            auto& points = system.getPoints();
            QVector3D pointPos = points[selectedPoint].position;
            QVector3D worldPos = screenToWorld(event->pos());
            mouseOffset = pointPos - worldPos;
            heldPosition = pointPos;
            points[selectedPoint].velocity = QVector3D(0, 0, 0);

            qDebug() << "Selected point at:" << pointPos << "Mouse offset:" << mouseOffset;
        } else{
            isRotating = true;
        }
    }
}

void PhysicsEngine3D::mouseMoveEvent(QMouseEvent *event){
    QPoint delta = event->pos() - lastMousePos;

    if(isDragging && selectedPoint != -1){
        auto& points = system.getPoints();

        QVector3D worldPos = screenToWorld(event->pos());

        heldPosition = worldPos + mouseOffset;
        points[selectedPoint].position = heldPosition;
        points[selectedPoint].velocity = QVector3D(0, 0, 0);

        update();

        qDebug() << "Dragging to:" << points[selectedPoint].position;
    } else if(isRotating){
        float sensitivity = 0.5f;

        QQuaternion rotX = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), -delta.x() * sensitivity);
        QQuaternion rotY = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), -delta.y() * sensitivity);

        cameraRotation = rotX * rotY * cameraRotation;
        update();
    }

    lastMousePos = event->pos();
}

void PhysicsEngine3D::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        isDragging = false;
        isRotating = false;
        if(selectedPoint != -1){
            selectedPoint = -1;
        }
    }
}

void PhysicsEngine3D::wheelEvent(QWheelEvent *event){
    cameraDistance *= (1.0f - event->angleDelta().y() * 0.001f);
    cameraDistance = std::max(2.0f, std::min(100.0f, cameraDistance));
    update();
}

void PhysicsEngine3D::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_R){
        resetSystem();
        update();
    }
}

int PhysicsEngine3D::findPointNearMouse(const QPoint& mousePos, float maxDistance){
    const auto& points = system.getPoints();
    int closestPoint = -1;
    float closestDistance = maxDistance;

    for(int i = 0; i < points.size(); ++i){
        QVector3D screenPos = worldToScreen(points[i].position);
        float distance = QVector2D(screenPos.x() - mousePos.x(), screenPos.y() - mousePos.y()).length();

        if(distance < closestDistance){
            closestDistance = distance;
            closestPoint = i;
        }
    }

    return closestPoint;
}

QVector3D PhysicsEngine3D::screenToWorld(const QPoint& screenPos){
    QVector3D cameraPos = cameraRotation.rotatedVector(QVector3D(0, 0, cameraDistance));
    QVector3D forward = (cameraTarget - cameraPos).normalized();
    QVector3D right = QVector3D::crossProduct(forward, cameraUp).normalized();
    QVector3D up = QVector3D::crossProduct(right, forward).normalized();

    float scale = cameraDistance * 0.3f;

    float mouseX = (screenPos.x() - width() / 2.0f) * scale / (width() * 0.5f);
    float mouseY = (height() / 2.0f - screenPos.y()) * scale / (height() * 0.5f);

    QVector3D pointInFront = cameraPos + forward * cameraDistance * 0.8f;

    pointInFront += right * mouseX + up * mouseY;

    return pointInFront;
}

QVector3D PhysicsEngine3D::worldToScreen(const QVector3D& worldPos){
    QVector4D clipPos = projection * view * QVector4D(worldPos, 1.0f);

    if(clipPos.w() == 0.0f) return QVector3D();

    QVector3D ndc = clipPos.toVector3D() / clipPos.w();

    return QVector3D(
        (ndc.x() + 1.0f) * 0.5f * width(),
        (1.0f - ndc.y()) * 0.5f * height(),
        ndc.z()
        );
}

void PhysicsEngine3D::updateSimulation(float deltaTime){
    if(selectedPoint != -1 && isDragging){
        auto& points = system.getPoints();
        points[selectedPoint].position = heldPosition;
        points[selectedPoint].velocity = QVector3D(0, 0, 0);
    }
    system.update(deltaTime);
    if(selectedPoint != -1 && isDragging){
        auto& points = system.getPoints();
        points[selectedPoint].position = heldPosition;
        points[selectedPoint].velocity = QVector3D(0, 0, 0);
    }
    handleCollisions();
    update();
}

void PhysicsEngine3D::setSpringStiffness(float stiffness){
    currentStiffness = stiffness;
    updateSpringParameters();
}

void PhysicsEngine3D::setSpringDamping(float damping){
    currentDamping = damping;
    updateSpringParameters();
}

void PhysicsEngine3D::setSpringRestLength(float restLength){
    currentRestLength = restLength;
    resetSystem();
}

void PhysicsEngine3D::updateSpringParameters(){
    auto& springs = system.getSprings();

    for(auto& spring : springs){
        float lengthRatio = spring.restLength / currentRestLength;

        if(std::abs(lengthRatio - 1.0f) < 0.01f){
            spring.stiffness = currentStiffness;
            spring.damping = currentDamping;
        }
        else if(std::abs(lengthRatio - std::sqrt(2.0f)) < 0.01f){
            spring.stiffness = currentStiffness * 0.8f;
            spring.damping = currentDamping * 0.75f;
        }
        else if(std::abs(lengthRatio - std::sqrt(3.0f)) < 0.01f){
            spring.stiffness = currentStiffness * 0.6f;
            spring.damping = currentDamping * 0.5f;
        }
        else if(std::abs(lengthRatio - 2.0f) < 0.01f){
            spring.stiffness = currentStiffness * 0.4f;
            spring.damping = currentDamping * 0.375f;
        }
    }

    update();
}

void PhysicsEngine3D::createCloth(int widthPoints, int heightPoints, float spacing){
    system = SpringMassSystem3D();
    clothMode = true;

    currentRestLength = spacing;

    for(int y = 0; y < heightPoints; ++y){
        for(int x = 0; x < widthPoints; ++x){
            QVector3D position(
                (x - (widthPoints-1)/2.0f) * spacing,
                4.0f - y * spacing * 0.5f,
                (0 - (heightPoints-1)/2.0f) * spacing * 0.5f
                );

            float mass = 1.0f;
            bool fixed = false;

            if(y == 0){
                fixed = true;
            }

            system.addPointMass(PointMass3D(position, mass, fixed));
        }
    }

    createClothSprings(widthPoints, heightPoints, spacing);

    qDebug() << "Created cloth with" << widthPoints << "x" << heightPoints << "points";
    qDebug() << "Total points:" << widthPoints * heightPoints;
    qDebug() << "Total springs:" << system.getSprings().size();
}

void PhysicsEngine3D::createClothSprings(int widthPoints, int heightPoints, float spacing){
    for(int y = 0; y < heightPoints; ++y){
        for(int x = 0; x < widthPoints; ++x){
            int currentIndex = y * widthPoints + x;

            if(x < widthPoints - 1){
                int rightIndex = y * widthPoints + (x + 1);
                system.addSpring(Spring3D(currentIndex, rightIndex,
                                          spacing, currentStiffness, currentDamping));
            }

            if(y < heightPoints - 1){
                int downIndex = (y + 1) * widthPoints + x;
                system.addSpring(Spring3D(currentIndex, downIndex,
                                          spacing, currentStiffness, currentDamping));
            }
        }
    }

    for(int y = 0; y < heightPoints; ++y){
        for(int x = 0; x < widthPoints; ++x){
            int currentIndex = y * widthPoints + x;

            if(x < widthPoints - 1 && y < heightPoints - 1){
                int diagIndex = (y + 1) * widthPoints + (x + 1);
                float diagLength = spacing * std::sqrt(2.0f);
                system.addSpring(Spring3D(currentIndex, diagIndex, diagLength,
                                          currentStiffness * 0.3f, currentDamping * 0.6f));
            }

            if(x > 0 && y < heightPoints - 1){
                int diagIndex = (y + 1) * widthPoints + (x - 1);
                float diagLength = spacing * std::sqrt(2.0f);
                system.addSpring(Spring3D(currentIndex, diagIndex, diagLength,
                                          currentStiffness * 0.3f, currentDamping * 0.6f));
            }
        }
    }

    for(int y = 0; y < heightPoints; ++y){
        for(int x = 0; x < widthPoints; ++x){
            int currentIndex = y * widthPoints + x;

            if(x < widthPoints - 2){
                int farRightIndex = y * widthPoints + (x + 2);
                system.addSpring(Spring3D(currentIndex, farRightIndex,
                                          spacing * 2.0f, currentStiffness * 0.2f, currentDamping * 0.5f));
            }

            if(y < heightPoints - 2){
                int farDownIndex = (y + 2) * widthPoints + x;
                system.addSpring(Spring3D(currentIndex, farDownIndex,
                                          spacing * 2.0f, currentStiffness * 0.2f, currentDamping * 0.5f));
            }
        }
    }

    for(int y = 0; y < heightPoints; ++y){
        for(int x = 0; x < widthPoints; ++x){
            int currentIndex = y * widthPoints + x;
            if(x < widthPoints - 2 && y < heightPoints - 2){
                int farDiagIndex = (y + 2) * widthPoints + (x + 2);
                float farDiagLength = spacing * std::sqrt(8.0f);
                system.addSpring(Spring3D(currentIndex, farDiagIndex, farDiagLength,
                                          currentStiffness * 0.1f, currentDamping * 0.4f));
            }
        }
    }
}
