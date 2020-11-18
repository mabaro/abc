#include "abc/core.hpp"
#include "abc/enum.hpp"
#include "abc/memory_mapped_file.hpp"
#include "abc/pointer.hpp"
#include "abc/result.hpp"

#include <stdexcept>
#include <cstdio>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "doctest/doctest.h"

#include <fstream>

TEST_CASE("abc - memory_mapped_file")
{
    const std::string filename       = "dummy_test_filename";
    auto              removeFileFunc = [](const std::string& filename) {
        if (std::ifstream(filename.c_str()).is_open())
        {
            system(abc::format("rm {}", filename).c_str());
            std::remove(filename.c_str());
        }
    };

    {  // should fail when file doesn't exist
        abc::memory_mapped_file mmf;
        removeFileFunc(filename);
        CHECK(std::ifstream(filename.c_str()).is_open() == false);
        auto openResult = mmf.open(filename, 1024, abc::memory_mapped_file::access_type::read);
        CHECK(openResult != abc::success);
    }
    {  // creates file if it doesn't exist (required non zero size)
        abc::memory_mapped_file mmf;
        auto openResult = mmf.open(filename, 1024, abc::memory_mapped_file::access_type::readwrite);
        CHECK(openResult == abc::success);
        ABC_ASSERT(openResult == abc::success, openResult.get_error().message());

        auto remapResult = mmf.remap(0, 200);
        ABC_ASSERT(remapResult == abc::success);

        int* intPtr = reinterpret_cast<int*>(mmf.getData(0));
        intPtr[1]   = 65;
        intPtr[100] = 66;
        intPtr[200] = 67;
    }
    {  // check contents is preserved
        abc::memory_mapped_file mmf;
        auto openResult = mmf.open(filename, 1024, abc::memory_mapped_file::access_type::read);
        CHECK(openResult == abc::success);
        ABC_ASSERT(openResult == abc::success, openResult.get_error().message());

        int* intPtr = reinterpret_cast<int*>(mmf.getData(0));
        CHECK(intPtr[1] == 65);
        CHECK(intPtr[100] == 66);
        CHECK(intPtr[200] == 67);
    }

    // clean temporaries
    removeFileFunc(filename);
    CHECK(std::ifstream(filename.c_str()).is_open() == false);
}

#include "abc/profiler.hpp"
TEST_CASE("abc - memory_mapped_file performance")
{
    const std::string filename       = "dummy_test_filename";
    auto              removeFileFunc = [](const std::string& filename) {
        if (std::ifstream(filename.c_str()).is_open())
        {
            system(abc::format("rm {}", filename).c_str());
            std::remove(filename.c_str());
        }
    };

    removeFileFunc(filename);

    const size_t k_numBytes   = 1024;
    const size_t k_numSamples = k_numBytes / sizeof(int);
    {  // prepare sample data
        std::ofstream ofs(filename.c_str(), std::ofstream::trunc | std::ofstream::binary);
        ABC_ASSERT(ofs.is_open());
        for (int i = 0; i < k_numSamples; ++i)
        {
            ofs.write(reinterpret_cast<char*>(&i), sizeof(i));
        }
    }

    {
        std::ifstream ifs(filename.c_str());
        ABC_ASSERT(ifs.is_open());
        int ibuff;
        PROFILE_BEGIN("std::ifstream");
        while (!ifs.eof())
        {
            ifs.read(reinterpret_cast<char*>(&ibuff), sizeof(ibuff));
        }
        PROFILE_END("std::ifstream");
    }
    {
        abc::memory_mapped_file mmf(filename, 0, abc::memory_mapped_file::access_type::read);
        ABC_ASSERT(mmf.is_open());
        const size_t availableBytes = mmf.mapped_size();
        ABC_ASSERT(availableBytes == k_numBytes);
        const uint8_t* data = mmf.getData();
        int            ibuff;
        PROFILE_BEGIN("memory_mapped_file");
        for (int i = 0; i < k_numBytes; ++i)
        {
            ibuff = data[i];
        }
        PROFILE_END("memory_mapped_file");
    }
}