# ----------------------------
# Makefile Options
# ----------------------------

NAME = TEST
ICON = icon.png
DESCRIPTION = "Moves a square around the screen"
COMPRESSED = NO
ARCHIVED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)