#include "abc/memory_mapped_file.hpp"

namespace abc
{
////////////////////////////////////////////////////////////////////////////////

// OS-specific
#ifdef ABC_PLATFORM_WINDOWS_FAMILY
// Windows
#    include <Windows.h>
#    include <memoryapi.h>
#    include <fileapi.h>

struct memory_mapped_file::pimpl
{
    pimpl(void* i_mappedFile, void* i_fileHandle)
        : m_fileMapping(i_mappedFile), m_fileHandle(i_fileHandle)
    {
    }

    void reset(void* i_mappedFile, void* i_fileHandle)
    {
        m_fileMapping = i_mappedFile;
        m_fileHandle  = i_fileHandle;
    }

    void* m_fileMapping;
    void* m_fileHandle;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

memory_mapped_file::memory_mapped_file()
    : m_filename(),
      m_filesize(0),
      m_access(access_type::read),
      m_cacheHint(cache_hint::normal),

      m_mappedBytes(0),
      m_mappedFileView(nullptr),
      m_impl(new pimpl(nullptr, nullptr))
{
}

/// open file, mappedBytes = 0 maps the whole file
memory_mapped_file::memory_mapped_file(const std::string& filename, size_t mappedBytes,
                                       access_type access, cache_hint hint)
    : m_filename(filename),
      m_filesize(0),
      m_access(access),
      m_cacheHint(hint),

