#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    physicsEngine(new PhysicsEngine3D(this)),
    simulationTimer(new QTimer(this))
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
    physicsEngine->updateSimulation(0.016f);
}

QHBoxLayout* MainWindow::createSliderRow(const QString& title, QSlider*& slider, QLabel*& valueLabel, int min, int max, int defaultVal){
    QHBoxLayout *rowLayout = new QHBoxLayout();
    rowLayout->setSpacing(8);

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setMinimumWidth(85);

    valueLabel = new QLabel(QString::number(defaultVal), this);
    valueLabel->setMinimumWidth(45);
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(min, max);
    slider->setValue(defaultVal);

    rowLayout->addWidget(titleLabel);
    rowLayout->addWidget(slider);
    rowLayout->addWidget(valueLabel);
    return rowLayout;
}

QGroupBox* MainWindow::createSpringControls(){
    QGroupBox *groupBox = new QGroupBox("Spring Parameters", this);
    QVBoxLayout *layout = new QVBoxLayout(groupBox);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 14, 10, 10);

    layout->addLayout(createSliderRow("Stiffness:", stiffnessSlider, stiffnessValueLabel, 100, 1000, 600));
    layout->addLayout(createSliderRow("Damping:", dampingSlider, dampingValueLabel, 1, 20, 8));
    layout->addLayout(createSliderRow("Rest Length:", restLengthSlider, restLengthValueLabel, 10, 100, 30));
    layout->addLayout(createSliderRow("Points per side:", pointsSlider, pointsValueLabel, 4, 15, 4));

    connect(stiffnessSlider, &QSlider::valueChanged, this, &MainWindow::updateStiffness);
    connect(dampingSlider, &QSlider::valueChanged, this, &MainWindow::updateDamping);
    connect(restLengthSlider, &QSlider::valueChanged, this, &MainWindow::updateRestLength);
    connect(pointsSlider, &QSlider::valueChanged, this, &MainWindow::updatePointsPerSide);

    return groupBox;
}

void MainWindow::updateStiffness(int value){
    float stiffness = static_cast<float>(value);
    physicsEngine->setSpringStiffness(stiffness);
    stiffnessValueLabel->setText(QString::number(stiffness, 'f', 1));
}

void MainWindow::updateDamping(int value){
    float damping = static_cast<float>(value);
    physicsEngine->setSpringDamping(damping);
    dampingValueLabel->setText(QString::number(damping, 'f', 1));
}

void MainWindow::updateRestLength(int value){
    float restLength = static_cast<float>(value) / 10.0f;
    physicsEngine->setSpringRestLength(restLength);
    restLengthValueLabel->setText(QString::number(restLength, 'f', 2));
}

void MainWindow::updatePointsPerSide(int value){
    physicsEngine->setPointsPerSide(value);
    pointsValueLabel->setText(QString::number(value));
}

void MainWindow::updateParameterValues(){
    updateStiffness(stiffnessSlider->value());
    updateDamping(dampingSlider->value());
    updateRestLength(restLengthSlider->value());
    updatePointsPerSide(pointsSlider->value());
}

