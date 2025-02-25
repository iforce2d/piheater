QT       += core gui opengl openglwidgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    configurephasedialog.cpp \
    configureprofiledialog.cpp \
    hardware.cpp \
    heatprofile.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    settings.cpp \
    settingsdialog.cpp

HEADERS += \
    configurephasedialog.h \
    configureprofiledialog.h \
    hardware.h \
    heatprofile.h \
    mainwindow.h \
    qcustomplot.h \
    settings.h \
    settingsdialog.h

FORMS += \
    configurephasedialog.ui \
    configureprofiledialog.ui \
    mainwindow.ui \
    settingsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
