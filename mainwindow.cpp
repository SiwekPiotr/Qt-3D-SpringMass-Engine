#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    physicsEngine(new PhysicsEngine3D(this)),
    simulationTimer(new QTimer(this))
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *resetButton = new QPushButton("Reset System (R)", this);
    QPushButton *cubeModeButton = new QPushButton("Cube Mode", this);
    QPushButton *clothModeButton = new QPushButton("Cloth Mode", this);

    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(cubeModeButton);
    buttonLayout->addWidget(clothModeButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(createSpringControls());
    mainLayout->addWidget(physicsEngine);

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
    QLabel *titleLabel = new QLabel(title, this);
    valueLabel = new QLabel(QString::number(defaultVal), this);
    valueLabel->setMinimumWidth(60);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(min, max);
    slider->setValue(defaultVal);
    slider->setTickPosition(QSlider::TicksBelow);

    rowLayout->addWidget(titleLabel);
    rowLayout->addWidget(slider);
    rowLayout->addWidget(valueLabel);
    return rowLayout;
}

QGroupBox* MainWindow::createSpringControls(){
    QGroupBox *groupBox = new QGroupBox("Spring Parameters (Real-time Control)", this);
    QVBoxLayout *layout = new QVBoxLayout();

    layout->addLayout(createSliderRow("Stiffness:", stiffnessSlider, stiffnessValueLabel, 100, 1000, 600));
    layout->addLayout(createSliderRow("Damping:", dampingSlider, dampingValueLabel, 1, 20, 8));
    layout->addLayout(createSliderRow("Rest Length:", restLengthSlider, restLengthValueLabel, 10, 100, 30));
    layout->addLayout(createSliderRow("Points per side:", pointsSlider, pointsValueLabel, 4, 15, 4));

    groupBox->setLayout(layout);
    groupBox->setMaximumHeight(150);

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

