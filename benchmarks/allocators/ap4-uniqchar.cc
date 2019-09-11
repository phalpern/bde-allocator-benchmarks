/* ap4-uniqchar.cc                  -*-C++-*-
 *
 * Copyright (C) 2019 Halpern-Wight Software, Inc. All rights reserved.
 */

#include <bdlma_bufferedsequentialallocator.h>
#include <bsls_assert.h>

#include <iostream>
#include <bslstl_allocator.h>
#include <bslma_testallocator.h>
#include <string>
#include <set>
#include <cstdlib>

using namespace BloombergLP;

template <class T>
using bbpmr_set = std::set<T, std::less<T>, bsl::allocator<T>>;

#if defined(USE_ALLOC) || defined(USE_WINKOUT)
std::size_t unique_chars(const std::string& s)
{
    char allocBuffer[4096];
    bdlma::BufferedSequentialAllocator
        alloc(allocBuffer, sizeof(allocBuffer));

#ifdef USE_WINKOUT
    auto& uniq = *new (alloc) bbpmr_set<char>(&alloc);
#else
    bbpmr_set<char> uniq(&alloc);
#endif
    uniq.insert(s.begin(), s.end());
    return uniq.size();
    // Wink out of uniq would happen here
}

#else // If not USE_ALLLOC or USE_WINKOUT

std::size_t unique_chars(const std::string& s)
{
    bbpmr_set<char> uniq;
    uniq.insert(s.begin(), s.end());
    return uniq.size();
}

#endif

/* constexpr char text[] = R"Cicero(
But I must explain to you how all this mistaken idea of denouncing pleasure
and praising pain was born and I will give you a complete account of the
system, and expound the actual teachings of the great explorer of the truth,
the master-builder of human happiness. No one rejects, dislikes, or avoids
pleasure itself, because it is pleasure, but because those who do not know how
to pursue pleasure rationally encounter consequences that are extremely
painful. Nor again is there anyone who loves or pursues or desires to obtain
pain of itself, because it is pain, but because occasionally circumstances
occur in which toil and pain can procure him some great pleasure. To take a
trivial example, which of us ever undertakes laborious physical exercise,
except to obtain some advantage from it? But who has any right to find fault
with a man who chooses to enjoy a pleasure that has no annoying consequences,
or one who avoids a pain that produces no resultant pleasure?

On the other hand, we denounce with righteous indignation and dislike men who
are so beguiled and demoralized by the charms of pleasure of the moment, so
blinded by desire, that they cannot foresee the pain and trouble that are
bound to ensue; and equal blame belongs to those who fail in their duty
through weakness of will, which is the same as saying through shrinking from
toil and pain. These cases are perfectly simple and easy to distinguish. In a
free hour, when our power of choice is untrammelled and when nothing prevents
our being able to do what we like best, every pleasure is to be welcomed and
every pain avoided. But in certain circumstances and owing to the claims of
duty or the obligations of business it will frequently occur that pleasures
have to be repudiated and annoyances accepted. The wise man therefore always
holds in these matters to this principle of selection: he rejects pleasures to
secure other greater pleasures, or else he endures pains to avoid worse pains.

"de Finibus Bonorum et Malorum", sec 1.10.32-33, written by Cicero in 45 BC
English Translation by H. Rackham, 1914
)Cicero";
*/

// Sppedup is most dramatic with short strings having minimal duplication.
constexpr char text[] = R"(
the quick brown fox jumped over the lazy dog
THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG
0123456789)";

volatile int eat;

int main(int argc, char *argv[])
{
    int reps = 1000000;
    if (argc > 1) {
        reps = std::atoi(argv[1]);
    }

    std::string text_str(text);

    for (int i = 0; i < reps; ++i) {
        eat = unique_chars(text_str);
    }
}

/* End ap4-uniqchar.cc */
