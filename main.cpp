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


void loadTorrent(std::string torrentFileNameIn, session *sessionIn){
    add_torrent_params p;
    p.save_path = "./";

    error_code ec;
    p.ti = boost::make_shared<torrent_info>(torrentFileNameIn, boost::ref(ec), 0);
    if (ec)
    {
        fprintf(stderr, "%s\n", ec.message().c_str());
        return;
    }
    sessionIn->add_torrent(p, ec);
    if (ec)
    {
        fprintf(stderr, "%s\n", ec.message().c_str());
        return;
    }
}

char const* state(lt::torrent_status::state_t s)
{
    switch(s) {
    case lt::torrent_status::checking_files: return "проверка";
    case lt::torrent_status::downloading_metadata: return "dl metadata";
    case lt::torrent_status::downloading: return "загрузка";
    case lt::torrent_status::finished: return "завершен";
    case lt::torrent_status::seeding: return "раздача";
    case lt::torrent_status::allocating: return "allocating";
    case lt::torrent_status::checking_resume_data: return "checking resume";
    default: return "<>";
    }
}

void torrentUpdateLog(session *sessionIn){
    clk::time_point last_save_resume = clk::now();

    lt::torrent_handle h;
    for (;;) {
        std::vector<lt::alert*> alerts;
        sessionIn->pop_alerts(&alerts);

        for (lt::alert const* a : alerts) {
            if (auto at = lt::alert_cast<lt::add_torrent_alert>(a)) {
                h = at->handle;
            }
            // if we receive the finished alert or an error, we're done
            if (lt::alert_cast<lt::torrent_finished_alert>(a)) {
                h.save_resume_data();
            }
            if (lt::alert_cast<lt::torrent_error_alert>(a)) {
                std::cout << a->message() << std::endl;
            }

            // when resume data is ready, save it
            if (auto rd = lt::alert_cast<lt::save_resume_data_alert>(a)) {
                std::ofstream of(".resume_file", std::ios_base::binary);
                of.unsetf(std::ios_base::skipws);
                lt::bencode(std::ostream_iterator<char>(of), *rd->resume_data);
            }

            if (auto st = lt::alert_cast<lt::state_update_alert>(a)) {
                if (st->status.empty()) continue;

                // we only have a single torrent, so we know which one
                // the status is for
                lt::torrent_status const& s = st->status[0];
                std::cout << "\r" << state(s.state) << " "
                          << (s.download_payload_rate / 1000) << " kB/s "
                          << (s.total_done / 1000) << " kB ("
                          << (s.progress_ppm / 10000) << "%) загружено\x1b[K";
                std::cout.flush();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // ask the session to post a state_update_alert, to update our
        // state output for the torrent
        sessionIn->post_torrent_updates();

        // save resume data once every 30 seconds
        if (clk::now() - last_save_resume > std::chrono::seconds(30)) {
            h.save_resume_data();
            last_save_resume = clk::now();
        }
    }
}




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
