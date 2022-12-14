#-----------------------------------------------------------------------------
# This is the base name of the library	
#-----------------------------------------------------------------------------
LIBNAME = libcpp03_framework

#-----------------------------------------------------------------------------
# This is a list of directories that have compilable code in them.  If there
# are no subdirectories, this line must SUBDIRS = .
#-----------------------------------------------------------------------------
SUBDIRS = . 

#-----------------------------------------------------------------------------
# For x86, declare whether to emit 32-bit or 64-bit code
#-----------------------------------------------------------------------------
X86_TYPE = 64

#-----------------------------------------------------------------------------
# These are the language standards we want to compile with
#-----------------------------------------------------------------------------
C_STD = -std=gnu99
CPP_STD = -std=c++0x


#-----------------------------------------------------------------------------
# Declare the compile-time flags that are common between all platforms
#-----------------------------------------------------------------------------
CXXFLAGS =	\
-O2 -g -Wall \
-c -fmessage-length=0 \
-D_GNU_SOURCE \
-Wno-sign-compare \
-Wno-unused-value 


#-----------------------------------------------------------------------------
# Special compile time flags for ARM targets
#-----------------------------------------------------------------------------
ARMFLAGS = 


#-----------------------------------------------------------------------------
# If there is no target on the command line, this is the target we use
#-----------------------------------------------------------------------------
.DEFAULT_GOAL := x86

#-----------------------------------------------------------------------------
# Define the name of the compiler and what "build all" means for our platform
#-----------------------------------------------------------------------------
ALL       = x86 arm
ARM_PATH  = /opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-none-linux-gnueabi
ARM_CC    = $(ARM_PATH)-gcc
ARM_CXX   = $(ARM_PATH)-g++
ARM_AR    = $(ARM_PATH)-ar
X86_CC    = $(CC)
X86_CXX   = $(CXX)
X86_AR    = ar


#-----------------------------------------------------------------------------
# Create the names of the library files
#-----------------------------------------------------------------------------
ARM_LIB = $(LIBNAME)_arm.a
X86_LIB = $(LIBNAME)_x86.a


#-----------------------------------------------------------------------------
# Declare where the object files get created
#-----------------------------------------------------------------------------
ARM_OBJ_DIR := obj_arm
X86_OBJ_DIR := obj_x86


#-----------------------------------------------------------------------------
# Always run the recipe to make the following targets
#-----------------------------------------------------------------------------
.PHONY: $(X86_OBJ_DIR) $(ARM_OBJ_DIR) 


#-----------------------------------------------------------------------------
# We're going to compile every .c and .cpp file in each directory
#-----------------------------------------------------------------------------
C_SRC_FILES   := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c))
CPP_SRC_FILES := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cpp))


#-----------------------------------------------------------------------------
# In the source files, normalize "./filename" to just "filename"
#-----------------------------------------------------------------------------
C_SRC_FILES   := $(subst ./,,$(C_SRC_FILES))
CPP_SRC_FILES := $(subst ./,,$(CPP_SRC_FILES))


#-----------------------------------------------------------------------------
# Create the base-names of the object files
#-----------------------------------------------------------------------------
C_OBJ     := $(C_SRC_FILES:.c=.o)
CPP_OBJ   := $(CPP_SRC_FILES:.cpp=.o)
OBJ_FILES := ${C_OBJ} ${CPP_OBJ}


#-----------------------------------------------------------------------------
# We are going to keep x86 and ARM object files in separate sub-directories
#-----------------------------------------------------------------------------
X86_OBJS := $(addprefix $(X86_OBJ_DIR)/,$(OBJ_FILES))
ARM_OBJS := $(addprefix $(ARM_OBJ_DIR)/,$(OBJ_FILES))


#-----------------------------------------------------------------------------
# This rules tells how to compile an X86 .o object file from a .cpp source
#-----------------------------------------------------------------------------
$(X86_OBJ_DIR)/%.o : %.cpp
	$(X86_CXX) -m$(X86_TYPE) $(CPPFLAGS) $(CPP_STD) $(CXXFLAGS) -c $< -o $@

