# You may need to set the following variables:
#
# CXX    - path to C++ compiler
# CC     - path to C compiler
# AR     - path to suitable 'ar' tool
# RANLIB - path to suitable 'ranlib' tool
# BDEWAF - path to 'waf' in BDE Tools (https://github.com/bloomberg/bde-tools)
#
# They may be overridden on the command line, e.g. for gcc 5.1 on 4 cores:
# $ make CXX=g++ CC=gcc BDEWAF=$HOME/bde-tools/bin/waf STDLIB= PARALLEL=-j4

CXX = clang++
CC = clang
AR = /usr/local/opt/llvm/bin/llvm-ar
RANLIB = /usr/local/opt/llvm/bin/llvm-ranlib
BDEWAF = waf

OPTIM = -g -O3 -DBSLS_ASSERT_LEVEL_OPT
# OPTIM = -g
LTO =
DEBUG =
PARALLEL =
GROWTH_SIZE = 20  # use 16 or less for testing

BSLSRC = ../..
BSL = $(BSLSRC)/groups/bsl
BDL = $(BSLSRC)/groups/bdl
BSLINC = $(INST)/include/bsl
BDLINC = $(INST)/include/bdl
BSLLIB = $(BSLSRC)/build/groups/bsl
BDLLIB = $(BSLSRC)/build/groups/bdl

# comment this out for gcc-5:
STDLIB = -stdlib=libc++
# If libc++ was built from source (not installed from a system package),
# then libc++abi may not have been linked into libc++. In this situation, the
# linker must be explicitly told to include it in the link, by uncommenting
# the line below.
#STDLIBABI = -lc++abi

INCLUDES = \
   -I$(BSL)/bsls -I$(BSL)/bslma -I$(BSL)/bslscm -I$(BSL)/bslh \
   -I$(BSL)/bsl+bslhdrs -I$(BSL)/bslstl -I$(BSL)/bslmf -I$(BSL)/bslalg \
   -I$(BDL)/bdlscm -I$(BDL)/bdlma

DEFS = -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DBSLS_IDENT_OFF
WAFCONFIGARGS = \
    --abi-bits=64 --cpp-std=11 --build-type release \
    $(PARALLEL)

LIBS = \
   -L$(BDLLIB) -lbdl -L$(BSLLIB) -lbsl \
   $(STDLIBABI) \
   -lpthread

# LIBS = \
#    -Wl,-Bstatic \
#    -L$(BDLLIB) -lbdl -L$(BSLLIB) -lbsl \
#    -Wl,-Bdynamic \
#    $(STDLIBABI) \
#    -lpthread

CFLAGS_BDE = $(DEBUG) $(OPTIM) $(LTO) $(DEFS) $(CFLAGS) -std=c99
CXXFLAGS_BDE = $(DEBUG) $(OPTIM) $(LTO) $(DEFS) $(STDLIB)
CXXFLAGS_LOCAL = $(CXXFLAGS_BDE) -std=c++14 $(INCLUDES) $(CXXFLAGS)
LDFLAGS_LOCAL = $(LIBS) $(LDFLAGS)

all: run

BINARIES = growth growth-DS159long shgrowth \
           locality-AS1 locality-AS7 locality-AS9 locality-AS13 \
           zation tention copymove-CP copymove-MV

build: $(BINARIES)

$(BINARIES): bde-tag

