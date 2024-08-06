# --------------------------------------------------
#  haloopdy 2024 
# --------------------------------------------------
# - This makefile is designed to work on linux
# - By default it builds a static library haloo3d.a with everything you need
# - You can build individual examples with this makefile too
# - The library doens't have to be built in the way specified here
# --------------------------------------------------

# Compiler and other things
LIBS = lib
CC = gcc
CFLAGS = -std=c99 -Wall -O3 -flto -I$(LIBS)
# CFLAGS = -std=c99 -Wall -O2 -flto -I$(LIBS) -lm
# CFLAGS = -std=c99 -Wall -O2 -g -flto -I$(LIBS)

# Define the object files for the static library
STATICOBJS = $(LIBS)/mathc.o
FULLOBJS = haloo3dex_img.o haloo3dex_obj.o
BASEOUT = haloo3d.a
FULLOUT = haloo3d_full.a

.PHONY: clean
.PHONY: full

# Build the main lib. Since this is first, this is what the makefile will
# do by default. We create a static archive with all deps added
$(BASEOUT): haloo3d.o $(STATICOBJS)
	ar -cvr $@ $< $(STATICOBJS)

full: $(FULLOUT) ;

# To make life simpler, you can also include all the various extras in one lib
$(FULLOUT): haloo3d.a $(FULLOBJS)
	cp $< $@
	ar -vr $@ $(FULLOBJS)

# Rule to build .o files
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build any example file. We ALWAYS need math so... link it
examples/%.exe: examples/%.o $(FULLOUT)
	$(CC) $(CFLAGS) -o $@ $< $(FULLOUT) -lm

# Rule to clean the build files
clean:
	find . -name "*.o" -type f -delete
	find . -name "*.a" -type f -delete
	find . -name "a.out" -type f -delete


