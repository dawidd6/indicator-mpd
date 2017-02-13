#Assembled by dawidd6
COMPILER=gcc
CFLAGS=-Wall -std=c11 `pkg-config --libs --cflags appindicator3-0.1 libmpdclient`
PROGRAM=indicator-mpd
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)
START_COLOR=\033[0;33m
CLOSE_COLOR=\033[m

src/%.o: src/%.c
	@echo "$(START_COLOR)[CC]$(CLOSE_COLOR)   $<"
	@$(COMPILER) -c -o $@ $< $(CFLAGS)

$(PROGRAM): $(OBJ)
	@echo "$(START_COLOR)[LD]$(CLOSE_COLOR)   $@"
	@$(COMPILER) -o $@ $^ $(CFLAGS)

install:
	@echo "$(START_COLOR)[INSTALL]$(CLOSE_COLOR)   /usr/bin/$(PROGRAM)"
	@echo "$(START_COLOR)[INSTALL]$(CLOSE_COLOR)   /usr/share/applications/$(PROGRAM).desktop"
	@echo "$(START_COLOR)[INSTALL]$(CLOSE_COLOR)   /etc/xdg/autostart/$(PROGRAM).desktop"
	@install $(PROGRAM) /usr/bin
	@install $(PROGRAM).desktop /usr/share/applications
	@install $(PROGRAM).desktop /etc/xdg/autostart

uninstall:
	@echo "$(START_COLOR)[RM]$(CLOSE_COLOR)   /usr/bin/$(PROGRAM)"
	@echo "$(START_COLOR)[RM]$(CLOSE_COLOR)   /usr/share/applications/$(PROGRAM).desktop"
	@echo "$(START_COLOR)[RM]$(CLOSE_COLOR)   /etc/xdg/autostart/$(PROGRAM).desktop"
	@rm -rf /usr/bin/$(PROGRAM)
	@rm -rf /usr/share/applications/$(PROGRAM).desktop
	@rm -rf /etc/xdg/autostart/$(PROGRAM).desktop

clean:
	@echo "$(START_COLOR)[RM]$(CLOSE_COLOR)   $(OBJ) $(PROGRAM)"
	@rm -rf $(OBJ) $(PROGRAM)

.PHONY: install uninstall clean
