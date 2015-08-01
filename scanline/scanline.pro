TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

include(deployment.pri)
qtcAddDeployment()


LIBS += -lglut
LIBS += -lGL
LIBS += -lGLU
