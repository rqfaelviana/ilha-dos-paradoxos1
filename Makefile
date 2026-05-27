CC     = cc
UNAME := $(shell uname)

# ── macOS ──────────────────────────────────────────────────────────
ifeq ($(UNAME), Darwin)
    # Homebrew coloca libs aqui (Intel e Apple Silicon)
    BREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo /usr/local)
    CFLAGS  = -std=c11 -Wall -Wextra -O2 \
              -I$(BREW_PREFIX)/include
    LDFLAGS = -L$(BREW_PREFIX)/lib -lraylib \
              -framework OpenGL -framework Cocoa \
              -framework IOKit -framework CoreVideo \
              -lm
endif

# ── Linux ──────────────────────────────────────────────────────────
ifeq ($(UNAME), Linux)
    CFLAGS  = -std=c11 -Wall -Wextra -O2 -I/usr/local/include
    LDFLAGS = -L/usr/local/lib -lraylib -lGL -lX11 -lm -lpthread -ldl
endif

# ── Windows (MinGW-w64) ────────────────────────────────────────────
# Usado quando compilado via MinGW no Windows (variável OS=Windows_NT).
# Requer Raylib para Windows extraído em C:/raylib
# Ajuste RAYLIB_PATH abaixo se extraiu em outro local.
ifeq ($(OS), Windows_NT)
    CC          = gcc
    RAYLIB_PATH = C:/raylib
    CFLAGS      = -std=c11 -Wall -Wextra -O2 -I$(RAYLIB_PATH)/include
    LDFLAGS     = -L$(RAYLIB_PATH)/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET      = ilha_dos_paradoxos.exe
endif

TARGET = ilha_dos_paradoxos
SRCDIR = src
SRCS   = $(SRCDIR)/main.c \
         $(SRCDIR)/game.c \
         $(SRCDIR)/menu.c \
         $(SRCDIR)/map.c \
         $(SRCDIR)/player.c \
         $(SRCDIR)/npc.c \
         $(SRCDIR)/dialogue.c \
         $(SRCDIR)/deduction.c \
         $(SRCDIR)/ui.c \
         $(SRCDIR)/environment.c \
         $(SRCDIR)/utils.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo ""
	@echo "=========================================="
	@echo "  Ilha dos Paradoxos compilado com sucesso"
	@echo "  Execute:  ./$(TARGET)"
	@echo "=========================================="

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/game.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
