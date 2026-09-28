#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QHBoxLayout>
#include "physicsengine3d.h"

class QLabel;
class QSlider;
class QGroupBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateSimulation();
    void updateStiffness(int value);
    void updateDamping(int value);
    void updateRestLength(int value);
    void updateParameterValues();
    void updatePointsPerSide(int value);
    void switchMode(bool cloth);

private:
    PhysicsEngine3D* physicsEngine;
    QTimer* simulationTimer;

    QSlider* stiffnessSlider;
    QSlider* dampingSlider;
    QSlider* restLengthSlider;
    QSlider* pointsSlider;

    QLabel* stiffnessValueLabel;
    QLabel* dampingValueLabel;
    QLabel* restLengthValueLabel;
    QLabel* pointsValueLabel;

    QGroupBox* createSpringControls();
    QHBoxLayout* createSliderRow(const QString& title, QSlider*& slider, QLabel*& valueLabel, int min, int max, int defaultVal);
};
#endif
