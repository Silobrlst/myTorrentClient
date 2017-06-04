#include "mainwindow.h"
#include <QApplication>
#include <qdebug.h>
#include <qmessagebox.h>

#include <stdlib.h>
#include </home/q/Загрузки/boost_1_64_0/boost/make_shared.hpp>
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/settings.hpp"
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_status.hpp>
#include "libtorrent/announce_entry.hpp"
#include "libtorrent/bdecode.hpp"
#include "libtorrent/magnet_uri.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

using namespace libtorrent;
namespace lt = libtorrent;
using clk = std::chrono::steady_clock;


Window window_from_name_search(Display *display, Window current, char const *needle) {
    Window retval, root, parent, *children;
    unsigned children_count;
    char *name = NULL;

    /* Check if this window has the name we seek */
    if(XFetchName(display, current, &name) > 0) {
        int r = strcmp(needle, name);
        XFree(name);
        if(r == 0) {
            return current;
        }
    }

    retval = 0;

    /* If it does not: check all subwindows recursively. */
    if(0 != XQueryTree(display, current, &root, &parent, &children, &children_count)) {
        unsigned i;
        for(i = 0; i < children_count; ++i) {
            Window win = window_from_name_search(display, children[i], needle);

            if(win != 0) {
                retval = win;
                break;
            }
        }

        XFree(children);
    }

    return retval;
}

// frontend function: open display connection, start searching from the root window.
Window window_from_name(char const *name) {
    Display *display = XOpenDisplay(NULL);
    Window w = window_from_name_search(display, XDefaultRootWindow(display), name);
    XCloseDisplay(display);
    return w;
}


int main(int argc, char *argv[])
{
    //loadTorrent("[rutracker.org].t538269.torrent", &ses);
    //torrentUpdateLog(&ses);

    QApplication a(argc, argv);
    if(window_from_name("myTorrentClient") != 0){
        if(argc >= 2){
        }else{
            QMessageBox msgBox;
            msgBox.setText("уже запущено");
            msgBox.exec();
        }

        return 0;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
