QT       += core gui widgets openglwidgets opengl

CONFIG   += c++17

TARGET = PhysicsEngine3D
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    physicsengine3d.cpp \
    springmasssystem3d.cpp

HEADERS += \
    mainwindow.h \
    physicsengine3d.h \
    springmasssystem3d.h

QT_OPENGL = desktop

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

QMAKE_CXXFLAGS += -O2

win32:qtHaveModule(opengl):!qtHaveModule(angle) {
    LIBS += -lopengl32
} else:qtHaveModule(angle) {
    LIBS += -lGLESv2
}

DISTFILES += \
    .gitignore
