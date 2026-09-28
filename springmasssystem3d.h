#ifndef SPRINGMASSYSTEM3D_H
#define SPRINGMASSYSTEM3D_H

#include <QVector3D>
#include <vector>
#include <cmath>

struct PointMass3D {
    QVector3D position;
    QVector3D velocity;
    QVector3D force;
    float mass;
    bool fixed;

    PointMass3D(const QVector3D& pos, float m, bool isFixed = false)
        : position(pos), velocity(0, 0, 0), force(0, 0, 0), mass(m), fixed(isFixed) {}
};

struct Spring3D {
    int pointA, pointB;
    float restLength;
    float stiffness;
    float damping;

    Spring3D(int a, int b, float rest, float k, float d)
        : pointA(a), pointB(b), restLength(rest), stiffness(k), damping(d) {}
};

class SpringMassSystem3D {
public:
    SpringMassSystem3D();

    void addPointMass(const PointMass3D& point);
    void addSpring(const Spring3D& spring);
    void update(float deltaTime);

    std::vector<PointMass3D>& getPoints() { return points; }
    std::vector<Spring3D>& getSprings() { return springs; }
    const std::vector<PointMass3D>& getPoints() const { return points; }
    const std::vector<Spring3D>& getSprings() const { return springs; }

private:
    void applyForces();
    void integrate(float deltaTime);

    std::vector<PointMass3D> points;
    std::vector<Spring3D> springs;

    QVector3D gravity;
};

#endif
