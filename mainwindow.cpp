#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSignalBlocker>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    physicsEngine(new PhysicsEngine3D(this)),
    simulationTimer(new QTimer(this)),
    timeStep(0.016f)
{
    QWidget *centralWidget = new QWidget(this);
    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    mainLayout->addWidget(physicsEngine, 0, 0);

    QWidget *leftPanel = new QWidget(this);
    leftPanel->setObjectName("leftPanel");
    QVBoxLayout *leftPanelLayout = new QVBoxLayout(leftPanel);

    QGroupBox *modeGroup = new QGroupBox("Simulation Control", leftPanel);
    QVBoxLayout *modeLayout = new QVBoxLayout(modeGroup);

    QPushButton *resetButton = new QPushButton("Reset System (R)", this);
    QPushButton *cubeModeButton = new QPushButton("Cube Mode", this);
    QPushButton *clothModeButton = new QPushButton("Cloth Mode", this);

    modeLayout->addWidget(resetButton);
    modeLayout->addWidget(cubeModeButton);
    modeLayout->addWidget(clothModeButton);

    leftPanelLayout->addWidget(modeGroup);
    leftPanelLayout->addWidget(createSpringControls());

    QWidget *overlayContainer = new QWidget(this);
    QVBoxLayout *containerLayout = new QVBoxLayout(overlayContainer);
    containerLayout->addWidget(leftPanel);

    mainLayout->addWidget(overlayContainer, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    leftPanel->raise();

    setCentralWidget(centralWidget);

    simulationTimer->setTimerType(Qt::PreciseTimer);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    simulationTimer->start(16);

    connect(resetButton, &QPushButton::clicked, physicsEngine, &PhysicsEngine3D::resetSystem);

    connect(cubeModeButton, &QPushButton::clicked, this, [this](){
        switchMode(false);
    });

    connect(clothModeButton, &QPushButton::clicked, this, [this](){
        switchMode(true);
    });

    updateParameterValues();
}

MainWindow::~MainWindow(){
}

void MainWindow::switchMode(bool cloth){
    physicsEngine->setSimulationMode(cloth);
    physicsEngine->resetSystem();
    updateParameterValues();
}

void MainWindow::updateSimulation(){
    physicsEngine->updateSimulation(timeStep);
}

QHBoxLayout* MainWindow::createIntSpinBoxRow(const QString& title, QSlider*& slider, QSpinBox*& spinBox, int min, int max, int defaultVal){
    QHBoxLayout *rowLayout = new QHBoxLayout();
    rowLayout->setSpacing(8);

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setMinimumWidth(85);

    spinBox = new QSpinBox(this);
    spinBox->setRange(min, max);
    spinBox->setValue(defaultVal);
    spinBox->setFixedWidth(65);
    spinBox->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(min, max);
    slider->setValue(defaultVal);

    rowLayout->addWidget(titleLabel);
    rowLayout->addWidget(slider);
    rowLayout->addWidget(spinBox);
    return rowLayout;
}

QHBoxLayout* MainWindow::createDoubleSpinBoxRow(const QString& title, QSlider*& slider, QDoubleSpinBox*& spinBox, double min, double max, double defaultVal, double step, int decimals){
    QHBoxLayout *rowLayout = new QHBoxLayout();
    rowLayout->setSpacing(8);

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setMinimumWidth(85);

    spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(min, max);
    spinBox->setValue(defaultVal);
    spinBox->setSingleStep(step);
    spinBox->setDecimals(decimals);
    spinBox->setFixedWidth(65);
    spinBox->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    slider = new QSlider(Qt::Horizontal, this);
    double factor = std::pow(10.0, decimals);
    slider->setRange(static_cast<int>(std::round(min * factor)), static_cast<int>(std::round(max * factor)));
    slider->setValue(static_cast<int>(std::round(defaultVal * factor)));

    rowLayout->addWidget(titleLabel);
    rowLayout->addWidget(slider);
    rowLayout->addWidget(spinBox);
    return rowLayout;
}

QGroupBox* MainWindow::createSpringControls(){
    QGroupBox *groupBox = new QGroupBox("Spring Parameters", this);
    QVBoxLayout *layout = new QVBoxLayout(groupBox);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 14, 10, 10);

    layout->addLayout(createDoubleSpinBoxRow("Stiffness:", stiffnessSlider, stiffnessSpinBox, 100, 1000, 600, 10, 1));
    layout->addLayout(createDoubleSpinBoxRow("Damping:", dampingSlider, dampingSpinBox, 1, 20, 8, 0.5, 1));
    layout->addLayout(createDoubleSpinBoxRow("Rest Length:", restLengthSlider, restLengthSpinBox, 1, 10, 3, 0.1, 2));
    layout->addLayout(createDoubleSpinBoxRow("Time Step (dt):", timeStepSlider, timeStepSpinBox, 0.001, 0.033, 0.016, 0.001, 3));
    layout->addLayout(createIntSpinBoxRow("Points per side:", pointsSlider, pointsSpinBox, 4, 15, 4));

    connect(stiffnessSlider, &QSlider::valueChanged, this, &MainWindow::updateStiffnessFromSlider);
    connect(dampingSlider, &QSlider::valueChanged, this, &MainWindow::updateDampingFromSlider);
    connect(restLengthSlider, &QSlider::valueChanged, this, &MainWindow::updateRestLengthFromSlider);
    connect(timeStepSlider, &QSlider::valueChanged, this, &MainWindow::updateTimeStepFromSlider);
    connect(pointsSlider, &QSlider::valueChanged, this, &MainWindow::updatePointsFromSlider);

    connect(stiffnessSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateStiffnessFromSpinBox);
    connect(dampingSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateDampingFromSpinBox);
    connect(restLengthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateRestLengthFromSpinBox);
    connect(timeStepSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateTimeStepFromSpinBox);
    connect(pointsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updatePointsFromSpinBox);

    return groupBox;
}

