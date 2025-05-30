
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <libtorrent/torrent_info.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/span.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/settings_pack.hpp>

#include <thread>
#include <chrono>

namespace lt = libtorrent;

int main(int argc, char* argv[])
{
    //Kao argument komandne linije unosi se naziv .torrent fajla (ovo cemo naravno kasnije promeniti)
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <torrent-file>\n";
        return 1;
    }

    //Citanje .torrent fajla (prvi argument komandne linije, citamo ga u binarnoj formi a ne kao txt fajl
    // jer je on u bencode formatu, sadrzi bajtove koji nisu tekst npr SHA-1 hesevi)
    std::ifstream file(argv[1], std::ios_base::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return 1;
    }

    //Procitano ucitavamo u vektor charova (iterator na pocetak i kraj fajla)
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    //Objekat za hvatanje gresaka
    lt::error_code ec;
    //Pogled na buffer
    lt::span<const char> span_buffer(buffer.data(), buffer.size());
    //Bdecode parsiranje .torrent fajla
    lt::bdecode_node node = libtorrent::bdecode(span_buffer, ec);
    if(ec){
        std::cerr << "bdecode failed: " << ec.message() << "\n";
        return 1;
    }

    //Promenljiva koja sadrzi informacije o .torrent fajlu(ime,velicinu,broj delova,velicinu jednog dela...)
    lt::torrent_info t_info(node, ec);
    if (ec) {
        std::cerr << "Failed to create torrent_info: " << ec.message() << "\n";
        return 1;
    }

    //Pristupanje informacijama i njihovo ispisivanje
    auto name = t_info.name();
    auto total_size = t_info.total_size();
    auto piece_length = t_info.piece_length();
    auto num_of_files = t_info.num_files();
    auto hash_info = t_info.info_hashes();
    auto num_of_pieces = t_info.num_pieces();

    std::cout << "Name: " << name << std::endl;
    std::cout << "Number of files: " << num_of_files << std::endl;
    std::cout << "Number of pieces: " << num_of_pieces << std::endl;
    std::cout << "Piece length: " << piece_length << std::endl;
    std::cout << "Total size: " << total_size << std::endl;
    std::cout << "Hash info: " << hash_info.v1 << std::endl;

    if(!t_info.trackers().empty()){
        std:: cout << "Tracker: " << t_info.trackers()[0].url << "\n";
    }else{
        std::cout << "No trackers found.\n";
    }

    std::cout << "---------------------------\n";


    lt::settings_pack settings; //objekat za podesavanja, inicijalno postavljen na podrazumevana
    lt::session ses(settings); //kreiramo objekat koji je kljucan za BitTorrent client jer se povezuje sa drugim klijentima, preuzima, seed-uje itd...

    lt::add_torrent_params atp; //objekat koji sadrzi sve informacije potrebne za pokretanje torrenta
    atp.ti = std::make_shared<lt::torrent_info>(t_info); //pristupamo polju ove promenljive i tu smestamo deljeni pokazivac na kopiju t_info, ovako se osiguravamo da ce se taj objekat automatski obrisati iz memorije 
    atp.save_path = "."; //biramo gde zelimo da se prikaze skinut fajl, . je trenutni dir
    lt::torrent_handle handle = ses.add_torrent(atp); //dodajemo torrent sesiju u handle koji koristimo za kontrolu ovog torrenta npr pauziranje, provera napretka itd
    //ova linija iznad pokrece skidanje automatski

    std::cout << "Torrent added! \n" << std::endl;


    //Beskonacna petlja koja prikazuje napredak skidanja torenta

    for(;;){
        lt::torrent_status status = handle.status(); //vraca informacije o trenutnom stanju torenta

        //Ispisujemo stanje, status.progress je broj izmedju 0 i 1, pa puta 100 dobijamo procente, download_rate i upload_rate su u bajtovima po sekundi pa delimo sa hiljadu da dobijem kb
        std::cout << "\rProgress: " << (status.progress * 100) << "%" << " | Download rate:" << (status.download_rate / 1000) << "kB/s" << "| Upload rate: " << (status.upload_rate / 1000) << " | Peers: " << status.num_peers << std::flush;
    
        //ceka jednu sekundu da se terminal ne bi zatrpao
        std::this_thread::sleep_for(std::chrono::seconds(1));

        //Ako je preuzmanje gotovo izlazimo iz petlje i ispisujemo da je kraj.
        if(status.is_seeding){
            std::cout << "\nDownload complete.\n";
            break;
        }
    
    }


    return 0;
}