      m_mappedBytes(mappedBytes),
      m_mappedFileView(nullptr),
      m_impl(new pimpl(nullptr, nullptr))
{
    auto openResult = open(filename, mappedBytes, access, hint);
    ABC_ASSERT(openResult == abc::success, "{}", openResult.get_error().message_with_inner());
}

/// close file (see close() )
memory_mapped_file::~memory_mapped_file()
{
    close();
    delete m_impl;
}

/// open file
memory_mapped_file::open_result memory_mapped_file::open(const std::string& filename,
                                                         size_t mappedBytes, access_type access,
                                                         cache_hint hint)
{
    if (is_open())
    {
        return abc::success;
    }

    m_filename       = filename;
    m_filesize       = 0;
    m_access         = access;
    m_cacheHint      = hint;
    m_mappedBytes    = 0;
    m_mappedFileView = nullptr;
    m_impl->reset(nullptr, nullptr);

    const DWORD windowsHint = [&]() -> DWORD {
        switch (m_cacheHint)
        {
            case cache_hint::normal:
                return FILE_ATTRIBUTE_NORMAL;
            case cache_hint::random:
                return FILE_FLAG_RANDOM_ACCESS;
            case cache_hint::sequential:
                return FILE_FLAG_SEQUENTIAL_SCAN;
            //default:
        }
        ABC_FAIL("not supported");
        return 0;
    }();

    const DWORD windowsAccess = [&]() -> DWORD {
        switch (m_access)
        {
            case access_type::read:
                return GENERIC_READ;
            case access_type::write:
                return GENERIC_WRITE;
             case access_type::readwrite:
                 return GENERIC_READ | GENERIC_WRITE;
            // default:
        }
        ABC_FAIL("not supported");
        return 0;
    }();
    const DWORD openMode = m_access == access_type::read ? OPEN_EXISTING : OPEN_ALWAYS;

    m_impl->m_fileHandle = CreateFileA(m_filename.c_str(),
                                       windowsAccess,    // access
                                       FILE_SHARE_READ,  // sharingmode
                                       0,                // security attributes
                                       openMode,         // creation mode
                                       windowsHint,      // flags
                                       nullptr);         // template file
    if (m_impl->m_fileHandle == INVALID_HANDLE_VALUE)
    {
        return open_error(OpenErrorCode::FileNotFound,
                          abc::format("{} file couldn't be opened", m_filename));
    }

    LARGE_INTEGER result;
    if (!GetFileSizeEx(m_impl->m_fileHandle, &result))
    {
        return open_error(OpenErrorCode::FileNotFound,
                          abc::format("{} Failed retrieving size", m_filename));
    }
    m_filesize = static_cast<size_t>(result.QuadPart);

    if (m_filesize == 0 && mappedBytes == 0)
    {
        return open_error(
            OpenErrorCode::InvalidParameters,
            abc::format("{} Cannot create an empty mapping. File is empty.", m_filename));
    }

    const DWORD windowsPageAccess = [&]() -> DWORD {
        switch (m_access)
        {
            case access_type::read:      return PAGE_READONLY;
            case access_type::write:     return PAGE_READWRITE;
            case access_type::readwrite: return PAGE_READWRITE;
            //default:
        }
        ABC_FAIL("not supported");
        return 0;
    }();
    constexpr size_t HMASK  = ~((size_t(1) << ((8*sizeof(mappedBytes))>>1))-1);
    const DWORD offsetLow  = DWORD(mappedBytes & HMASK);
    const DWORD offsetHigh = DWORD(mappedBytes >> (8*sizeof(mappedBytes)/2));

    m_impl->m_fileMapping = CreateFileMapping(
        m_impl->m_fileHandle,  // file (if INVALID_HANDLE_VALUE -> system paging files)
        0,                     // mapping attributes
        windowsPageAccess,     // protect attributes
        offsetHigh,            // max size high
        offsetLow,             // max size low
        nullptr);              // mapping name
    if (m_impl->m_fileMapping == nullptr)
    {
        auto err = GetLastError();
        if (err == 1006)
        {
            return open_error(
                OpenErrorCode::InvalidParameters,
                abc::format(
                    "{} Failed creating file mapping, size cannot be zero when creating a file",
                    m_filename));
        }
        return open_error(OpenErrorCode::FileNotFound,
                          abc::format("Failed creating file mapping for: '{}'", m_filename));
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return open_error(
            OpenErrorCode::MappingAlreadyExists,
            abc::format(
                "Failed creating file mapping for: '{}', since there is already a mapping onto it",
                m_filename));
    }
    if (GetLastError() == ERROR_DISK_FULL)
    {
        return open_error(OpenErrorCode::InvalidParameters,
                          abc::format("{} Disk is full", m_filename));
    }

    auto remapResult = remap(0, mappedBytes);
    if (remapResult != abc::success)
    {
        return open_error(
            OpenErrorCode::InvalidParameters,
            abc::format("{} Failed remapping: {}", m_filename, remapResult.get_error().message()));
    }

    return abc::success;
}

void memory_mapped_file::close()
{
    if (m_mappedFileView)
    {
        UnmapViewOfFile(m_mappedFileView);
        m_mappedFileView = nullptr;
    }

    if (m_impl->m_fileMapping)
    {
        CloseHandle(m_impl->m_fileMapping);
        m_impl->m_fileMapping = nullptr;
    }

    if (m_impl->m_fileHandle)
    {
        CloseHandle(m_impl->m_fileHandle);
        m_impl->m_fileHandle = nullptr;
    }

    m_filesize = 0;
}

uint8_t memory_mapped_file::operator[](size_t offset) const
{
    return (static_cast<uint8_t*>(m_mappedFileView))[offset];
}

uint8_t memory_mapped_file::at(size_t offset) const
{
    // checks
    if (!m_mappedFileView)
    {
        ABC_FAIL("No view mapped");
    }
    if (offset >= m_filesize)
    {
        ABC_FAIL("View is not large enough");
    }
    return operator[](offset);
}

const uint8_t* memory_mapped_file::getData(size_t offset) const
{
    return static_cast<const uint8_t*>(m_mappedFileView) + offset;
}

uint8_t* memory_mapped_file::getData(size_t offset)
{
    return static_cast<uint8_t*>(m_mappedFileView) + offset;
}

bool memory_mapped_file::is_open() const { return m_mappedFileView != nullptr; }

size_t memory_mapped_file::size() const { return m_filesize; }

size_t memory_mapped_file::mapped_size() const { return m_mappedBytes; }

/// replace mapping by a new one of the same file, offset MUST be a multiple of the page size
memory_mapped_file::remap_result memory_mapped_file::remap(size_t offset, size_t mappedBytes)
{
    if (!m_impl->m_fileHandle)
    {
        return remap_error(RemapErrorCode::InvalidParameters, "Invalid file handle");
    }
    if (mappedBytes == static_cast<size_t>(map_range::whole))
    {
        mappedBytes = m_filesize;
    }
    if (m_mappedFileView != nullptr)
    {
        UnmapViewOfFile(m_mappedFileView);
        m_mappedFileView = nullptr;
    }

    if (offset > m_filesize)
    {
        return remap_error(
            RemapErrorCode::InvalidParameters,
            abc::format("Invalid parameters: offset({}) is bigger than file size({})", offset,
                        m_filesize));
    }
    if (offset + mappedBytes > m_filesize)
    {
        mappedBytes = size_t(m_filesize - offset);
    }

    DWORD offsetLow  = DWORD(offset & 0xFFFFFFFF);
    DWORD offsetHigh = DWORD(offset >> 32);
    m_mappedBytes    = mappedBytes;

    const DWORD windowsPageAccess = [&]() -> DWORD {
        switch (m_access)
        {
            case access_type::read:      return FILE_MAP_READ;
            case access_type::write:     return FILE_MAP_WRITE;
            case access_type::readwrite: return FILE_MAP_ALL_ACCESS;
            //default:
        }

        ABC_FAIL("not supported");
        return 0;
    }();
    m_mappedFileView = MapViewOfFile(m_impl->m_fileMapping,  // filemapping
                                     windowsPageAccess,      // access
                                     offsetHigh, offsetLow,  // offset
                                     mappedBytes);           // mapped size
    if (m_mappedFileView == nullptr)
    {
        m_mappedBytes    = 0;
        m_mappedFileView = nullptr;
        return remap_error(RemapErrorCode::InvalidParameters,
                           abc::format("Couldn't create the map view of the file"));
    }

    return abc::success;
}

size_t memory_mapped_file::get_page_size() const
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwAllocationGranularity;
}

// Windows
#elif defined(ABC_PLATFORM_LINUX_FAMILY)
// Linux
// enable large file support on 32 bit systems
#    ifndef _LARGEFILE64_SOURCE
#        define _LARGEFILE64_SOURCE
#    endif
#    ifdef _FILE_OFFSET_BITS
#        undef _FILE_OFFSET_BITS
#    endif
#    define _FILE_OFFSET_BITS 64
// and include needed headers
#    include <sys/stat.h>
#    include <sys/mman.h>
#    include <fcntl.h>
#    include <errno.h>
#    include <unistd.h>

struct memory_mapped_file::pimpl
{
    void* m_fileMapping;
    void* m_fileHandle;
};

/// do nothing, must use open()
memory_mapped_file::memory_mapped_file()
    : _filename(),
      _filesize(0),
      _hint(Normal),
      _mappedBytes(0),
      _file(0),
#    ifdef _MSC_VER
      _mappedFile(NULL),
#    endif
      _mappedView(NULL)
{
}

/// open file, mappedBytes = 0 maps the whole file
memory_mapped_file::memory_mapped_file(const std::string& filename, size_t mappedBytes,
                                       cache_hint hint)
    : _filename(filename),
      _filesize(0),
      _hint(hint),
      _mappedBytes(mappedBytes),
      _file(0),
#    ifdef _MSC_VER
      _mappedFile(NULL),
#    endif
      _mappedView(NULL)
{
    open(filename, mappedBytes, hint);
}

/// close file (see close() )
memory_mapped_file::~memory_mapped_file() { close(); }

/// open file
bool memory_mapped_file::open(const std::string& filename, size_t mappedBytes, cache_hint hint)
{
    // already open ?
    if (isValid())
        return false;

    _file       = 0;
    _filesize   = 0;
    _hint       = hint;
#    ifdef _MSC_VER
    _mappedFile = NULL;
#    endif
    _mappedView = NULL;

#    ifdef _MSC_VER
    // Windows

    DWORD winHint = 0;
    switch (_hint)
    {
        case Normal:
            winHint = FILE_ATTRIBUTE_NORMAL;
            break;
        case SequentialScan:
            winHint = FILE_FLAG_SEQUENTIAL_SCAN;
            break;
        case RandomAccess:
            winHint = FILE_FLAG_RANDOM_ACCESS;
            break;
        default:
            break;
    }

    // open file
    _file = ::CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                          winHint, NULL);
    if (!_file)
        return false;

