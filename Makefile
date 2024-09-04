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
DEFINES = -DH3D_SANITY_CHECK
CFLAGS = $(DEFINES) -std=c99 -Wall -Wextra
# CLIBFLAGS = -std=c99
ifdef MARCH 		# Allows you to define the architecture (usually not required)
	CFLAGS += -march=$(MARCH)
	# CLIBFLAGS += -march=$(MARCH)
endif
ifndef FORCE 		# Force the build to move past warnings (disable warnings as errors)
	CFLAGS += -Werror
endif
ifdef DEBUG 		# Build in debug mode, usable in gdb/valgrind/etc
	CFLAGS += -O2 -g
	# CLIBFLAGS += -O2 -g
else
	CFLAGS += -O3 -flto
endif


# Define the object files for the static library
STATICOBJS = $(BUILDD)/haloo3d.o $(BUILDD)/$(LIBD)/mathc.o
FULLOBJS = $(BUILDD)/haloo3dex_img.o $(BUILDD)/haloo3dex_obj.o \
					 $(BUILDD)/haloo3dex_gen.o $(BUILDD)/haloo3dex_print.o \
					 $(BUILDD)/haloo3dex_easy.o $(BUILDD)/$(LIBD)/FastNoiseLite.o \
					 $(BUILDD)/haloo3dex_console.o
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

# Rule to build SPECIFICALLY haloo3d.c
$(BUILDD)/haloo3d.o: haloo3d.c haloo3d.h haloo3d_trimacro.c haloo3d_trimacroswitch.c
	@echo "Building haloo3d.o special"
	mkdir -p $(BUILDD)
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
	@# find . -name "*.o" -type f -delete
	@# find . -name "*.a" -type f -delete
	@# find . -name "a.out" -type f -delete