OUTPUT =  results/*-result results/growth-*-* \
    results/shuffle results/schedule-AS1 results/schedule-AS7

clean:
	rm -rf $(BINARIES) bde-tag $(BSLSRC)/build growth-*-* *-result

reallyclean: clean
	rm -rf $(OUTPUT)

bde-tag: Makefile
	( \
	cd $(BSLSRC) && \
	export CC='$(CC)' && \
	export CXX='$(CXX)' && \
	export AR='$(AR)' && \
	export RANLIB='$(RANLIB)' && \
	export CFLAGS='$(CFLAGS_BDE)' && \
	export CXXFLAGS='$(CXXFLAGS_BDE)' && \
	$(BDEWAF) configure $(WAFCONFIGARGS) && \
	$(BDEWAF) build -v \
	)
	touch bde-tag

# section 7
growth: growth.cc allocont.h bde-tag
	$(CXX) -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

growth-DS159long: growth-DS159long.cc allocont.h bde-tag
	$(CXX) -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

shgrowth: shgrowth.cc allocont.h bde-tag
	$(CXX) -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

# section 8
locality-AS1: locality.cc allocont.h
	$(CXX) -DSTDALLOC -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)
locality-AS7: locality.cc allocont.h
	$(CXX) -DCTMULTI -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)
locality-AS9: locality.cc allocont.h
	$(CXX) -DRTMULTI -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)
locality-AS13: locality.cc allocont.h
	$(CXX) -DRTMULTIMONO -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

# section 9
zation: zation.cc
	$(CXX) -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

# section 10
tention: tention.cc
	$(CXX) -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

# Copy vs move for AP0 and ACCU talk
copymove-CP: copymove.cc allocont.h
	$(CXX) -DUSE_COPY -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)
copymove-MV: copymove.cc allocont.h
	$(CXX) -DUSE_MOVE -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

# unique_chars example for AP4, demonstrating BufferedSequentalAllocator
ap4-uniqchar-default: ap4-uniqchar.cc
	$(CXX) -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)
ap4-uniqchar-alloc: ap4-uniqchar.cc
	$(CXX) -DUSE_ALLOC -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)
ap4-uniqchar-winkout: ap4-uniqchar.cc
	$(CXX) -DUSE_WINKOUT -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

run: run-locality run-zation run-tention run-growth \
     run-shuffle run-schedule-AS1 run-schedule-AS7 run-copymove

run-growth: growth
	@echo With GROWTH_SIZE 20 this may take a full day to complete:
	SIZE=$(GROWTH_SIZE) ; \
	for i in 04 05 06 07 08 09 10 11 12 13 14 15 16; do \
           echo ./growth $$SIZE $$i - ; \
           (cd results; ../growth $$SIZE $$i - | tee "growth-$$SIZE-$$i") ; \
        done && \
	(cd results; cat growth-$$SIZE-* >growth-result ) && \
	(cd results; ./reduce-growth-results; rm T*; )

run-growth-DS159long: growth-DS159long
	@echo With GROWTH_SIZE 20 this may take a full day to complete:
	SIZE=$(GROWTH_SIZE) ; \
	for i in 04 05 06 07 08 09 10 11 12 13 14 15 16; do \
           echo ./growth-DS159long $$SIZE $$i - ; \
           mkdir -p results-DS159long; \
           (cd results-DS159long; ../growth-DS159long $$SIZE $$i - | tee "growth-$$SIZE-$$i") ; \
        done && \
	(cd results-DS159long; cat growth-$$SIZE-* >growth-result ) && \
	(cd results-DS159long; ./reduce-growth-results; rm T*; )

run-shgrowth: shgrowth
	@echo With GROWTH_SIZE 20 this may take a full day to complete:
	SIZE=$(GROWTH_SIZE) ; \
	for i in 04 05 06 07 08 09 10 11 12 13 14 15 16; do \
           echo ./shgrowth $$SIZE $$i - ; \
           (cd results; ../shgrowth $$SIZE $$i - | tee "growth-$$SIZE-$$i") ; \
        done && \
	(cd results; cat growth-$$SIZE-* >growth-result ) && \
	(cd results; ./reduce-growth-results; rm T* )

run-locality: locality-AS1 locality-AS7 locality-AS9 locality-AS13
	@( \
	echo "********** using AS1 default std::allocator:"; \
	./test-locality ./locality-AS1 2>&1; \
	echo "********** using AS7 compile-time-bound multipool"; \
	./test-locality ./locality-AS7 2>&1; \
	echo "********** using AS9 polymorphic multipool:"; \
	./test-locality ./locality-AS9 2>&1; \
	echo "********** using AS13 polymorphic multipool backed by monotonic:"; \
	./test-locality ./locality-AS13 2>&1; \
	) | tee >results/locality-result
	(cd results; ./reduce-locality-results)

run-copymove: copymove-CP copymove-MV
	./test-copymove ./copymove-CP ./copymove-MV 2>&1 | tee results/copymove-result
	(cd results; ./reduce-copymove-results)
	cat results/copymove.csv

run-shuffle: locality-AS1
	( \
    mkdir -p results/shuffle && cd results/shuffle && \
    for i in ../../test-shuffle-scripts/s*; do \
        o=$${i##*/} ; \
        $$i  2>&1 | tee o$${o%%_*} ; \
    done ; \
    )

run-schedule-AS1: locality-AS1
	( \
    mkdir -p results/schedule-AS1 && cd results/schedule-AS1 && \
    for i in ../../test-schedule-scripts/z*; do \
        o=$${i##*/} ; \
        $$i locality-AS1 2>&1 | tee o$${o} ; \
    done ; \
    )

run-schedule-AS7: locality-AS7
	( \
    mkdir -p results/schedule-AS7 && cd results/schedule-AS7 && \
    for i in ../../test-schedule-scripts/z*; do \
        o=$${i##*/} ; \
        $$i locality-AS7 2>&1 | tee o$${o} ; \
    done ; \
    )

run-zation: zation
	(ulimit -v 5000000; time ./test-zation 2>/dev/null | tee zation-result)
	mv zation-result results/

run-tention: tention
	time ./test-tention | tee tention-result
	mv tention-result results/

run-ap4-uniqchar: ap4-uniqchar-default ap4-uniqchar-alloc ap4-uniqchar-winkout
	time ./ap4-uniqchar-default
	time ./ap4-uniqchar-alloc
	time ./ap4-uniqchar-winkout
