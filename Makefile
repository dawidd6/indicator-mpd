all:
	g++ indicator-mpd.cpp -o indicator-mpd \
	`pkg-config --libs gtk+-3.0 --cflags gtk+-3.0` \
	`pkg-config --libs appindicator3-0.1 --cflags appindicator3-0.1` \
	`pkg-config --libs libmpdclient --cflags libmpdclient`
