#include "TorrentWrapper.hpp"
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/bdecode.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>

#include <fstream>
#include <vector>
#include <stdexcept>
#include <iomanip>

namespace torrent {

// === TorrentWrapper ===
class TorrentWrapper::Impl {
public:
    std::shared_ptr<libtorrent::torrent_info> t_info;
};

TorrentWrapper::TorrentWrapper(const std::string& filepath)
    : impl_(std::make_unique<Impl>()) {

    std::ifstream file(filepath, std::ios_base::binary);
    if (!file) {
        throw std::runtime_error("Ne mogu da otvorim .torrent fajl: " + filepath);
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    libtorrent::error_code ec;
    libtorrent::bdecode_node node;
    libtorrent::span<const char> data_span(buffer.data(), buffer.size());
    node = libtorrent::bdecode(data_span, ec);

    if (ec) {
        throw std::runtime_error("Greska pri bdecode: " + ec.message());
    }

    impl_->t_info = std::make_shared<libtorrent::torrent_info>(node, ec);
    if (ec) {
        throw std::runtime_error("Greska pri kreiranju torrent_info: " + ec.message());
    }
}

TorrentWrapper::~TorrentWrapper() = default;

std::shared_ptr<void> TorrentWrapper::info() const {
    return impl_->t_info;
}


std::string TorrentWrapper::name() const { return impl_->t_info->name(); }
size_t TorrentWrapper::totalSize() const { return impl_->t_info->total_size(); }
int TorrentWrapper::numPieces() const { return impl_->t_info->num_pieces(); }
int TorrentWrapper::pieceLength() const { return impl_->t_info->piece_length(); }
int TorrentWrapper::numFiles() const { return impl_->t_info->num_files(); }

// === TorrentHandle ===
class TorrentHandle::Impl {
public:
    libtorrent::torrent_handle handle;
    Impl(libtorrent::torrent_handle h) : handle(std::move(h)) {}  //konstruktor kopije
    Impl() = default;   //podrazumevani konstruktor
};

TorrentHandle::~TorrentHandle() = default;

void TorrentHandle::pause() {
    if (impl_) impl_->handle.pause();
}

void TorrentHandle::resume() {
    if (impl_) impl_->handle.resume();
}

bool TorrentHandle::isPaused() const {
    if (impl_) return impl_->handle.status().flags & libtorrent::torrent_flags::paused
;
    return false;
}

TorrentStatus TorrentHandle::status() const {
    auto st = impl_->handle.status();
    return TorrentStatus{
        st.progress,
        st.download_rate,
        st.upload_rate,
        st.is_seeding
    };
}

void TorrentHandle::remove(bool deleteFiles) {
    if (!impl_) return;
    // Uklanjanje preko alert sistema bi bilo idealno, ali ovde koristimo direktnu metodu
    // Korisnik bi trebalo da osigura da session zivi dovoljno dugo
    impl_->handle.auto_managed(false); // sprečava automatsko ponovno dodavanje
    impl_->handle.pause();
    impl_->handle.clear_error();
    // Napomena: remove se obično poziva preko session-a, ovde možemo samo "pripremiti" torrent
    // Ili eventualno javno eksponovati remove iz TorrentSession (preporučljivo)
}

std::string TorrentHandle::infoHash() const {
    if (!impl_) return {};
    auto hash = impl_->handle.info_hashes().v1;
    std::ostringstream oss;
    oss << std::hex;
    for (auto b : hash) oss << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return oss.str();
}

std::vector<std::string> TorrentHandle::fileNames() const {
    std::vector<std::string> files;
    if (!impl_) return files;
    auto info = impl_->handle.torrent_file();
    auto const& storage = info->files();
    for (int i = 0; i < storage.num_files(); ++i) {
        files.emplace_back(storage.file_path(i));
    }
    return files;
}

std::vector<std::string> TorrentHandle::fileHashes() const {
    std::vector<std::string> hashes;
    if (!impl_) return hashes;
    auto info = impl_->handle.torrent_file();
    for (int i = 0; i < info->num_pieces(); ++i) {
        auto hash = info->hash_for_piece(i);
        std::ostringstream oss;
        oss << std::hex;
        for (auto b : hash) oss << std::setw(2) << std::setfill('0') << static_cast<int>(b);
        hashes.push_back(oss.str());
    }
    return hashes;
}


// === TorrentSession ===
class TorrentSession::Impl {
public:
    libtorrent::session session;

    // Funkcija za čitanje alerta iz sesije
    std::vector<libtorrent::alert*> pop_alerts() {
        std::vector<libtorrent::alert*> alerts;
        session.pop_alerts(&alerts);
        return alerts;
    }

    // Funkcija za ažuriranje statusa
    void post_torrent_updates() {
        session.post_torrent_updates();
    }
};

TorrentSession::~TorrentSession() = default;

TorrentSession::TorrentSession()
    : impl_(std::make_unique<Impl>()) {
    libtorrent::settings_pack pack;
    impl_->session = libtorrent::session(pack);
}


std::shared_ptr<TorrentHandle> TorrentSession::addTorrent(const TorrentWrapper& wrapper, const std::string& save_path) {
    libtorrent::add_torrent_params atp;
    // zbog void u implenetaciji fje info(), koristimo ceo potpis
    atp.ti = std::static_pointer_cast<libtorrent::torrent_info>(wrapper.info());
    atp.save_path = save_path;

    auto handle = impl_->session.add_torrent(std::move(atp));
    auto th = std::make_shared<TorrentHandle>();
    th->impl_ = std::make_unique<TorrentHandle::Impl>();
    th->impl_->handle = handle;
    return th;
}

TorrentStatus TorrentSession::getStatus(const std::shared_ptr<TorrentHandle>& handle) const {
    auto st = handle->impl_->handle.status();
    return TorrentStatus{
        st.progress,
        st.download_rate,
        st.upload_rate,
        st.is_seeding
    };
}

}
