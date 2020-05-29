# add this line to kill vncserver after exit
# -gone 'killall Xvfb' \
x11vnc -create -env FD_PROG=/usr/bin/fluxbox \
    -env X11VNC_FINDDISPLAY_ALWAYS_FAILS=1 \
        -env X11VNC_CREATE_GEOM=${1:-1920x1080x16} \
        -bg -nopw
