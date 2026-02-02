QT       += core gui multimedia multimediawidgets serialport bluetooth

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

QMAKE_CXXFLAGS += -Wno-deprecated-copy

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += DDESKTOP_ENABLE

SOURCES += \
    bluetooth.cpp \
    main.cpp \
    mainwindow.cpp \
    serialport.cpp

HEADERS += \
    bluetooth.h \
    mainwindow.h \
    serialport.h

FORMS += \
    bluetooth.ui \
    mainwindow.ui \
    serialport.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
