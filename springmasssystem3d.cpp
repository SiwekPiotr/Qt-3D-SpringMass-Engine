#include "springmasssystem3d.h"

SpringMassSystem3D::SpringMassSystem3D() :
    gravity(0.0f, -9.81f, 0.0f){
}

void SpringMassSystem3D::addPointMass(const PointMass3D& point){
    points.push_back(point);
}

void SpringMassSystem3D::addSpring(const Spring3D& spring){
    springs.push_back(spring);
}

void SpringMassSystem3D::update(float deltaTime){
    applyForces();
    integrate(deltaTime);
}

void SpringMassSystem3D::applyForces(){
    for(auto& point : points){
        point.force = QVector3D(0, 0, 0);

        if(!point.fixed && point.mass > 0){
            point.force += gravity * point.mass;
        }
    }

    for(const auto& spring : springs){
        if(spring.pointA >= points.size() || spring.pointB >= points.size()) continue;

        PointMass3D& pointA = points[spring.pointA];
        PointMass3D& pointB = points[spring.pointB];

        QVector3D delta = pointB.position - pointA.position;
        float currentLength = delta.length();

        if(currentLength > 0){
            QVector3D forceDirection = delta / currentLength;

            float displacement = currentLength - spring.restLength;
            QVector3D springForce = forceDirection * (spring.stiffness * displacement);

            if(!pointA.fixed) pointA.force += springForce;
            if(!pointB.fixed) pointB.force -= springForce;

            if(spring.damping > 0){
                QVector3D relativeVel = pointB.velocity - pointA.velocity;
                float velProjection = QVector3D::dotProduct(forceDirection, relativeVel);
                QVector3D dampingForce = forceDirection * velProjection * spring.damping;

                if(!pointA.fixed) pointA.force += dampingForce;
                if(!pointB.fixed) pointB.force -= dampingForce;
            }
        }
    }
}

void SpringMassSystem3D::integrate(float deltaTime){
    for(auto& point : points){
        if(point.fixed || point.mass <= 0) continue;
        // a = F/m
        QVector3D acceleration = point.force / point.mass;
        // v = v + a * dt
        point.velocity += acceleration * deltaTime;
        // x = x + v * dt
        point.position += point.velocity * deltaTime;
    }
}
