// copymove.cc

// Compare shuffling data with copy vs. move assignment

//#define VERBOSE

#include <iostream>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <bsls_assert.h>

#include "allocont.h"

using Element = overaligned_monotonic::vector<char>;
using std::ptrdiff_t;
using std::size_t;

class Subsystem {
    // This class simulates a subsystem that might do various work on elements
    // that have different move vs. copy profiles

    overaligned_monotonic::vector<Element> d_data;

    friend std::ostream& operator<<(std::ostream&, const Subsystem&);

  public:
    using allocator_type = overaligned_monotonic::allocator<Element>;

    Subsystem(const allocator_type& alloc)
        // Create a subsystem using the specified 'alloc' allocator.
        : d_data(alloc) {
    }

    void initialize(ptrdiff_t subsystemSize, ptrdiff_t elemSize)
        // Initialize this subsystem to 'subsystemSize' elements of 'elemSize'
        // length.
    {
        d_data.reserve(subsystemSize);
	for (ptrdiff_t i = 0; i < subsystemSize; ++i) {
            char c = 'A' + (std::rand() & 31);
	    d_data.emplace_back(elemSize, c);
        }
    }

    void access(ptrdiff_t iterationCount)
        // Ping the system, simulating read/write accesses proportional to the
        // specified 'iterationCount'.
    {
	if (d_data.empty()) {
	    return;  // nothing to access
	}

	for (ptrdiff_t i = 0; i < iterationCount; ++i) {
            for (Element& e : d_data) {
                // XOR last 3 bits of each byte of element into first byte
                char x = 0;
                for (char c : e) {
                    x ^= c;
                }
                e[0] ^= (x & 7);
            }
	}
    }

    Element      & operator[](ptrdiff_t i)       { return d_data[i]; }
    Element const& operator[](ptrdiff_t i) const { return d_data[i]; }

    ptrdiff_t size() const
    {
	return d_data.size();
    }

    bool isEmpty() const
    {
	return d_data.empty();
    }
};

std::ostream& operator<<(std::ostream& stream, const Subsystem& object)
    // Write the contents of the specified 'object' to the specified
    // 'stream' in a single-line, human readable format.
{
    stream << '{';
    for (const Element& e : object.d_data) {
        stream << ' ';
        stream.write(e.data(), e.size());
    }

    return stream << " }";
}

void print(const char *label,
           const overaligned_monotonic::vector<Subsystem>& system)
{
    std::cout << std::endl << label << ':' << std::endl;

    for (const Subsystem& ss : system) {
	std::cout << ss << std::endl;
    }
}

void churn(overaligned_monotonic::vector<Subsystem> *system,
           ptrdiff_t churnCount)
{
    std::mt19937 rengine;
    std::uniform_int_distribution<ptrdiff_t> urand(0, system->size() - 1);

    const ptrdiff_t nS = system->size();
    const ptrdiff_t sS = (*system)[0].size(); // Subsystem size

    // Vector of indexes used to shuffle elements between subsystems
    std::vector<ptrdiff_t> randomSeq(nS);
    for (ptrdiff_t i = 0; i < nS; ++i) {
        randomSeq[i] = i;
    }

    Element tempElem(system->get_allocator());
    Element *hole = &tempElem;

    // Repeat shuffle 'churnCount' times (default 1)
    for (ptrdiff_t c = 0; c < churnCount; ++c) {
        for (ptrdiff_t e = 0; e < sS; ++e) {
            // Rotate values randomly through 'nS' elements at rank 'e'
            std::shuffle(randomSeq.begin(), randomSeq.end(), rengine);
            hole = &tempElem;
            for (auto k : randomSeq) {
                Element &fromElem = (*system)[k][e];

#if defined(USE_COPY)
                *hole = fromElem;               // Copy
#elif defined(USE_MOVE)
                *hole = std::move(fromElem);    // Move
#else
#  error "Either USE_COPY or USE_MOVE must be defined"
#endif
                hole = &fromElem;
            }
            // Finish rotation
#if defined(USE_COPY)
            *hole = tempElem;               // Copy
#elif defined(USE_MOVE)
            *hole = std::move(tempElem);    // Move
#endif
        }
    }

#ifdef VERBOSE
    print("configuration after churn", *system);
#endif
}