    // file size
    LARGE_INTEGER result;
    if (!GetFileSizeEx(_file, &result))
        return false;
    _filesize = static_cast<uint64_t>(result.QuadPart);

    // convert to mapped mode
    _mappedFile = ::CreateFileMapping(_file, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!_mappedFile)
        return false;

#    else

    // Linux

    // open file
    _file = ::open(filename.c_str(), O_RDONLY | O_LARGEFILE);
    if (_file == -1)
    {
        _file = 0;
        return false;
    }

    // file size
    struct stat64 statInfo;
    if (fstat64(_file, &statInfo) < 0)
        return false;

    _filesize = statInfo.st_size;
#    endif

    // initial mapping
    remap(0, mappedBytes);

    if (!_mappedView)
        return false;

    // everything's fine
    return true;
}

/// close file
void memory_mapped_file::close()
{
    // kill pointer
    if (_mappedView)
    {
#    ifdef _MSC_VER
        ::UnmapViewOfFile(_mappedView);
#    else
        ::munmap(_mappedView, _filesize);
#    endif
        _mappedView = NULL;
    }

#    ifdef _MSC_VER
    if (_mappedFile)
    {
        ::CloseHandle(_mappedFile);
        _mappedFile = NULL;
    }
#    endif

    // close underlying file
    if (_file)
    {
#    ifdef _MSC_VER
        ::CloseHandle(_file);
#    else
        ::close(_file);
#    endif
        _file = 0;
    }

    _filesize = 0;
}

