// copymove.cc

// Compare shuffling data with copy vs. move assignment

//#define VERBOSE

#include <iostream>
#include <cstdlib>
#include <bsls_assert.h>

#include "allocont.h"

using Element = monotonic::vector<char>;

class Subsystem {
    // This class simulates a subsystem that might do various work on elements
    // that have different move vs. copy profiles

    monotonic::vector<Element> d_data;
    unsigned                   d_nextIdx;  // Next element to move from

    friend std::ostream& operator<<(std::ostream&, const Subsystem&);

  public:
    using allocator_type = monotonic::allocator<Element>;

    Subsystem(const allocator_type& alloc)
        // Create a subsystem using the specified 'alloc' allocator.
        : d_data(alloc), d_nextIdx(0) {
    }

    void initialize(int subsystemSize, unsigned elemSize)
        // Initialize this subsystem to 'subsystemSize' elements of 'elemSize'
        // length.
    {
        d_data.reserve(subsystemSize);
	for (unsigned i = 0; i < subsystemSize; ++i) {
            char c = 'A' + i & 31;
	    d_data.emplace_back(elemSize, c);
            d_data.back()[elemSize - 1] = '\0';
        }
    }

    Element& nextElement() {
        if (d_nextIdx >= d_data.size()) {
            d_nextIdx = 0;
        }
        return d_data[d_nextIdx++];
    }

    void access(unsigned accessCount)
        // Ping the system, simulating read/write accesses proportional to the
        // specified 'accessCount'.
    {
	if (d_data.empty()) {
	    return;  // nothing to access
	}

	auto it = d_data.begin();
	for (unsigned i = 0; i < accessCount; ++i) {
            (*it)[0] |= 5;
            if (++it == d_data.end()) {
	        it = d_data.begin();
	    }
	}
    }

    Element      & operator[](unsigned i)       { return d_data[i]; }
    Element const& operator[](unsigned i) const { return d_data[i]; }

    int size() const
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
        stream << ' ' << e.data();
    }

    return stream << " }";
}

void print(const char *label, const monotonic::vector<Subsystem>& array)
{
    std::cout << std::endl << label << ':' << std::endl;

    for (std::size_t i = 0; i < array.size(); ++i) {
	std::cout << array[i] << std::endl;
    }
}

void churn(monotonic::vector<Subsystem> *system, int churnCount)
{
if (churnCount < 0) {
    // Shuffle each element of each subsystem with the corresponding element
    // of all of the other subsystems using a Fisher-Yates suffle.
    BSLS_ASSERT(system.size() <= 31);

    const unsigned sS = (*system)[0].size(); // Subsystem size

    Element temp(system->get_allocator());
    Element *hole = &temp;

    for (int c = 0; c < -churnCount; ++c) {
    for (unsigned e = 0; e < sS; ++e) {
        for (unsigned i = system->size()-1; i > 0; --i) {
            size_t k = rand() % (i + 1);
            Element &toElem   = (*system)[i][e];
            Element &fromElem = (*system)[k][e];
#if defined(USE_COPY)
            // Swap using copy
            *hole = toElem;
            toElem = fromElem;
#elif defined(USE_MOVE)
            // std::cout << "Swapping [" << i << ',' << e << "] with ["
            //           << k << ',' << e << "]\n";
            // Swap using move
            *hole = std::move(toElem);
            toElem = std::move(fromElem);
#else
#  error "Either USE_COPY or USE_MOVE must be defined"
#endif
            hole = &fromElem;
        }
#if defined(USE_COPY)
        *hole = temp;
#elif defined(USE_MOVE)
        *hole = std::move(temp);
#endif
    }
    }
} else {
    Element tempElem(system->get_allocator());
    Element *toElem = &tempElem;

    // Rotate values through 'churnCount' elements.
    for (int c = 0; c < churnCount; ++c) {

        size_t k = rand() % system->size();

        // std::cout << "from k = " << k << std::endl;

        Element &fromElem = (*system)[k].nextElement();

#if defined(USE_COPY)
        *toElem = fromElem;               // Copy
#elif defined(USE_MOVE)
        *toElem = std::move(fromElem);    // Move
#else
#  error "Either USE_COPY or USE_MOVE must be defined"
#endif
        toElem = &fromElem;
    }

    // Finish rotation
#if defined(USE_COPY)
    *toElem = tempElem;               // Copy
#elif defined(USE_MOVE)
    *toElem = std::move(tempElem);    // Move
#endif
}

#ifdef VERBOSE
    print("configuration after churn", *system);
#endif
}

int main(int argc, const char *argv[])
{
    int a = 1;
    int numSubsystems    = argc > a ? atoi(argv[a++]) : 4;
    int subsystemSize    = argc > a ? atoi(argv[a++]) : 20;
    unsigned elemSize    = argc > a ? atoi(argv[a++]) : 3;
    unsigned accessCount = argc > a ? atoi(argv[a++]) : 18;
    int iterations       = argc > a ? atoi(argv[a++]) : 3;
    int churnCount       = argc > a ? atoi(argv[a++]) : 5;

#ifdef VERBOSE
    std::cout << std::endl
              << "numSubsystems = " << numSubsystems << std::endl
              << "subsystemSize = " << subsystemSize << std::endl
	      << "elementSize   = " << elemSize << std::endl
	      << "accessCount   = " << accessCount << std::endl
              << "iterations    = " << iterations << std::endl
              << "churnCount    = " << churnCount << std::endl;
#else
    std::cout << std::endl
              << "nS = " << numSubsystems << '\t'
              << "sS = " << subsystemSize << '\t'
              << "eS = " << elemSize << '\t'
	      << "aC = " << accessCount << '\t'
              << "it = " << iterations << '\t'
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

    numSubsystems = 1 << numSubsystems;

    subsystemSize = 1 << subsystemSize;

    elemSize = 1 << elemSize;

    accessCount *= subsystemSize;

    if (churnCount > 0)
        churnCount *= numSubsystems * subsystemSize;

    std::cout << "nS = " << numSubsystems << '\t'
              << "sS = " << subsystemSize << '\t'
              << "eS = " << elemSize << '\t'
              << "aC = " << accessCount << '\t'
              << "it = " << iterations << '\t'
              << "cC = " << churnCount << std::endl;

// ----------------------------------------------------------------------------

#ifdef VERBOSE
    std::cout << std::endl
              << "Create an array to hold 'numSubsystems' = "
              << numSubsystems
              << std::endl;
#endif

    char buffer[1024];
    BloombergLP::bdlma::BufferedSequentialPool pool(buffer, sizeof(buffer));
    monotonic::allocator<Subsystem> alloc(&pool);
    monotonic::vector<Subsystem> system(numSubsystems, alloc);

#ifdef VERBOSE
    std::cout << std::endl
              << "Populate each subsystem to have 'subsystemSize' = "
              << subsystemSize
              << std::endl;
#endif

    for (int i = 0; i < numSubsystems; ++i) {
	system[i].initialize(subsystemSize, elemSize);
    }

#ifdef VERBOSE
    print("Initial Configuration", system);
#endif

//    if (churnCount >= 0) {
        churn(&system, churnCount);
//    }

    for (int n = 0; n < iterations; ++n) {

#ifdef VERBOSE
        std::cout << std::endl << "***** iteration " << n << std::endl;
#endif

        for (int s = 0; s < numSubsystems; ++s) {

	    system[s].access(accessCount);

        }

#ifdef VERBOSE
	print("configuration after access", system);
#endif

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
