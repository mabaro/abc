#include "abc/memory_mapped_file.hpp"
#include "abc/string.hpp"
#include "abc/result.hpp"

#include <Windows.h>
#include <memoryapi.h>
#include <fileapi.h>

namespace abc
{
////////////////////////////////////////////////////////////////////////////////

struct memory_mapped_file::pimpl
{
    pimpl(void* i_mappedFile, void* i_fileHandle)
        : m_fileMapping(i_mappedFile), m_fileHandle(i_fileHandle)
    {
    }
    ~pimpl()
    {
        ABC_ASSERT(!is_memory_mapped());
        if (is_memory_mapped())
        {
            unmap();
        }
        if (is_file_mapped())
        {
            close_file();
        }
    }

    bool is_open() const { return m_fileHandle != nullptr; }
    bool is_file_mapped() const { return m_fileMapping != nullptr; }
    bool is_memory_mapped() const { return m_mappedMemory != nullptr; }

    enum class error_code
    {
        disk_full,
        file_not_found,
        invalid_parameters,
        mapping_already_exists,
        unknown
    };
    using error_t = abc::error<error_code>;

    abc::result<void, error_t> create_file(const abc::string& i_fileName, access_type i_accessType,
                                           cache_hint i_cacheHint)
    {
        const DWORD windowsAccess = [i_accessType]() -> DWORD {
            switch (i_accessType)
            {
                case access_type::read:
                    return GENERIC_READ;
                case access_type::write:
                    return GENERIC_WRITE;
                case access_type::readwrite:
                    return GENERIC_READ | GENERIC_WRITE;
                default:
                    ABC_FAIL("not supported");
                    return 0;
            }
        }();
        const DWORD windowsHint = [i_cacheHint]() -> DWORD {
            switch (i_cacheHint)
            {
                case cache_hint::normal:
                    return FILE_ATTRIBUTE_NORMAL;
                case cache_hint::random:
                    return FILE_FLAG_RANDOM_ACCESS;
                case cache_hint::sequential:
                    return FILE_FLAG_SEQUENTIAL_SCAN;
                default:
                    ABC_FAIL("not supported");
                    return 0;
            }
        }();

        const DWORD openMode = i_accessType == access_type::read ? OPEN_EXISTING : OPEN_ALWAYS;

        m_fileHandle = CreateFileA(i_fileName.c_str(),
                                   windowsAccess,    // access
                                   FILE_SHARE_READ,  // sharingmode
                                   0,                // security attributes
                                   openMode,         // creation mode
                                   windowsHint,      // flags
                                   nullptr);         // template file

        if (m_fileHandle == INVALID_HANDLE_VALUE)
        {
            return error_t(error_code::file_not_found,
                           abc::format("{} file couldn't be opened", i_fileName));
        }

        return abc::success;
    }

    abc::result<void, error_t> create_mapping(access_type i_accessType, size_t i_mappedBytes)
    {
        const DWORD windowsPageAccess = [i_accessType]() -> DWORD {
            switch (i_accessType)
            {
                case access_type::read:
                    return PAGE_READONLY;
                case access_type::write:
                    return PAGE_READWRITE;
                case access_type::readwrite:
                    return PAGE_READWRITE;
                default:
                    ABC_FAIL("not supported");
            }
            return 0;
        }();
        const DWORD offsetLow  = DWORD(i_mappedBytes & 0xFFFFFFFF);
        const DWORD offsetHigh = DWORD(i_mappedBytes >> 32);

        m_fileMapping = CreateFileMapping(
            m_fileHandle,       // file (if INVALID_HANDLE_VALUE -> system paging files)
            0,                  // mapping attributes
            windowsPageAccess,  // protect attributes
            offsetHigh,         // max size high
            offsetLow,          // max size low
            nullptr);           // mapping name

        if (m_fileMapping == nullptr)
        {
            auto err = GetLastError();
            if (err == 1006)
            {
                return error_t(
                    error_code::invalid_parameters,
                    "Failed creating file mapping, size cannot be zero when creating a file");
            }
            if (err == ERROR_ALREADY_EXISTS)
            {
                return error_t(
                    error_code::mapping_already_exists,
                    "Failed creating file mapping. There is already a mapping on the same file.");
            }
            if (err == ERROR_DISK_FULL)
            {
                return error_t(error_code::disk_full,
                               abc::format("Failed creating mapping. Disk is full"));
            }
            return error_t(error_code::unknown, "Failed creating file mapping");
        }

        return abc::success;
    }

