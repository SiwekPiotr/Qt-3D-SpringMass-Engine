#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QHBoxLayout>
#include "physicsengine3d.h"

class QLabel;
class QSlider;
class QGroupBox;
class QDoubleSpinBox;
class QSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateSimulation();

    void updateStiffnessFromSlider(int value);
    void updateDampingFromSlider(int value);
    void updateRestLengthFromSlider(int value);
    void updateTimeStepFromSlider(int value);
    void updatePointMassFromSlider(int value);
    void updatePointsFromSlider(int value);

    void updateStiffnessFromSpinBox(double value);
    void updateDampingFromSpinBox(double value);
    void updateRestLengthFromSpinBox(double value);
    void updateTimeStepFromSpinBox(double value);
    void updatePointMassFromSpinBox(double value);
    void updatePointsFromSpinBox(int value);

    void updateParameterValues();
    void switchMode(bool cloth);

private:
    PhysicsEngine3D* physicsEngine;
    QTimer* simulationTimer;

    float timeStep;

    QSlider* stiffnessSlider;
    QSlider* dampingSlider;
    QSlider* restLengthSlider;
    QSlider* timeStepSlider;
    QSlider* pointMassSlider;
    QSlider* pointsSlider;

    QDoubleSpinBox* stiffnessSpinBox;
    QDoubleSpinBox* dampingSpinBox;
    QDoubleSpinBox* restLengthSpinBox;
    QDoubleSpinBox* timeStepSpinBox;
    QDoubleSpinBox* pointMassSpinBox;
    QSpinBox* pointsSpinBox;

    QGroupBox* createSpringControls();
    QHBoxLayout* createIntSpinBoxRow(const QString& title, QSlider*& slider, QSpinBox*& spinBox, int min, int max, int defaultVal);
    QHBoxLayout* createDoubleSpinBoxRow(const QString& title, QSlider*& slider, QDoubleSpinBox*& spinBox, double min, double max, double defaultVal, double step, int decimals);
};
#endif
