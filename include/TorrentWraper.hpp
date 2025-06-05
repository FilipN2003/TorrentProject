#ifndef TORRENTWRAPPER_HPP
#define TORRENTWRAPPER_HPP

#include <string>
#include <memory>

namespace torrent {

class TorrentHandle;
struct TorrentStatus;

class TorrentWrapper {
public:
    explicit TorrentWrapper(const std::string& filepath);
    ~TorrentWrapper();

    std::string name() const;
    size_t totalSize() const;
    int numPieces() const;
    int pieceLength() const;
    int numFiles() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Implementacija klase za upravljanje torrent sesijom
class TorrentSession {
public:
    TorrentSession();
    ~TorrentSession() = default;

    std::shared_ptr<TorrentHandle> addTorrent(const TorrentWrapper& wrapper, const std::string& save_path = ".");
    
    // Ažuriranje statusa svih torrenta
    void updateStatus();
    
    // Dohvatanje statusa za određeni torrent
    TorrentStatus getStatus(const std::shared_ptr<TorrentHandle>& handle) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

struct TorrentStatus {
    float progress;     // Napredak preuzimanja (0.0 do 1.0)
    int downloadRate;   // Brzina preuzimanja u KB/s
    int uploadRate;     // Brzina slanja u KB/s
    bool isSeeding;     // Da li je torrent u fazi seedovanja
};

class TorrentHandle {
public:
    ~TorrentHandle() = default;

    // Upravljanje preuzimanjem
    void pause();
    void resume();
    bool isPaused() const;

    // Status
    TorrentStatus status() const;

    // Uklanjanje
    void remove(bool deleteFiles = false);

    // Detalji
    std::string infoHash() const;
    std::vector<std::string> fileNames() const;
    std::vector<std::string> fileHashes() const;

private:
    friend class TorrentSession;
    class Impl;
    std::unique_ptr<Impl> impl_;
};


}

#endif

