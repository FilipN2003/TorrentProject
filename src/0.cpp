

#include <iostream>
#include <libtorrent/torrent_info.hpp>

namespace lt = libtorrent;

int main(int argc, char* argv[]){

    // std::ifstream file(argv[1], std::ios_base::binary);
    // if(!file){
    //     std::cerr << "Neuspesno otvaranje .torrent fajla: " << argv[1] << "\n";
    //     return 1;
    // }

    if(argc != 2){
        std::cerr << "Neispravan broj argumenata" << std::endl;
        return 1;
    }


    try{
        lt::torrent_info ti(argv[1]); //u ovom objektu se nalaze informacije o fajlovima

        auto const& fs = ti.files(); //ovo je fajl storage tu se nalaze svi fajlovi iz .torrent fajla
        std::cout << "Broj fajlova u torentu: " << fs.num_files() << "\n";

        for(int i=0;i<fs.num_files();++i){ //obilazimo sve fajlove i ispisujemo imena i velicine
            std::cout << "Fajl " << i << ": " << fs.file_path(i) << " (" << fs.file_size(i) << "bytes)" << std::endl; 
        }


    }catch(const std::exception& e){
        std::cerr << "Greska prilikom ucitavanja torrent fajla: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}