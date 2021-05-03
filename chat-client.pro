QT += core gui
QT += network
QT += concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chat_client.cpp \
    chat_user.cpp \
    crc32.cpp \
    main.cpp \
    send_file_dialog.cpp \
    socket_udp.cpp

HEADERS += \
    chat_client.h \
    chat_packet_udp.h \
    chat_user.h \
    crc32.h \
    send_file_dialog.h \
    socket_udp.h

FORMS += \
    chat_client.ui \
    send_file_dialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
