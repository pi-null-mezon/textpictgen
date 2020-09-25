QT += widgets # we need to use widgets to apply blur effect to QImage

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET=textpictgen
VERSION=0.0.0.0

DEFINES += APP_NAME=\\\"$${TARGET}\\\" \
           APP_VERSION=\\\"$${VERSION}\\\" \
           APP_DESIGNER=\\\"Alex_A.Taranov\\\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /usr/local/bin
!isEmpty(target.path): INSTALLS += target