    abc::result<size_t, error_t> get_file_size() const
    {
        LARGE_INTEGER result;
        if (!GetFileSizeEx(m_fileHandle, &result))
        {
            return error_t(error_code::file_not_found, abc::format("Failed retrieving size."));
        }
        return static_cast<size_t>(result.QuadPart);
    }

    void close_file()
    {
        ABC_ASSERT(m_fileMapping != nullptr);
        if (m_fileMapping != nullptr)
        {
            CloseHandle(m_fileMapping);
            m_fileMapping = nullptr;
        }
    }

    void unmap()
    {
        ABC_ASSERT(m_mappedMemory != nullptr);
        UnmapViewOfFile(m_mappedMemory);
        m_mappedMemory = nullptr;
    }

    abc::result<void, error_t> map(uint64_t offset, size_t mappedBytes, access_type i_accesType)
    {
        if (m_mappedMemory != nullptr)
        {
            unmap();
        }

        DWORD offsetLow  = DWORD(offset & 0xFFFFFFFF);
        DWORD offsetHigh = DWORD(offset >> 32);

        const DWORD windowsPageAccess = [i_accesType]() -> DWORD {
            switch (i_accesType)
            {
                case access_type::read:
                    return FILE_MAP_READ;
                case access_type::write:
                    return FILE_MAP_WRITE;
                case access_type::readwrite:
                    return FILE_MAP_ALL_ACCESS;
                default:
                    ABC_FAIL("not supported");
            }

            return 0;
        }();
        m_mappedMemory = MapViewOfFile(m_fileMapping,          // filemapping
                                       windowsPageAccess,      // access
                                       offsetHigh, offsetLow,  // offset
                                       mappedBytes);           // mapped size
        if (m_mappedMemory == nullptr)
        {
            return error_t(error_code::invalid_parameters,
                           "Couldn't create the map view of the file");
        }

        return abc::success;
    }

    void* get_mapped_memory() const { return m_mappedMemory; }

private:
    void* m_fileMapping  = nullptr;
    void* m_fileHandle   = nullptr;
    void* m_mappedMemory = nullptr;
};

/// open file
memory_mapped_file::open_result memory_mapped_file::open(const std::string& filename,
                                                         size_t mappedBytes, access_type access,
                                                         cache_hint hint)
{
    auto if (m_impl->m_fileHandle == INVALID_HANDLE_VALUE)
    {
        return open_error(OpenErrorCode::FileNotFound,
                          abc::format("{} file couldn't be opened", m_filename));
    }

    auto getSizeResult = m_impl->get_file_size();
    if (getSizeResult != abc::success)
    {
        return open_error(OpenErrorCode::Unknown, getSizeResult.get_error().message_with_inner());
    }
    m_filesize = getSizeResult.get_payload();

    if (m_filesize == 0 && mappedBytes == 0)
    {
        return open_error(
            OpenErrorCode::InvalidParameters,
            abc::format("{} Cannot create an empty mapping. File is empty.", m_filename));
    }

    auto mappingResult = m_impl->create_mapping(access, mappedBytes);
    if (mappingResult != abc::success)
    {
        return open_error(OpenErrorCode::Unknown, mappingResult.get_error().)
    }

    auto remapResult = m_impl->map(0, mappedBytes);
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
        m_impl->unmap(std::move(m_mappedFileView));
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

const uint8_t* memory_mapped_file::getData(size_t offset = 0) const
{
    return static_cast<const uint8_t*>(m_mappedFileView) + offset;
}

uint8_t* memory_mapped_file::getData(size_t offset = 0)
{
    return static_cast<uint8_t*>(m_mappedFileView) + offset;
}

bool memory_mapped_file::is_open() const { return m_mappedFileView != nullptr; }

size_t memory_mapped_file::size() const { return m_filesize; }

size_t memory_mapped_file::mapped_size() const { return m_mappedBytes; }

/// replace mapping by a new one of the same file, offset MUST be a multiple of the page size
memory_mapped_file::remap_result memory_mapped_file::remap(uint64_t offset, size_t mappedBytes)
{
    if (!m_impl->is_open())
    {
        return remap_error(RemapErrorCode::InvalidParameters, "Invalid file handle");
    }
    if (mappedBytes == static_cast<size_t>(map_range::whole))
    {
        mappedBytes = m_filesize;
    }
    if (m_impl->is_mapped())
    {
        m_impl->unmap();
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

    auto mapResult = m_impl->map(offset, mappedBytes);

    return abc::success;
}

size_t memory_mapped_file::get_page_size() const
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwAllocationGranularity;
}

// Windows

////////////////////////////////////////////////////////////////////////////////
}  // namespace abc