void testOveralignedBufferedSequentialPool()
    // Mini test driver for 'OveralignedBufferedSequentialPool'
{
    static const int CACHELINE_SIZE = 64;

    union {
        char  buffer[256];
        void *aligner;
    };

    OveralignedBufferedSequentialPool pool(buffer, sizeof(buffer),
                                           CACHELINE_SIZE);
    overaligned_monotonic::allocator<char> alloc(&pool);

    char *p = nullptr;
#define TEST_ALIGNED_ALLOC(n)                                           \
    BSLS_ASSERT_OPT(0 == ((intptr_t) (p = alloc.allocate(n)) & (n - 1)))

    TEST_ALIGNED_ALLOC(1);
    BSLS_ASSERT_OPT(buffer == p);
    TEST_ALIGNED_ALLOC(2);
    TEST_ALIGNED_ALLOC(4);
    TEST_ALIGNED_ALLOC(8);
    TEST_ALIGNED_ALLOC(CACHELINE_SIZE);
    BSLS_ASSERT_OPT(p > buffer);
    BSLS_ASSERT_OPT(CACHELINE_SIZE + 16 >= ((char*) p - buffer));
    BSLS_ASSERT_OPT(0 == ((intptr_t) (p = alloc.allocate(256)) &
                          (CACHELINE_SIZE - 1)));
}

int main(int argc, const char *argv[])
{
    int a = 1;
    ptrdiff_t numSubsystems    = argc > a ? atoi(argv[a++]) : 4;
    ptrdiff_t subsystemSize    = argc > a ? atoi(argv[a++]) : 20;
    ptrdiff_t elemSize         = argc > a ? atoi(argv[a++]) : 3;
    ptrdiff_t iterationCount   = argc > a ? atoi(argv[a++]) : 18;
    ptrdiff_t repCount         = argc > a ? atoi(argv[a++]) : 3;
    ptrdiff_t churnCount       = argc > a ? atoi(argv[a++]) : 1;

#ifdef VERBOSE
    std::cout << std::endl
              << "numSubsystems  = " << numSubsystems << std::endl
              << "subsystemSize  = " << subsystemSize << std::endl
	      << "elementSize    = " << elemSize << std::endl
	      << "iterationCount = " << iterationCount << std::endl
              << "repCount       = " << repCount << std::endl
              << "churnCount     = " << churnCount << std::endl;
#else
    std::cout << std::endl
              << "nS = " << numSubsystems << '\t'
              << "sS = " << subsystemSize << '\t'
              << "eS = " << elemSize << '\t'
	      << "iC = " << iterationCount << '\t'
              << "rC = " << repCount << '\t'
              << "cC = " << churnCount << std::endl;
#endif

    if (numSubsystems < 0) {
        numSubsystems = -numSubsystems - subsystemSize - elemSize;
        if (numSubsystems < 0) {
            std::cerr <<
                "Error: System size must be >= subsystemSize + elemSize\n";
            return -1;
        }
    }

    ptrdiff_t systemSize = numSubsystems + subsystemSize + elemSize;

    numSubsystems  = 1 << numSubsystems;
    subsystemSize  = 1 << subsystemSize;
    elemSize       = 1 << elemSize;
    iterationCount = iterationCount < 0 ? 0 : 1 << iterationCount;
    repCount       = repCount       < 0 ? 0 : 1 << repCount;
    churnCount     = churnCount     < 0 ? 0 : 1 << churnCount;

    std::cout << "nS = " << numSubsystems << '\t'
              << "sS = " << subsystemSize << '\t'
              << "eS = " << elemSize << '\t'
              << "aC = " << iterationCount << '\t'
              << "rC = " << repCount << '\t'
              << "cC = " << churnCount << std::endl;

// ----------------------------------------------------------------------------

#ifdef VERBOSE
    std::cout << std::endl
              << "Create an array to hold 'numSubsystems' = "
              << numSubsystems
              << std::endl;
#endif

    testOveralignedBufferedSequentialPool();

    static const int CACHELINE_SIZE = 64;

    union {
        char  buffer[1024];
        void *aligner;
    };

    OveralignedBufferedSequentialPool pool(buffer, sizeof(buffer),
                                           CACHELINE_SIZE);
    overaligned_monotonic::allocator<Subsystem> alloc(&pool);
    overaligned_monotonic::vector<Subsystem> system(numSubsystems, alloc);

#ifdef VERBOSE
    std::cout << std::endl
              << "Populate each subsystem to have 'subsystemSize' = "
              << subsystemSize
              << std::endl;
#endif

    for (ptrdiff_t i = 0; i < numSubsystems; ++i) {
	system[i].initialize(subsystemSize, elemSize);
    }

#ifdef VERBOSE
    print("Initial Configuration", system);
#endif

    churn(&system, churnCount);

    for (ptrdiff_t n = 0; n < repCount; ++n) {
        for (ptrdiff_t s = 0; s < numSubsystems; ++s) {
	    system[s].access(iterationCount);
        }
    }
}

// ----------------------------------------------------------------------------
// Copyright 2019 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
