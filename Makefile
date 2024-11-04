# --------------------------------------------------
#  haloopdy 2024 
# --------------------------------------------------
# - This makefile is designed to work on linux
# - By default it builds a static library haloo3d.a with everything you need
# - You can build individual examples with this makefile too
# - The library doesn't have to be built in the way specified here
# --------------------------------------------------

# Compiler and other things
LIBD = lib
BUILDD = build
CC = gcc
DEFINES = 
CFLAGS = $(DEFINES) -std=c99 -Wall -Wextra # -DH3DEBUG_SKIPWHOLETRI
ifdef MARCH 		# Allows you to define the architecture (usually not required)
	CFLAGS += -march=$(MARCH)
endif
ifndef FORCE 		# Force the build to move past warnings (disable warnings as errors)
	CFLAGS += -Werror
endif
ifdef DEBUG 		# Build in debug mode, usable in gdb/valgrind/etc
	CFLAGS += -O2 -g
else
	CFLAGS += -O3 -flto
endif


# Define the object files for the static library
FULLOBJS = $(BUILDD)/haloo3dex_unigi.o $(BUILD)/haloo3dex_obj.o
FULLOUT = $(BUILDD)/haloo3d.a
CORE = haloo3d.h

.PHONY: clean

# Build the main lib. Since this is first, this is what the makefile will
# do by default. We create a static archive with all deps added
$(FULLOUT): $(FULLOBJS) $(CORE)
	ar -cvr $@ $(FULLOBJS)

# Rule to build lib .o files
$(BUILDD)/$(LIBD)/%.o: $(LIBD)/%.c $(LIBD)/%.h
	mkdir -p $(BUILDD)/$(LIBD)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build .o files in main folder
$(BUILDD)/%.o: %.c %.h $(CORE)
	mkdir -p $(BUILDD)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build any example file. We ALWAYS need math so... link it
examples/%.exe: examples/%.o $(FULLOUT)
	$(CC) $(CFLAGS) $< $(FULLOUT) -o $@ -lm

# Rule to build any example file. We ALWAYS need math so... link it
tests/%.exe: tests/%.o $(FULLOUT)
	$(CC) $(CFLAGS) $< $(FULLOUT) -o $@ -lm

# Rule to clean the build files
clean:
	rm -rf $(BUILDD)
	find examples/ -name "*.exe" -type f -delete
	find tests/ -name "*.exe" -type f -delete

