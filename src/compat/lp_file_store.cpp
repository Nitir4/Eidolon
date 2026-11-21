#include <tora/compat/lp_file.hpp>
#include <chrono>
#include <fstream>
#include <random>
#include <system_error>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace tora::compat {
namespace {
[[noreturn]] void ioError(const char* operation) { throw FileError(FileErrorCode::Io,operation); }
void atomicWrite(const std::filesystem::path& destination,const std::string& bytes) {
    if (destination.empty()) throw FileError(FileErrorCode::MissingFilename,"Choose a filename before saving");
    auto directory=destination.parent_path();if (directory.empty()) directory=".";
    std::filesystem::path temporary;
    std::random_device random;
    // Exclusive creation prevents a concurrent saver from reusing our file.
#ifdef _WIN32
    HANDLE handle=INVALID_HANDLE_VALUE;
#else
    int handle=-1;
#endif
    for (int attempt=0;attempt<32;++attempt) {
        temporary=directory/(".tora-tmp-"+std::to_string(random())+"-"+
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
#ifdef _WIN32
        handle=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if (handle!=INVALID_HANDLE_VALUE) break;
        if (GetLastError()!=ERROR_FILE_EXISTS && GetLastError()!=ERROR_ALREADY_EXISTS) ioError("Cannot create temporary save file");
#else
        handle=::open(temporary.c_str(),O_CREAT|O_EXCL|O_WRONLY,0666);
        if (handle>=0) break;
        if (errno!=EEXIST) ioError("Cannot create temporary save file");
#endif
    }
#ifdef _WIN32
    if (handle==INVALID_HANDLE_VALUE) ioError("Cannot reserve temporary save file");
#else
    if (handle<0) ioError("Cannot reserve temporary save file");
#endif
    try {
        std::size_t at=0;
        while (at<bytes.size()) {
#ifdef _WIN32
            DWORD written=0;
            if (!WriteFile(handle,bytes.data()+at,static_cast<DWORD>(bytes.size()-at),&written,nullptr) || written==0)
                ioError("Cannot write temporary save file");
#else
            const auto written=::write(handle,bytes.data()+at,bytes.size()-at);
            if (written<0 && errno==EINTR) continue;
            if (written<=0) ioError("Cannot write temporary save file");
#endif
            at+=static_cast<std::size_t>(written);
        }
#ifdef _WIN32
        if (!FlushFileBuffers(handle)) ioError("Cannot flush temporary save file");
        const auto closing=handle;handle=INVALID_HANDLE_VALUE;
        if (!CloseHandle(closing)) ioError("Cannot close temporary save file");
        if (!MoveFileExW(temporary.c_str(),destination.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
            ioError("Cannot replace destination file");
#else
        if (::fsync(handle)!=0) ioError("Cannot flush temporary save file");
        const auto closing=handle;handle=-1;
        if (::close(closing)!=0) ioError("Cannot close temporary save file");
        std::error_code error;std::filesystem::rename(temporary,destination,error);
        if (error) ioError("Cannot replace destination file");
#endif
    } catch (...) {
#ifdef _WIN32
        if (handle!=INVALID_HANDLE_VALUE) CloseHandle(handle);
#else
        if (handle>=0) ::close(handle);
#endif
        std::error_code ignored;std::filesystem::remove(temporary,ignored);
        throw;
    }
}
} // namespace

LpDocument LpFileStore::open(const std::filesystem::path& path,bool readOnly,QuotePolicy quotes) {
    std::ifstream input(path,std::ios::binary);
    if (!input) ioError("Cannot open LP file");
    std::string bytes;char buffer[8192];
    while (input) {
        input.read(buffer,sizeof(buffer));const auto count=input.gcount();
        if (bytes.size()+static_cast<std::size_t>(count)>ToraLpCodec::MaxFileBytes)
            throw FileError(FileErrorCode::TooLarge,"LP file exceeds the supported byte limit");
        bytes.append(buffer,static_cast<std::size_t>(count));
    }
    if (!input.eof()) ioError("Cannot read LP file");
    auto document=ToraLpCodec::decode(bytes,quotes);document.path_=path;document.readOnly_=readOnly;
    return document;
}

void LpFileStore::save(LpDocument& document) {
    if (document.readOnly_) throw FileError(FileErrorCode::ReadOnly,"Document was opened read-only");
    if (document.path_.empty()) throw FileError(FileErrorCode::MissingFilename,"Choose a filename before saving");
    saveAs(document,document.path_);
}
LpDocument LpFileStore::imported(LpDocument document,const std::filesystem::path& path,bool readOnly) {
    (void)ToraLpCodec::encode(document);
    document.path_=path;document.readOnly_=readOnly;
    document.savedProblem_=document.problem;document.savedFormat_=document.inputFormat;
    return document;
}

void LpFileStore::saveAs(LpDocument& document,const std::filesystem::path& path) {
    const auto bytes=ToraLpCodec::encode(document);
    // Prepare all allocations and diagnostics before replacing the target.
    auto saved=ToraLpCodec::decode(bytes,document.quotePolicy_);saved.path_=path;
    atomicWrite(path,bytes);
    document=std::move(saved);
}
} // namespace tora::compat
