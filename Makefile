LIB_NAME = libparagraph
LIB = $(LIB_NAME).a

PREFIX ?= /usr
export PKG_CONFIG_PATH:=${PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}::
CC = gcc
LD = gcc
CFLAGS = \
	--std=c99 -g -Wall -Wextra -fanalyzer \
	`pkg-config sdl2 --cflags` \
	`pkg-config freetype2 --cflags` \
	`pkg-config harfbuzz --cflags` \
	`pkg-config libcss --cflags` \
	`pkg-config libdom --cflags` \
	-I include
LFLAGS = \
	`pkg-config sdl2 --libs` \
	`pkg-config freetype2 --libs` \
	`pkg-config harfbuzz --libs` \
	`pkg-config libcss --libs` \
	`pkg-config libdom --libs`

MKDIR=mkdir -p

TARGET=test
BUILDDIR=build/$(TARGET)

SOURCES_PARAGRAPH = \
	ctx.c \
	para.c \
	util.c \
	style.c \
	content.c

SRC_PARAGRAPH := $(addprefix src/,$(SOURCES_PARAGRAPH))
OBJ_PARAGRAPH = $(patsubst %.c,%.o, $(addprefix $(BUILDDIR)/,$(SRC_PARAGRAPH)))

SOURCES_STYLED_DOC = \
	sd.c \
	doc.c \
	file.c \
	sheet.c \
	style.c

SRC_STYLED_DOC := $(addprefix test/styled-doc/src/,$(SOURCES_STYLED_DOC))
OBJ_STYLED_DOC = $(patsubst %.c,%.o, $(addprefix $(BUILDDIR)/,$(SRC_STYLED_DOC)))

OBJ = $(OBJ_STYLED_DOC) $(OBJ_PARAGRAPH)

all: $(BUILDDIR)/basic

$(BUILDDIR)/basic: test/basic.c $(OBJ_STYLED_DOC) $(OBJ_PARAGRAPH)
		$(LD) -Itest/styled-doc/include $(CFLAGS) -o $@ $^ $(LFLAGS)

$(OBJ): $(BUILDDIR)/%.o : %.c
		@$(MKDIR) $(basename $@)
		$(CC) -Itest/styled-doc/include $(CFLAGS) -c -o $@ $<

clean:
	rm -rf build