/// access position, no range checking (faster)
unsigned char memory_mapped_file::operator[](size_t offset) const
{
    return ((unsigned char*)_mappedView)[offset];
}

/// access position, including range checking
unsigned char memory_mapped_file::at(size_t offset) const
{
    // checks
    if (!_mappedView)
        throw std::invalid_argument("No view mapped");
    if (offset >= _filesize)
        throw std::out_of_range("View is not large enough");

    return operator[](offset);
}

/// raw access
const unsigned char* memory_mapped_file::getData() const
{
    return (const unsigned char*)_mappedView;
}

/// true, if file successfully opened
bool memory_mapped_file::isValid() const { return _mappedView != NULL; }

/// get file size
size_t memory_mapped_file::size() const { return _filesize; }

/// get number of actually mapped bytes
size_t memory_mapped_file::mappedSize() const { return _mappedBytes; }

/// replace mapping by a new one of the same file, offset MUST be a multiple of the page size
bool memory_mapped_file::remap(size_t offset, size_t mappedBytes)
{
    if (!_file)
        return false;

    if (mappedBytes == WholeFile)
        mappedBytes = _filesize;

    // close old mapping
    if (_mappedView)
    {
#    ifdef _MSC_VER
        ::UnmapViewOfFile(_mappedView);
#    else
        ::munmap(_mappedView, _mappedBytes);
#    endif
        _mappedView = NULL;
    }

    // don't go further than end of file
    if (offset > _filesize)
        return false;
    if (offset + mappedBytes > _filesize)
        mappedBytes = size_t(_filesize - offset);

#    ifdef _MSC_VER
    // Windows

    DWORD offsetLow  = DWORD(offset & 0xFFFFFFFF);
    DWORD offsetHigh = DWORD(offset >> 32);
    _mappedBytes     = mappedBytes;

    // get memory address
    _mappedView = ::MapViewOfFile(_mappedFile, FILE_MAP_READ, offsetHigh, offsetLow, mappedBytes);

    if (_mappedView == NULL)
    {
        _mappedBytes = 0;
        _mappedView  = NULL;
        return false;
    }

    return true;

#    else

    // Linux
    // new mapping
    _mappedView = ::mmap64(NULL, mappedBytes, PROT_READ, MAP_SHARED, _file, offset);
    if (_mappedView == MAP_FAILED)
    {
        _mappedBytes = 0;
        _mappedView  = NULL;
        return false;
    }

    _mappedBytes = mappedBytes;

    // tweak performance
    int linuxHint = 0;
    switch (_hint)
    {
        case Normal:
            linuxHint = MADV_NORMAL;
            break;
        case SequentialScan:
            linuxHint = MADV_SEQUENTIAL;
            break;
        case RandomAccess:
            linuxHint = MADV_RANDOM;
            break;
        default:
            break;
    }
    // assume that file will be accessed soon
    // linuxHint |= MADV_WILLNEED;
    // assume that file will be large
    // linuxHint |= MADV_HUGEPAGE;

    ::madvise(_mappedView, _mappedBytes, linuxHint);

    return true;
#    endif
}

/// get OS page size (for remap)
size_t memory_mapped_file::get_page_size()
{
#    ifdef _MSC_VER
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwAllocationGranularity;
#    else
    return sysconf(_SC_PAGESIZE);  //::getpagesize();
#    endif
}

// Linux
#endif

////////////////////////////////////////////////////////////////////////////////
}  // namespace abc