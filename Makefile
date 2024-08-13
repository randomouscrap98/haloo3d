# --------------------------------------------------
#  haloopdy 2024 
# --------------------------------------------------
# - This makefile is designed to work on linux
# - By default it builds a static library haloo3d.a with everything you need
# - You can build individual examples with this makefile too
# - The library doens't have to be built in the way specified here
# --------------------------------------------------

# Compiler and other things
LIBD = lib
BUILDD = build
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -O3 -flto
# Debug arguments maybe
# CFLAGS = -std=c99 -Wall -O2 -g -flto -I$(LIBS)
# Make sure to enable sse on non 64 bit systems/compilers
# CFLAGS = -std=c99 -Wall -Wextra -Werror -O3 -msse -flto -I$(LIBD)


# Define the object files for the static library
STATICOBJS = $(BUILDD)/haloo3d.o $(BUILDD)/$(LIBD)/mathc.o
FULLOBJS = $(BUILDD)/haloo3dex_img.o $(BUILDD)/haloo3dex_obj.o \
					 $(BUILDD)/haloo3dex_gen.o $(BUILDD)/haloo3dex_print.o
BASEOUT = $(BUILDD)/haloo3d.a
FULLOUT = $(BUILDD)/haloo3d_full.a

.PHONY: clean
.PHONY: full

# Build the main lib. Since this is first, this is what the makefile will
# do by default. We create a static archive with all deps added
$(BASEOUT): $(STATICOBJS)
	ar -cvr $@ $(STATICOBJS)

full: $(FULLOUT) ;

# To make life simpler, you can also include all the various extras in one lib
$(FULLOUT): $(BASEOUT) $(FULLOBJS)
	cp $< $@
	ar -vr $@ $(FULLOBJS)

# Rule to build lib .o files
$(BUILDD)/$(LIBD)/%.o: $(LIBD)/%.c $(LIBD)/%.h
	mkdir -p $(BUILDD)/$(LIBD)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build .o files in main folder
$(BUILDD)/%.o: %.c %.h
	mkdir -p $(BUILDD)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build any example file. We ALWAYS need math so... link it
examples/%.exe: examples/%.o $(FULLOUT)
	$(CC) $(CFLAGS) $< $(FULLOUT) -o $@ -lm

# Rule to clean the build files
clean:
	rm -rf $(BUILDD)
	find examples/ -name "*.exe" -type f -delete
	# find . -name "*.o" -type f -delete
	# find . -name "*.a" -type f -delete
	# find . -name "a.out" -type f -delete


