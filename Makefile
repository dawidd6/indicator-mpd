PROGRAM=indicator-mpd
all:
	@echo "[CC+LD]    $(PROGRAM)"
	@gcc $(PROGRAM).c -o $(PROGRAM) \
	`pkg-config --libs gtk+-3.0 --cflags gtk+-3.0` \
	`pkg-config --libs appindicator3-0.1 --cflags appindicator3-0.1` \
	`pkg-config --libs libmpdclient --cflags libmpdclient`

install:
	install $(PROGRAM) /usr/bin
	install $(PROGRAM).desktop /usr/share/applications
	install $(PROGRAM).desktop /etc/xdg/autostart

uninstall:
	rm -f /usr/bin/$(PROGRAM)
	rm -f /usr/share/applications/$(PROGRAM).desktop
	rm -f /etc/xdg/autostart/$(PROGRAM).desktop

.PHONY: install uninstall