void MainWindow::updateStiffnessFromSlider(int value){
    float stiffness = static_cast<float>(value) / 10.0f;
    physicsEngine->setSpringStiffness(stiffness);
    QSignalBlocker blocker(stiffnessSpinBox);
    stiffnessSpinBox->setValue(stiffness);
}

void MainWindow::updateDampingFromSlider(int value){
    float damping = static_cast<float>(value) / 10.0f;
    physicsEngine->setSpringDamping(damping);
    QSignalBlocker blocker(dampingSpinBox);
    dampingSpinBox->setValue(damping);
}

void MainWindow::updateRestLengthFromSlider(int value){
    float restLength = static_cast<float>(value) / 100.0f;
    physicsEngine->setSpringRestLength(restLength);
    QSignalBlocker blocker(restLengthSpinBox);
    restLengthSpinBox->setValue(restLength);
}

void MainWindow::updateTimeStepFromSlider(int value){
    timeStep = static_cast<float>(value) / 1000.0f;
    QSignalBlocker blocker(timeStepSpinBox);
    timeStepSpinBox->setValue(timeStep);
}

void MainWindow::updatePointsFromSlider(int value){
    physicsEngine->setPointsPerSide(value);
    QSignalBlocker blocker(pointsSpinBox);
    pointsSpinBox->setValue(value);
}

void MainWindow::updateStiffnessFromSpinBox(double value){
    float stiffness = static_cast<float>(value);
    physicsEngine->setSpringStiffness(stiffness);
    QSignalBlocker blocker(stiffnessSlider);
    stiffnessSlider->setValue(static_cast<int>(std::round(value * 10.0)));
}

void MainWindow::updateDampingFromSpinBox(double value){
    float damping = static_cast<float>(value);
    physicsEngine->setSpringDamping(damping);
    QSignalBlocker blocker(dampingSlider);
    dampingSlider->setValue(static_cast<int>(std::round(value * 10.0)));
}

void MainWindow::updateRestLengthFromSpinBox(double value){
    float restLength = static_cast<float>(value);
    physicsEngine->setSpringRestLength(restLength);
    QSignalBlocker blocker(restLengthSlider);
    restLengthSlider->setValue(static_cast<int>(std::round(value * 100.0)));
}

void MainWindow::updateTimeStepFromSpinBox(double value){
    timeStep = static_cast<float>(value);
    QSignalBlocker blocker(timeStepSlider);
    timeStepSlider->setValue(static_cast<int>(std::round(value * 1000.0)));
}

void MainWindow::updatePointsFromSpinBox(int value){
    physicsEngine->setPointsPerSide(value);
    QSignalBlocker blocker(pointsSlider);
    pointsSlider->setValue(value);
}

void MainWindow::updateParameterValues(){
    updateStiffnessFromSpinBox(stiffnessSpinBox->value());
    updateDampingFromSpinBox(dampingSpinBox->value());
    updateRestLengthFromSpinBox(restLengthSpinBox->value());
    updatePointsFromSpinBox(pointsSpinBox->value());
}