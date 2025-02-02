#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <system_error>
#include <stdexcept>

#include "fileloader.hpp"

namespace BinFileUtils {

FileLoader::FileLoader()
    : fd(-1)
{
}

FileLoader::FileLoader(const std::string& fileName)
    : fd(-1)
{
    load(fileName);
}

void FileLoader::load(const std::string& fileName)
{
    if (fd != -1) {
        throw std::invalid_argument("file already loaded");
    }

    struct stat sb;

    fd = open(fileName.c_str(), O_RDONLY);
    if (fd == -1)
        throw std::system_error(errno, std::generic_category(), "open");


    if (fstat(fd, &sb) == -1) {          /* To obtain file size */
        close(fd);
        throw std::system_error(errno, std::generic_category(), "fstat");
    }

    size = sb.st_size;

    addr = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (addr == MAP_FAILED) {
        close(fd);
        throw std::system_error(errno, std::generic_category(), "mmap failed");
    }

    madvise(addr, size, MADV_SEQUENTIAL);
}

FileLoader::~FileLoader()
{
    if (fd != -1) {
        munmap(addr, size);
        close(fd);
    }
}

} // Namespace
