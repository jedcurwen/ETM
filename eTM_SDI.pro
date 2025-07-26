QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
QT += sql
QT += widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += src/
INCLUDEPATH += $$PWD/lib
SOURCES += \
    src/company.cpp \
    src/createorder.cpp \
    src/createuser.cpp \
    src/driverui.cpp \
    src/editdetails.cpp \
    src/leavefeedback.cpp \
    src/loggedin.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mosquittothread.cpp \
    src/passwordreset.cpp \
    src/thread.cpp\

HEADERS += \
    src/company.h \
    src/createorder.h \
    src/createuser.h \
    src/driverui.h \
    src/editdetails.h \
    src/leavefeedback.h \
    src/loggedin.h \
    src/mainwindow.h \
    src/mosquittothread.h \
    src/passwordreset.h \
    src/thread.h\
    src/mosquitto.h\

FORMS += \
    src/company.ui \
    src/createorder.ui \
    src/createuser.ui \
    src/driverui.ui \
    src/editdetails.ui \
    src/leavefeedback.ui \
    src/loggedin.ui \
    src/mainwindow.ui \
    src/passwordreset.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
DISTFILES += \
    lib/mosquitto.lib \

LIBS += -L$$PWD/lib -lmosquitto

macx{
    INCLUDEPATH += \
    "/opt/homebrew/cellar/boost/1.80.0/include" \
    "/opt/homebrew/Cellar/mosquitto/2.0.15/include"
    LIBS += \
    -L"/opt/homebrew/cellar/boost/1.80.0/lib" \
    -L"/opt/homebrew/Cellar/mosquitto/2.0.15/lib"
}
