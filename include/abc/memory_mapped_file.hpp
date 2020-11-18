#pragma once

#include "abc/enum.hpp"
#include "abc/pointer.hpp"
#include "abc/result.hpp"

namespace abc
{
//////////////////////////////////////////////////////////////////////////

class memory_mapped_file
{
public:
    enum class access_type
    {
        read,
        write,
        readwrite
    };
    enum class cache_hint
    {
        normal,
        sequential,
        random
    };

    enum class map_range
    {
        whole = 0
    };

public:
    memory_mapped_file();
    memory_mapped_file(const std::string& filename,                                             //
                       size_t             mappedBytes = static_cast<size_t>(map_range::whole),  //
                       access_type        acess       = access_type::read,                      //
                       cache_hint         hint        = cache_hint::normal);
    ~memory_mapped_file();

    ABC_ENUM(OpenErrorCode, InvalidParameters, CannotOpenFile, FileNotFound, MappingAlreadyExists,
             Unknown);
    using open_error  = abc::error<OpenErrorCode>;
    using open_result = abc::result<void, open_error>;
    open_result open(const std::string& filename,                                             //
                     size_t             mappedBytes = static_cast<size_t>(map_range::whole),  //
                     access_type        acess       = access_type::read,                      //
                     cache_hint         hint        = cache_hint::normal);
    void        close();

    uint8_t        operator[](size_t offset) const;
    uint8_t        at(size_t offset) const;
    const uint8_t* getData(size_t offset = 0) const;
    uint8_t*       getData(size_t offset = 0);

    bool   is_open() const;
    size_t size() const;
    size_t mapped_size() const;
    size_t get_page_size() const;

    ABC_ENUM(RemapErrorCode, InvalidParameters);
    using remap_error  = abc::error<RemapErrorCode>;
    using remap_result = result<void, remap_error>;
    remap_result remap(uint64_t offsetMultipleOfPageSize, size_t mappedBytes);

protected:
    std::string m_filename;
    size_t      m_filesize;
    access_type m_access;
    cache_hint  m_cacheHint;

    size_t m_mappedBytes;
    void*  m_mappedFileView;

    struct pimpl;
    pimpl* m_impl = nullptr;
};

//////////////////////////////////////////////////////////////////////////
}  // namespace abc
