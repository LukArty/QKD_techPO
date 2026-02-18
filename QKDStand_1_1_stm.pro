QT += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH +=\
    ./ \
    ./forms \
    ./headers \
    ./img \
    ./resources \
    ./sources \
    ./ftdi

QMAKE_CXXFLAGS += -O1

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# # Автоматическое определение архитектуры
# win32 {
#     # Проверяем, 64-битная ли система
#     !contains(QT_ARCH, x86_64) {
#         ARCH = x86
#         message("Building for Windows 32-bit")
#     } else {
#         ARCH = x64
#         message("Building for Windows 64-bit")
#     }

#     PLATFORM = win
#     LIB_EXT = .lib
#     DLL_EXT = .dll
#     LIB_NAME = ftd2xx
# }

# linux {
#     # Проверяем архитектуру Linux
#     contains(QT_ARCH, x86_64) {
#         ARCH = x86_64
#         message("Building for Linux 64-bit")
#     } else {
#         ARCH = i386
#         message("Building for Linux 32-bit")
#     }

#     PLATFORM = linux
#     LIB_EXT = .so
#     DLL_EXT = .so
#     LIB_NAME = ftd2xx
# }

# # Устанавливаем пути
# LIB_PATH = $$PWD/lib/$${PLATFORM}/$${ARCH}
# message("lib path -- $${LIB_PATH}")
# INCLUDE_PATH = $$PWD/include
# message("include path -- $${INCLUDE_PATH}")

# # Проверка существования библиотеки
# !exists($${LIB_PATH}/$${LIB_NAME}$${LIB_EXT}) {
#     error("FTDI library not found at: $${LIB_PATH}/$${LIB_NAME}$${LIB_EXT}")
# }

# # Настройка компилятора
# INCLUDEPATH += $${INCLUDE_PATH}
# DEPENDPATH += $${INCLUDE_PATH}

# # Настройка линковки
# LIBS += -L$${LIB_PATH}
# LIBS += -l$${LIB_NAME}
win32 {
    INCLUDEPATH += $$PWD/include
    LIBS += -L$$PWD/lib/win/x64
    LIBS += -lftd2xx
}


# # Для Windows: копирование DLL
# win32 {
#     # Путь к DLL
#     FTD2XX_DLL = $${LIB_PATH}/$${LIB_NAME}$${DLL_EXT}

#     # Копируем DLL в выходную директорию
#     QMAKE_POST_LINK += $$QMAKE_COPY \"$${FTD2XX_DLL}\" \"$${OUT_PWD}/$${DESTDIR}\"

#     # Добавляем определение
#     DEFINES += FT_WINDOWS
# }

# # Для Linux: настройка rpath
# linux {
#     QMAKE_RPATHDIR += $${LIB_PATH}
#     DEFINES += FT_LINUX
# }


FORMS += $$files(forms/*.ui)

DISTFILES += \
    img/19.png \
    img/Cube.png \
    img/PD.png \
    img/Polarisation_half_wave.png \
    img/Polarisation_quatro_wave.png \
    img/info-graphic.png

HEADERS += $$files(headers/*.h)

SOURCES += $$files(sources/*.cpp)

RESOURCES += \
    resource.qrc


