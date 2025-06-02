
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>

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

std::atomic<bool> exit_flag{false}; //promenljiva koja se moze bezbedno citati i menjati iz vise niti bez mutexa
//atomic omogucava da ne dodje do "trke za podacima" ono iz dpja
//atomic promenljiva je bezbedna za koriscenje iz vise niti

//funkcija koja prima pokazivac na handle i u zavisnosti od toga sta korisnik unese pauzira, nastavlja i prekida program
void input_thread(lt::torrent_handle& handle){

    while(!exit_flag){
        std::string cmd;
        std::getline(std::cin, cmd); //zaustavlja nit dok korisnik ne unese nesto

        if(cmd == "p"){
            handle.pause();
            std::cout << "\nTorrent paused.\n";
        }else if(cmd == "r"){
            handle.resume();
            std::cout << "\nTorrent resumed.\n";
        }else if(cmd== "q"){
            std::cout << "\nExiting...\n";
            exit_flag = true; // ovde glavna nit zavrsava
        }
    }
}


int main(int argc, char* argv[]){

        if(argc != 2){
            std::cerr << "Greska: pogresan broj argumenata" << std::endl;
            return 1;  
        }

        std::ifstream file(argv[1], std::ios_base::binary);
        if(!file){
            std::cerr << "Neuspesno otvaranje fajla" << std::endl;
            return 1;
        }

        std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        lt::error_code ec;
        lt::span<const char> buf(buffer.data(), buffer.size());
        lt::bdecode_node node = lt::bdecode(buf, ec);
        if(ec){
            std::cerr << "bdecode failed: " << ec.message() << "\n";
            return 1;
        }

        lt::torrent_info t_info(node, ec);
        if(ec){
            std::cerr << "torrent_info creation failed" << "\n";
            return 1;
        }


        lt::settings_pack settings;
        lt::session ses(settings);

        lt::add_torrent_params atp;
        atp.save_path = ".";
        atp.ti = std::make_shared<lt::torrent_info>(t_info);
        
        lt::torrent_handle handle = ses.add_torrent(atp);
        std::cout << "Torrent added. Type 'p' to pause, 'r' to resume, 'q' to quit.\n";

        //pokrecemo nit i izvrsavamo funkciju input_thread
        std::thread input(input_thread, std::ref(handle)); //std::ref(handle) znaci da prosledjujemo handle kao referencu a ne kao kopiju


        while (!exit_flag) { //kad exit_flag postane true zavrsava se program
            auto status = handle.status();
            std::cout << "\rProgress: " << (status.progress * 100) << "% | Download rate: "
                      << (status.download_rate / 1000) << " kB/s | Peers: "
                      << status.num_peers << std::flush;
    
            if (status.is_seeding) {
                std::cout << "\nDownload complete.\n";
                exit_flag = true;
            }
    
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    
        //Ovo ispod znaci sacekaj da se ova nit zavrsi pre nego sto nastavim dalje u programu
        input.join(); //.join garantuje da se glavni program nece zavrsiti dok se nit ne zavrsi

    return 0;
}