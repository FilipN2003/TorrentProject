#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <filesystem>
#include <thread>
#include <chrono>

#define TORRENT_USE_LIBTORRENT_NAMESPACE
#include <libtorrent/alert_types.hpp>

#include <libtorrent/torrent_info.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/span.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/alert.hpp>


namespace lt = libtorrent;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Upotreba: " << argv[0] << " <putanja_do_torrent_fajla>\n";
        return 1;
    }

    std::string torrent_path = argv[1];
    if (!fs::exists(torrent_path)) {
        std::cerr << "Fajl ne postoji: " << torrent_path << "\n";
        return 1;
    }

    lt::settings_pack settings; //konfiguracija za lt session objekat
    settings.set_str(lt::settings_pack::user_agent, "MyLibtorrentClient/1.0"); //set_str postavlja user_agent string koji je zapravo tekst koji nas torrent client salje kada komunicira sa trakerima. Iskreno mi ovo nije bas najjasnije
    settings.set_int(lt::settings_pack::alert_mask, //alert mask je zapravo bitmask i zato koristimo | da dodamo sve sto zelimo
        lt::alert::status_notification |
        lt::alert::error_notification |
        lt::alert::storage_notification |
        lt::alert::tracker_notification);

    lt::session ses(settings); //sesiji prosledjujemo settings koji sadrzi podesavanja ponasanja sesije npr koji alertovi se prijavljuju, maksimalan broj peerova itd.

    lt::add_torrent_params atp;
    atp.ti = std::make_shared<lt::torrent_info>(torrent_path); //ovde mozemo da koristimo i putanju, ali mozemo i onaj torrent_info objekat od ranije
    atp.save_path = "."; // current directory
    lt::torrent_handle handle = ses.add_torrent(atp); //ovde se pokrece skidanje torenta

    bool done = false;

    while (!done) {
        ses.post_torrent_updates(); // sesija azurira status torenta

        std::vector<lt::alert*> alerts;
        ses.pop_alerts(&alerts); // uzima sve trenutno dostupne alertove i stavlja ih u vektor alerts
        //alertovi su notifikacije od libtorenta, javljaju da se nesto desilo npr fajl je skinut ili je doslo do greske itd

        for (lt::alert* a : alerts) { // obilazimo sve alertove
            std::cout << a->message() << "\n";

            if (auto* ta = lt::alert_cast<lt::torrent_finished_alert>(a)) { //ukoliko ovo kastovanje uspe znaci da se taj dogadjaj desio, isto za naredna dva if uslova
                std::cout << "Download complete: " << ta->handle.status().name << "\n"; //ako se nije desio ta ce biti null i necemo uci u ovu if granu
                done = true;
            } else if (auto* err = lt::alert_cast<lt::torrent_error_alert>(a)) {
                std::cerr << "Error: " << err->error.message() << "\n";
                done = true;
            } else if (auto* sa = lt::alert_cast<lt::state_update_alert>(a)) {
                for (const auto& s : sa->status) {
                    std::cout << s.name << ": " << (s.progress_ppm / 10000) << "% downloaded\n";
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); //cekamo jednu sekundi da se ne bi zatrpavao terminal
    }

    return 0;
}