$(X86_OBJ_DIR)/%.o : %.c
	$(X86_CC) -m$(X86_TYPE) $(CPPFLAGS) $(C_STD) $(CXXFLAGS) -c $< -o $@


#-----------------------------------------------------------------------------
# This rules tells how to compile an ARM .o object file from a .cpp source
#-----------------------------------------------------------------------------
$(ARM_OBJ_DIR)/%.o : %.cpp
	$(ARM_CXX) $(CPPFLAGS) $(CPP_STD) $(CXXFLAGS) $(ARMFLAGS) -c $< -o $@

$(ARM_OBJ_DIR)/%.o : %.c
	$(ARM_CC) $(CPPFLAGS) $(C_STD) $(CXXFLAGS) $(ARMFLAGS) -c $< -o $@


#-----------------------------------------------------------------------------
# This rule builds the x86 library from the object files
#-----------------------------------------------------------------------------
$(X86_LIB) : $(X86_OBJS)
	$(X86_AR) crs $@ $(X86_OBJS)


#-----------------------------------------------------------------------------
# This rule builds the ARM library from the object files 
#-----------------------------------------------------------------------------
$(ARM_LIB) : $(ARM_OBJS)
	$(ARM_AR) crs $@ $(ARM_OBJS)


#-----------------------------------------------------------------------------
# This target builds all executables supported by this platform
#-----------------------------------------------------------------------------
all:	$(ALL)


#-----------------------------------------------------------------------------
# This target builds just the ARM executable
#-----------------------------------------------------------------------------
arm:	$(ARM_OBJ_DIR) $(ARM_LIB)


#-----------------------------------------------------------------------------
# This target builds just the x86 executable
#-----------------------------------------------------------------------------
x86:	$(X86_OBJ_DIR) $(X86_LIB)


#-----------------------------------------------------------------------------
# These targets makes all neccessary folders for object files
#-----------------------------------------------------------------------------
$(X86_OBJ_DIR):
	@for subdir in $(SUBDIRS); do \
	    mkdir -p -m 777 $(X86_OBJ_DIR)/$$subdir ;\
	done

$(ARM_OBJ_DIR):
	@for subdir in $(SUBDIRS); do \
	    mkdir -p -m 777 $(ARM_OBJ_DIR)/$$subdir ;\
	done


#-----------------------------------------------------------------------------
# This target removes all files that are created at build time
#-----------------------------------------------------------------------------
clean:
	rm -rf Makefile.bak makefile.bak $(LIBNAME).tgz *.a
	rm -rf $(X86_OBJ_DIR) $(ARM_OBJ_DIR)


#-----------------------------------------------------------------------------
# This target creates a compressed tarball of the source code
#-----------------------------------------------------------------------------
tarball:	clean
	rm -rf $(LIBNAME).tgz
	tar --create --exclude-vcs -v -z -f $(LIBNAME).tgz *


#-----------------------------------------------------------------------------
# This target appends/updates the dependencies list at the end of this file
#-----------------------------------------------------------------------------
depend:
	@makedepend    -p$(X86_OBJ_DIR)/ $(C_SRC_FILES) $(CPP_SRC_FILES) -Y 2>/dev/null
	@makedepend -a -p$(ARM_OBJ_DIR)/ $(C_SRC_FILES) $(CPP_SRC_FILES) -Y 2>/dev/null


#-----------------------------------------------------------------------------
# Convenience target for displaying makefile variables 
#-----------------------------------------------------------------------------
debug:
	@echo "SUBDIRS       = ${SUBDIRS}"
	@echo "C_SRC_FILES   = ${C_SRC_FILES}"
	@echo "CPP_SRC_FILES = ${CPP_SRC_FILES}"
	@echo "C_OBJ         = ${C_OBJ}"
	@echo "CPP_OBJ       = ${CPP_OBJ}"
	@echo "OBJ_FILES     = ${OBJ_FILES}"


#-----------------------------------------------------------------------------




