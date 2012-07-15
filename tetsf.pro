TEMPLATE = app
CONFIG += console
CONFIG -= qt

LIBS += -lSDL -lSDL_ttf

SOURCES += main.c \
    xmalloc.c \
    xerror.c \
    text.c \
    tetris.c \
    random.c \
    mainloop.c \
    highscores.c \
    gfx.c \
    game.c \
    autoplay.c \
    mouseinput.c \
    copypaste.c

OTHER_FILES += \
    icon.rc

HEADERS += \
    xmalloc.h \
    xerror.h \
    text.h \
    tetris.h \
    random.h \
    mainloop.h \
    highscores.h \
    gfx.h \
    game.h \
    common.h \
    autoplay.h \
    mouseinput.h \
    copypaste.h

