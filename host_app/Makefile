# Makefile for Host Application + PSP Generator

CXX = g++
CXXFLAGS = -Wall -O3 -std=c++11
SRCDIR = src
BUILDDIR = build
DATADIR = ../5k_impl/data

# Two targets: PSP generator (runs on PC) and XRT host app (cross-compile for ARM)
PSP_GEN = psp_generator
HOST_APP = host_app

# For XRT cross-compilation (ARM)
# Set these when building for VCK190
SYSROOT ?= 
EDGE_COMMON_SW ?= 
ifeq ($(SYSROOT),)
    # Native build (x86) - for testing only
    XRT_CXXFLAGS = $(CXXFLAGS)
    XRT_LDFLAGS = 
else
    # Cross-compile for ARM
    CXX = aarch64-linux-gnu-g++
    XRT_CXXFLAGS = $(CXXFLAGS) --sysroot=$(SYSROOT) -I$(SYSROOT)/usr/include/xrt
    XRT_LDFLAGS = --sysroot=$(SYSROOT) -L$(SYSROOT)/usr/lib -lxrt_coreutil -pthread
endif

# PSP Generator sources (CPU tool)
PSP_GEN_SOURCES = $(SRCDIR)/psp_generator.cpp $(SRCDIR)/rambo.cpp
PSP_GEN_OBJECTS = $(BUILDDIR)/psp_generator.o $(BUILDDIR)/rambo.o

# Host App sources (XRT application)
HOST_APP_SOURCES = $(SRCDIR)/host.cpp
HOST_APP_OBJECTS = $(BUILDDIR)/host.o

all: directories $(PSP_GEN)
	@echo "====================================================="
	@echo " PSP Generator built successfully!"
	@echo " For XRT host app, use: make host"
	@echo "====================================================="

host: directories $(HOST_APP)
	@echo "====================================================="
	@echo " XRT Host Application built!"
	@echo " Deploy to VCK190 and run with .xclbin"
	@echo "====================================================="

directories:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(DATADIR)

# PSP Generator (x86)
$(PSP_GEN): $(PSP_GEN_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR)/psp_generator.o: $(SRCDIR)/psp_generator.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/rambo.o: $(SRCDIR)/rambo.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# XRT Host App (ARM or x86)
$(HOST_APP): $(HOST_APP_OBJECTS)
	$(CXX) $(XRT_LDFLAGS) -o $@ $^

$(BUILDDIR)/host.o: $(SRCDIR)/host.cpp
	$(CXX) $(XRT_CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR) $(PSP_GEN) $(HOST_APP)
	@echo "Cleaned build files"

distclean: clean
	rm -f $(DATADIR)/psp_in_*.bin $(DATADIR)/psp_in_*.txt $(DATADIR)/me2_out_*.txt
	@echo "Cleaned data files"

# Generate test data (10 PSPs per trunk, 8 trunks)
test: $(PSP_GEN)
	./$(PSP_GEN) -n 10 -t 8 -s 1802 -e 1500 -o $(DATADIR)

# Generate full dataset
generate: $(PSP_GEN)
	./$(PSP_GEN) -n 10 -t 8 -s 1802 -e 1500 -o $(DATADIR)
	@echo ""
	@echo "Generated PSP data for 80-pipeline architecture"
	@echo "Data location: $(DATADIR)"

.PHONY: all host directories clean distclean test generate
