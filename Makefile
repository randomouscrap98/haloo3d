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
CFLAGS = -Wall -O2 -I$(LIBS)

# Define the object files for the static library
STATICOBJS = $(LIBS)/mathc.o

.PHONY: clean

# Build the main lib. Since this is first, this is what the makefile will
# do by default. We create a static archive with all deps added
haloo3d.a: haloo3d.o $(STATICOBJS)
	ar -cvq $@ $< $(STATICOBJS)

# Rule to build .o files
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

# Laziness? Or just not knowing how to do this properly? IDK
# $(LIBS)/%.o: $(LIBS)/%.c $(LIBS)/%.h
	# $(CC) $(CFLAGS) -c $< -o $@

# # Rule to build main.o
# main.o: main.c math.h
# 	$(CC) $(CFLAGS) -c main.c


# Rule to clean the build files
clean:
	find . -name "*.o" -type f -delete
	find . -name "*.a" -type f -delete
	find . -name "a.out" -type f -delete


