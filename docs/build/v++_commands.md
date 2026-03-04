#### Confidential - Copyright © Fluid Topics


#### Displayed in the footer

###### Vitis Commands and Utilities

```
Launching the Vitis Unified IDE
v++ Command
emconfigutil Utility
kernelinfo Utility
launch_emulator Utility
manage_ipcache Utility
package_xo Command
platforminfo Utility
vitis-run Command
xrt-smi Utility
xbmgmt Utility
xclbinutil Utility
xrt.ini File
```

#### Displayed in the footer

##### ★

# Vitis Commands and Utilities

The AMD Vitis Design Suite includes a command-line interface and an integrated development environment (Vitis Unified IDE). The tool
uses the v++ command and the vitis-run command to compile and run design components. The reference materials contained in
this section include the following commands.

**v++ Command**
The v++ Command section provides a description of the compiler options (--compile) and includes the following commands.
AI Engine and HLS compiler modes.
Linking options (--link) for creating the device binary.
Packaging options (--package) for building boot files and generating an SD card for the system.
The v++ Command section also includes a discussion of --config command and configuration files.

**HLS Pragmas**
The HLS Pragmas section provides a description of pragmas for use in HLS components.

**xrt.ini file**
The Xilinx Runtime (XRT) file initializes XRT to produce reports, debug, and profiling data between the host and kernels. This file
is useful for emulation and hardware builds. You need to create the file manually when running the build process from the
command line.

The reference materials also include various AMD utilities for the Vitis tools and Xilinx Runtime (XRT). The utlities provide detailed
information about platform resources. The resources include SLR and memory resource availability to help you construct the v++
command line and manage the build and run process.

```
emconfigutil Utility
kernelinfo Utility
launch_emulator Utility
manage_ipcache Utility
package_xo Command
platforminfo Utility
vitis-run Command
xrt-smi Utility
xbmgmt Utility
xclbinutil Utility
```
```
Tip: The Xilinx Runtime (XRT) Architecture reference material is available on the Xilinx Runtime GitHub repository.
```
## Launching the Vitis Unified IDE

To launch the Vitis Unified IDE, follow these steps:

1. Use the following command to load the Vitis software platform environment.

```
source <Vitis_Installation_Directory>/settings64.sh
```
2. Use the following command to launch the Vitis Unified IDE.

```
vitis -w <workspace>
```
```
Here, <workspace> indicates a folder to hold all the contents of your design project. The workspace groups together the source
and data files that make up a design, or multiple designs, and stores the configuration of the tool for that workspace.
```
**Figure: Welcome Screen**


#### Displayed in the footer

###### ✎

###### ✎

For other supported launch modes, see Launch Options.
**Note:** Before launching the Vitis Unified IDE, you can set up other environmental settings to ensure that the tool can pick up these
settings. For example, you can set up Xilinx Runtime (XRT) for building and running data center acceleration applications:

```
source <XRT_Install_Path>/setup.sh
```
You can set up the platform repository path with the following example:

```
export PLATFORM_REPO_PATHS=<platform_path>
```
###### Launch Options

The Vitis Unified IDE supports the following modes:

**Workspace Mode**
Opens the Vitis Unified IDE with the specified workspace.

```
vitis -w <workspace>
```
**Analysis Mode**
Launches the tool directly into the Vitis Analyzer (see Working with the Analysis View (Vitis Analyzer)). You can review the
summary reports the tool generates during build, run, and debug.

```
vitis -a
```
**Interactive Mode**
Allows you to enter commands through the command-line interface, outside of the GUI.

```
vitis -i
```
```
Note: Type help() from the interactive command prompt to explore the available command modules.
```
**Batch Mode**
Executes the specified script and exits.

```
vitis -s <script>.py
```
**Jupyter Notebook Mode**
Launches Jupyter Notebook server with the Vitis environment and the front end UI in your default web browser.

```
vitis -j
```
```
You can use vitis_server command line interface in this environment as described in Vitis Interactive Python Shell.
```
You can use -h to print the options of vitis.


#### Displayed in the footer

##### ★

```
vitis -h
```
```
Syntax: vitis [-a | -w | -i | -s | -h | -v]
```
```
Options:
-a/--analyze [<summary file | folder | waveform file: *.[wdb|wcfg]>]
Open the summary file in the Analysis view.
Opening a folder opens the summary files found in the folder.
Open the waveform file in a waveform view tab.
If no file or folder is specified, opens the Analysis view.
-w/--workspace <workspace_location>
Launches Vitis IDE with the given workspace location.
-i/--interactive
Launches Vitis python interactive shell.
-s/--source <python_script>
Runs the given python script.
-j/--jupyter
Launches Vitis Jupyter Web UI.
-h/--help
Display help message.
-v/--version
Display Vitis version.
```
## v++ Command

This section describes the AMD Vitis™ compiler command, v++, and the various options it supports for building the device binaries.
v++ is a standalone command line utility with three command modes.

**--compile (-c)**
Compiles C++ to create AI Engine binaries and PL modules.
For embedded applications, to develop AI Engine kernels and compile the AI Engine graph applications into a libadf.a file, refer to
Compiling an AI Engine Graph Application in the _AI Engine Tools and Flows User Guide_ (UG1076). For developing and compiling
HLS PL kernel into a XO object file, refer to HLS Kernel Development in the _Embedded Design Development Using Vitis_
(UG1701).
To target a data center card, refer to Compiling C/C++ PL Kernels in the _Data Center Acceleration using Vitis_ (UG1700).

**--link (-l)**
For linking AI Engine kernels, HLS PL kernels, and Vitis subsystems into an extensible XSA platform to create a fixed XSA
hardware design or emulation design. Also used to create Vitis subsystems and Vitis metadata archive that can be imported into a
Vivado project for design iteration and completion.

**--package (-p**
For packaging AI Engine libadf.a binaries into .pdi and .xclbin images as described in Packaging for Vitis Flow in the
_Embedded Design Development Using Vitis_ (UG1701). Also generates an SD card image for Zynq Ultrascale target devices.

Beyond these three command modes, many other options are available to customize the build process. The following sections describe
these options. Some of the options are supported for all three command modes, and some options are specific to compilation, linking,
or packaging.

###### v++ General Options

The v++ command supports options and directives for compilation, linking, and packaging processes as described below.

**Tip:** Specify all v++ command-line options in a configuration file for use with the --config option (see Vitis Compiler Configuration
File). For example, the --platform option can be specified in a configuration file using the following syntax:

```
platform=xilinx_u50_gen3x16_xdma_5_202210_
```

#### Displayed in the footer

##### ★

### --advanced

**Applies to**
Compile, Link, Package

Specifies parameters and properties for use by the v++ command. See --advanced Options for more information.

### --board_connection

**Applies to**
Link

```
--board_connection
```
Specifies a dual in-line memory module (DIMM) board file for each DIMM connector slot. The board is specified using the
Vendor:Board:Name:Version (vbnv) attribute of the DIMM card as it appears in the board repository.
For example:

```
<DIMM_connector>:<vbnv_of_DIMM_board>
```
### -c | --compile

**Applies to**
Compile

```
--compile
```
Required for compilation, but mutually exclusive with --link and --package. Run v++ -c to generate XO files from kernel source
files.

### --clock

**Applies to**
Link

Provide a method for assigning clocks to kernels during the linking process. See --clock Options for more information.

### --config

**Applies to**
Compile, Link, Package

```
--config <config_file> ...
```
Specifies a configuration file containing v++ command options. The configuration file can be used to capture compilation, linking, or
packaging strategies, that can be easily reused by referring to the config file on the v++ command line. In addition, the config file allows
the v++ command line to be shortened to include only the options that are not specified in the config file. Refer to the Vitis Compiler
Configuration File for more information.

**Tip:** Multiple configuration files can be specified on the v++ command line. Vitis tools require a separate --config switch for each
file used. For example:

```
v++ -l --config system.cfg --config vivado.cfg ...
```

#### Displayed in the footer

###### !!

### --connectivity

**Applies to**
Link

Specifies directives to the v++ linker to integrate compiled components. See --connectivity Options for more information.

### --custom_script

**Applies to**
Compile, Link

```
--custom_script <kernel_name>:<file_name>
```
This option lets you specify custom Tcl scripts for compilation or linking during the build process. Use with the --export_script
option to create, edit, and run the scripts to customize the build process.
When used with the v++ --compile command, this option lets you specify a custom HLS script to be used when compiling the
specified kernel. The script lets you modify or customize the Vitis HLS tool. Use the --export_script option to extract a Tcl script.
This Tcl script is useful for Vitis HLS to compile the kernel, modify the script and resubmit using the --custom_script option to better
manage the kernel build process.
The argument lets you specify the kernel name and path to the Tcl script to apply to that kernel. For example:

```
v++ -c -k kernel1 --export_script ...
*** Modify the exported script to customize in some way, then resubmit. ****
v++ -c --custom_script kernel1:./kernel1.tcl ...
```
You can use this option with the v++ --link command for the hardware build target (-t hw). The option lets you specify the absolute
path to an run_script_map.dat file. The run_script_map.dat file contains a list of steps in the build process and Tcl scripts that the Vitis
or Vivado tool runs during the build process. You can edit run_script_map.dat to specify custom Tcl scripts to run at various steps in the
build process. Use the following steps to customize the Tcl scripts:

1. Run the build process to specify the --export_script option.

```
v++ -t hw -l -k kernel1 --export_script ...
```
2. Copy the Tcl scripts in the run_script_map.dat file for any of the steps you want to customize. For example, copy the Tcl file
    specified for the synthesis run, or the implementation run. You must copy the file to a separate location, outside of the project build
    structure.
3. Edit the Tcl script to add or modify any of the existing commands to create a new custom Tcl script.
4. Edit the run_script_map.dat file to point a specific implementation step to the new custom script.
5. Relaunch the build process using the --custom_script option. Specify the absolute path to the run_script_map.dat file as
    shown below.

```
v++ -t hw -l -k kernel1 --custom_script /path/to/run_script_map.dat
```
**Important:** When editing a custom synthesis run script, you must either comment out the lines related to the dont_touch.xdc file, or
edit the lines to point to a new user-specified dont_touch.xdc file. See the following for the specific lines to comment or edit.

```
read_xdc dont_touch.xdc
set_property used_in_implementation false [get_files dont_touch.xdc]
```
The synthesis run returns an error related to a missing dont_touch.xdc file if this is not done.

### --debug

**Applies to**
Link

Specifies the addition of debug IP core insertion into the hardware design. See --debug Options for more information.


#### Displayed in the footer

###### ✎

### -D | --define

**Applies to**
Compile, Link

```
--define <arg>
```
Valid macro name and definition pair: <name>=<definition>.
Predefines name as a macro with definition. This option passes to the v++ preprocessor.
**Note:** -D only applies to classic CLI (for example, "v++ -c -k" ) for compiling HLS kernels.

### --export_archive

**Applies to**
Link

```
--export_archive
```
This option instructs the v++ linker to generate a Vitis metadata archive (.vma) that can be imported into a Vivado hardware project in
lieu of a fixed hardware XSA or .xclbin file. The flow for this command and the use of the .vma file is described in Packaging for Vitis
Export to Vivado Flow in the _Embedded Design Development Using Vitis_ (UG1701).

### --export_script

**Applies to**
Compile, Link, Package

```
--export_script
```
This option runs the build process up to the point of exporting one or more script files, and then stops execution. You must use the --
custom_script option to complete the build process. The --custom_script option lets you edit the exported script, or list of
scripts, and then rerun the build using your custom scripts.
When used with the v++ --compile command, this option exports a Tcl script for the specified kernel, <kernel_name>.tcl. The script
can execute Vitis HLS, but stops the build process before actually launching the HLS tool. You can interrupt the build process to edit the
generated Tcl script, and then restart the build process using the --custom_script option. The following is an example.

```
v++ -c -k kernel1 -export_script ...
```
When used with the v++ --link command for the hardware build target (-t hw), this option exports a run_script_map.dat file in the
current directory. This file contains a list of steps in the build process. The file also contains Tcl scripts that Vitis and Vivado tool run
during those steps. You can edit the specified Tcl scripts, customizing the build process in those scripts, and relaunch the build using
the --custom_script option. Export the run_script_map.dat file using the following command:

```
v++ -l -t hw --export_script ...
```
### --freqhz

**Applies to**
Compile and Link

```
--freqhz <value>::<cu>[.<clk_pin>][,<cu_n>[.<clk_pin_n>]]
```
Specifies a frequency for a PL component during compilation, or for the system during linking. The --freqhz option overrides any
default clock frequency and applies the stated frequency during compilation and linking.
Specify a clock frequency in Hertz and a list of associated compute unit names and optionally their clock pins:


#### Displayed in the footer

###### !!

###### !!

**<value>**
Specify this in Hertz. For example, 150 MHz is specified as 150000000. Units of Hz and MHz are also accepted, for example
150000000Hz and 150MHz.

**[cu[.clk_pin]]**
Optionally specifes a PL kernel or instance of a PL kernel (Compute Unit) target for the specified clock frequency. The PL kernel or
instance of a PL kernel lets you specify a different frequency for different instances of kernels. You can also specify the clock pin of
the CU by adding the .clk_pin syntax. You can add multiple CU and clock pins in a single --freqhz option with a single
frequency value.

For example:

```
v++ -l -t hw -–platform <pfm_name> --freqhz=200000000:mm2s \
--freqhz=300000000:s2mm –config system.cfg
```
**Important:** The --freqhz option can consolidate the various methods or commands associated with specifying clocks in the AI
Engine, PL kernels, or system design. The --freqhz option takes precedence over other clock commands, and v++ can return an
error if multiple commands are used.

### --from_step

**Applies to**
Compile, Link, Package

```
--from_step <arg>
```
Specifies a step name for the Vitis compiler to start the build process. If intermediate results are available, the link process fast forwards
and begins execution at the named step when possible. You can run the build through a --to_step, and then resume the build
process at the --from_step. You can use the --list_steps option to determine the list of valid steps.
**Important:** --to_step/--from_step are sequential build options that require you to use --from_step to resume the build in the
same project directory that you used when starting the build with --to_step.
For example:

```
v++ --link --from_step vpl.update_bd
```
### -g

**Applies to**
Compile, Link

```
-g
```
Generates code for debugging the kernel during hardware emulation. Using this option adds features to facilitate debugging the kernel
as it is compiled.
For example:

```
v++ -g ...
```
### -h | --help

**Applies to**
Compile, Link, Package

```
-h
```
Prints the help contents for the v++ command. For example:

```
v++ -h
```

#### Displayed in the footer

##### ★

### --hls

**Applies to**
Compile

Specifies commands for the Vitis HLS synthesis process during kernel compilation.

### -I | --include

**Applies to**
Compile, Link

```
--include <arg>
```
Adds the specified directory to the list of directories for searching header files. This option passes to the Vitis compiler pre-processor.

**Classic Compile Mode (v++ -c -k)**
Usage: v++ -c -k --include <path>

**HLS Mode (v++ -c --mode hls)**
Usage: --include <path> is not recommended. Specify include paths in an HLS configuration file instead of with compiler
flags.

**AIE Mode (v++ -c --mode aie)**
Usage: v++ -c -k --include <path>

### --input_files <input_file>

**Applies to**
Compile, Link, Package

```
--input_files <input_file1> <input_file2> ...
```
Specifies a C/C++ kernel source file for v++ -c -k classic compilation or v++ -c --mode aie, or Xilinx object (XO) files for v++
linking.
For example:

```
v++ -l --input_files kernel1.xo kernelRTL.xo ...
```
```
Tip: You can also specify input files positionally without the --input_files option. For example:
```
```
v++ -l kernel1.xo kernelRTL.xo ...
```
The --input_files option can be used to specify positional arguments on the command line, but typically, file names are provided
directly without the option.
Example for Linking:
v++ -l -t hw --input_files mykernel.xo
is more commonly written as:
v++ -l -t hw mykernel.xo

**Classic Compilation:**
v++ -c -k mykernel --input_files mykernel.cpp is commonly used as: v++ -c -k mykernel mykernel.cpp

**AIE Mode:**
v++ -c --mode aie --input_files mygraph.cpp is typically: v++ -c --mode aie mygraph.cpp

**HLS Mode:**
Usage of --input_files with HLS mode: v++ -c --mode hls --input_files myhlskernel.cpp is not supported.

Linking:


#### Displayed in the footer

###### !!

The --input_files option can specify files like libadf.a, .xo, and .vss. Generally, these are specified directly as positional
arguments.
Packaging:
When packaging, --input_files can include libadf.a, .vss, fixed.xsa, and .xclbin files, especially for Zynq UltraScale
devices. The output from linking is typically fixed.xsa or .xclbin files.

### --interactive

**Applies to**
Link

```
--interactive [ impl ]
```
v++ configures the required environment and launches the Vivado tool with the implementation project.
Because you are interactively launching the Vivado tool, the linking process is stopped after the vpl step, which is the equivalent of
using the --to_step vpl option in your v++ command.
When you are done interacting with the Vivado tool, save the design checkpoint (DCP). You can then resume the Vitis compiler linking
process using the v++ --from_step rtdgen, or use the --reuse_impl or --reuse_bit options to read in the implemented DCP file
or bitstream.
For example:

```
v++ --interactive impl
## Interactively use the Vivado tool
v++ --from_step rtdgen
```
### -k | --kernel

**Applies to**
Compile

```
--kernel <arg>
```
Compiles only the specified kernel from the input file. Only one -k option is allowed per v++ command. Valid values include the name
of the kernel to be compiled from the input .c/.cpp kernel source code.
Vitis tools requires this option for C/C++ kernels. You must identify the kernel by -k or --kernel.

**Classic compilation**
Use -k with v++ -c to identify the kernel to compile from a source file.

**AIE mode**
Do not use -k with v++ -c --mode aie.

**HLS mode**
Do not use -k with v++ -c --mode hls.

**Invalid combination**
v++ -c -k --mode hls is not a valid command.

### --kernel_frequency

**Important:** This command specifies kernel frequencies only for legacy Alveo platforms with scalable clocks. Platforms using fixed
clocks, including both Alveo and embedded platforms, use the --clock Options for clock management. Refer to Managing Clock
Frequencies in the _Data Center Acceleration using Vitis_ (UG1700) for more information.

**Applies to**
Link

```
--kernel_frequency <freq> | <clockID>:<freq>[<clockID>:<freq>]
```

#### Displayed in the footer

##### ★

Specifies a user-defined clock frequency (in MHz) for the kernel, overriding the default clock frequency defined on the hardware
platform. The <freq> specifies a single frequency for kernels with only a single clock, or can be used to specify the <clockID> and the
<freq> for kernels that support two clocks.
The syntax for overriding the clock on a platform with only one kernel clock, is to simply specify the frequency in MHz:

```
v++ --kernel_frequency 300
```
To override a specific clock on a platform with two clocks, specify the clock ID and frequency:

```
v++ --kernel_frequency 0:
```
To override both clocks on a multi-clock platform, specify each clock ID and the corresponding frequency. For example:

```
v++ --kernel_frequency 0:300|1:
```
**Tip:** If Vivado place and route tools are unable to meet specification frequencies during design implementation, the tools can scale
the clock frequency to an achievable frequency.

### -l | --link

**Applies to**
Link

```
--link
```
This is a required option for the linking process, which follows compilation, but is mutually exclusive with --compile or --package.
Run v++ in link mode to link XO input files and generate a fixed hardware platform (XSA). For MPSoC and UltraScale targets,
generates an .xclbin file instead of fixed .xsa.

### --linkhook

**Applies to**
Link

Lets you customize the build process for the device binary by specifying Tcl scripts to be run at specific steps in the implementation
flow. See --linkhook Options for more information.

### --list_steps

**Applies to**
Compile, Link, Package

```
--list_steps
```
Lists valid run steps for a given target. This option returns steps that can be used in the --from_step or --to_step options. You
must specify this command with the following options:

```
-t | --target [hw_emu | hw ]:
[ --compile | --link ]: Specifies the list of steps from either the compile or link process for the specified build target.
```
For example:

```
v++ -t hw_emu --link --list_steps
```
### --log_dir

**Applies to**
Compile, Link, Package


#### Displayed in the footer

```
--log_dir <dir_name>
```
Specifies a directory to store log files into. If you do not specify --log_dir, the tool saves the log files to ./_x/logs. Refer to Output
Directories of the v++ Command for more information.
For example:

```
v++ --log_dir /tmp/myProj_logs ...
```
### --message_rules

**Applies to**
Compile, Link, Package

```
--message-rules <file_name>
```
Specifies a message rule file with rules for controlling messages. Refer to Using the Message Rule File for more information.
For example:

```
v++ --message_rules ./minimum_out.mrf ...
```
### --mode

**Applies to**
Compile, Link

Specifies a compilation mode for the v++ command, one of aie, hls or vss to target AI Engine, PL, or to create a Vitis subsystem,
respectively.

**v++ -c --mode aie**
Creates a libadf.a component. See _AI Engine Tools and Flows User Guide_ (UG1076) for more details. The compilation mode can
be used with configuration commands described under v++ Mode AI Engine.

**v++ -c --mode hls**
Creates an .xo kernel or IP an HLS component as described in _Vitis High-Level Synthesis User Guide_ (UG1399), and can be used
with configuration file commands described in v++ Mode HLS.

**v++ -l --mode vss**
The v++ --mode vss selects a link flow that creates a Vitis Subsystem (VSS) artifact. The accepted value is vss. If mode is not
specified, the standard link flow is used.
A VSS is a platform-independent, reusable design component consisting of compiled .xo and libadf.a sub-components.
The flow generates a subsystem package (with metadata and binaries) that can be instantiated and linked within a system context
on the target platform later.
See Linking a VSS Component in the _Embedded Design Development Using Vitis_ (UG1701) for details.

### --no_ip_cache

**Applies to**
Link

```
--no_ip_cache
```
Disables the IP cache for out-of-context (OOC) synthesis for Vivado Synthesis. Disabling the IP cache repository requires the tool to
regenerate the IP synthesis results for every build, and can increase the build time. However, it also results in a clean build, eliminating
earlier results for IP in the design.
For example:

```
v++ --no_ip_cache ...
```

#### Displayed in the footer

### -O | --optimize

**Applies to**
Link

```
--optimize <arg>
```
This option specifies the optimization level of the Vivado implementation results. Valid optimization values include the following:

```
0 : Default optimization. Reduces compilation time.
1 : Optimizes to reduce power consumption by running Vivado implementation strategy Power_DefaultOpt. This takes more
time to build the design.
2 : Optimizes to increase kernel speed. This option increases build time, but also improves the performance of the generated
kernel by adding the PHYS_OPT_DESIGN step to implementation.
3 : This optimization provides the highest level performance in the generated code, but compilation time can increase considerably.
This option specifies retiming during synthesis, and enables both PHYS_OPT_DESIGN and POST_ROUTE_PHYS_OPT_DESIGN
during implementation.
s: Optimizes the design for size. This reduces the logic resources of the device used by the kernel by running the Area_Explore
implementation strategy.
quick: Reduces Vivado implementation time, but can reduce kernel performance, and increases the resources used by the
kernel. This enables the Flow_RuntimeOptimized strategy for both synthesis and implementation.
```
For example:

```
v++ --link --optimize 2
```
### -o | --output

**Applies to**
Compile, Link, Package

```
-o <output_name>
```
Specifies the name of the output file generated by the v++ command. The compilation (-c) process output name must end with the XO
file suffix, for Xilinx object file. This applies for v++ -c -k classic compilation. The linking (-l) process output file must end with an .xsa
(or an .xclbin for Zynq platforms). The --export_archive command requires the .vma suffix. The --package command takes the .xsa as
input and outputs the .xclbin.
For example:

```
v++ -o krnl_vadd.xo
```
If --o or --output are not specified, the output file names default to the following:

**Compilation**
a.o

**Linking**
An .xsa (or an .xclbin for Zynq platforms)

**Packaging**
a.xclbin

### -p | --package

**Applies to**
Package

Specifies options for the Vitis compiler to package your design for either running emulation or running on hardware. See v++ Command
for more information.


#### Displayed in the footer

###### !!

### --part

**Applies to**
Compile, Link, Package

```
--part <part_name>
```
Specifies an AMD part for use in compiling, linking, and packaging the components of your design. The --part command can be used
to specify a device rather than a full platform or XSA. Specifying v++ --link --part <Versal Gen 1 part only> instructs the
tools to generate a simple hardware emulation platform for the specified AMD Versal™ device.
For example:

```
v++ --link --part xcvc1902-vsva2197-2MP-e-S --target hw_emu ...
```
### -f | --platform

**Applies to**
Compile, Link, Package

```
--platform <platform_name>
```
Specifies the name of a supported acceleration platform as determined by the $PLATFORM_REPO_PATHS environment variable, or the
full path to the extensible hardware .xsa or base platform .xpfm file. For a list of supported platforms for the release, see the Vitis
Software Platform Installation in the _Vitis Software Platform Release Notes_ (UG1742).
This is a required option for both compilation and linking. The --platform option accepts either a platform name, or the path to a
platform file xpfm, using the full or relative path. Packaging can accept a fixed.xsa as a platform.
**Important:** The specified platform and build targets for compiling, linking, and packaging must match. The --platform and -t
options specified when the XO file is generated by compilation, must be the --platform and -t used during linking, and packaging.
For more information, see platforminfo Utility.
For example:

```
v++ --platform vek385_base
```
### --profile

**Applies to**
Compile, Link

Specifies options to configure the Xilinx Runtime environment to capture application performance information. See --profile Options for
more information.

### --remote_ip_cache

**Applies to**
Link

```
--remote_ip_cache <dir_name>
```
Specifies the location of the remote IP cache directory for Vivado Synthesis to use during out-of-context (OOC) synthesis of IP. OOC
synthesis lets the Vivado synthesis tool reuse synthesis results for IP that have not been changed in iterations of a design. This can
reduce the time required to build your .xclbin files, due to reusing synthesis results.
When the --remote_ip_cache option is not specified the IP cache is written to the current working directory from which v++ was
launched. You can use this option to provide a different cache location, used across multiple projects for instance.
For example:

```
v++ --remote_ip_cache /tmp/IP_cache_dir ...
```

#### Displayed in the footer

###### !!

### --report_dir

**Applies to**
Compile, Link, Package

```
--report_dir <dir_name>
```
Specifies a directory to store report files into. If --report_dir is not specified, the tool saves the report files to ./_x/reports. Refer to
Output Directories of the v++ Command for more information.
For example:

```
v++ --report_dir /tmp/myProj_reports ...
```
### -R | --report_level

**Applies to**
Compile, Link, Package

```
--report_level <arg>
```
Valid report levels: 0 , 1 , 2 , estimate.
These report levels have mappings kept in the optMap.xml file. You can override the installed optMap.xml to define custom report
levels.

```
-R0 specification turns off all intermediate design checkpoint (DCP) generation during Vivado implementation. Turns on post-route
timing report generation.
The -R1 specification includes everything from -R0, plus report_failfast pre-opt_design, report_failfast post-
opt_design, and enables all intermediate DCP generation.
The -R2 specification includes everything from -R1, plus report_failfast post-route_design.
The -Restimate specification forces Vitis HLS to generate a design.xml file if it does not exist and then generates a System
Estimate report, as described in System Estimate Report in the Data Center Acceleration using Vitis (UG1700).
```
For example:

```
v++ -R ...
```
### --reuse_bit

```
--reuse_bit <arg>
```
**Applies to**
Link

Specifies the path and file name of generated bitstream file (.bit) to use when generating the device binary (xclbin) file. As described in
Using --to_step and Launching Vivado Interactively in the _Data Center Acceleration using Vitis_ (UG1700), you can specify the --
to_step option to interrupt the Vitis build process and manually place and route a synthesized design to generate the bitstream.
**Important:** The --reuse_bit option is a sequential build option that requires you to use the same project directory when resuming
the Vitis compiler with --reuse_bit that you specified when using --to_step to start the build.
For example:

```
v++ --link --reuse_bit ./project.bit
```
### --reuse_impl

```
--reuse_impl <arg>
```

#### Displayed in the footer

###### !!

##### ★

###### ✎

###### !!

**Applies to**
Link

Specifies the path and file name of an implemented design checkpoint (DCP) file to use when generating the device binary (xclbin) file.
The link process uses the specified implemented DCP to extract the FPGA bitstream and generates the xclbin. You can manually edit
theVivado project created by a previously completed Vitis build, or specify the --to_step option to interrupt the Vitis build process and
manually place and route a synthesized design, for instance. This allows you to work interactively with Vivado Design Suite to change
the design and use DCP in the build process.
**Important:** The --reuse_impl option is an incremental build option that requires you to use the same project directory when
resuming the Vitis compiler with --reuse_impl that you specified when using --to_step to start the build.
For example:

```
v++ --link --reuse_impl ./manual_design.dcp
```
### -s | --save-temps

**Applies to**
Compile, Link, Package

```
--save-temps
```
Directs the v++ command to save intermediate files/directories created during the compilation and link process. Use the --temp_dir
option to specify a location to write the intermediate files to.

**Tip:** This option is useful for debugging when you encounter issues in the build process.
**Note:** This option is mandatory if using Bootgen to package the SD card, flash or equivalent.
For example:

```
v++ --save-temps ...
```
### -t | --target

**Applies to**
Compile, Link, Package

```
-t [ hw_emu | hw ]
```
Specifies the build target, as described in Selecting the Build Target in the _Data Center Acceleration using Vitis_ (UG1700). The build
target determines the results of the compilation and linking processes. You can choose to build an emulation model for debug and test,
or build the actual system to run in hardware. The build target defaults to hw if -t is not specified.
**Important:** The specified platform and build targets for compiling and linking must match. The --platform and -t options specified
when the XO file is generated by compilation must be the --platform and -t used during linking.
The valid values are as follows:

```
hw_emu: Hardware emulation
hw: Hardware
```
For example:

```
v++ --link -t hw_emu
```
### --temp_dir

**Applies to**
Compile, Link, Package

```
--temp_dir <dir_name>
```

#### Displayed in the footer

###### !!

##### ★

###### ✎

This allows you to manage the location where the tool writes temporary files created during the build process. The temporary results
are written by the v++ compiler, and then removed, unless the --save-temps option is also specified.
If --temp_dir is not specified, the tool saves the temporary files to ./_x/temp. Refer to Output Directories of the v++ Command for
more information.
For example:

```
v++ --temp_dir /tmp/myProj_temp ...
```
### --to_step

**Applies to**
Compile, Link, Package

```
--to_step <arg>
```
Specifies a step name, for either the compile or link process, to run the build process through that step. You can use the --list_step
option to determine the list of valid compile or link steps.
The build process terminates after completing the named step. At this time, you can interact with the build results. For example,
manually accessing the HLS project or the Vivado Design Suite project to perform specific tasks before returning to the build flow,
launch the v++ command with the --from_step option.
**Important:** --to_step/--from_step are sequential build options that require you to use --from_step to resume the build in the
same project directory that you used when starting the build with --to_step.
You must also specify --save-temps when using --to_step to preserve the temporary files required by the Vivado tools. For
example:

```
v++ --link --save-temps --to_step vpl.update_bd
```
### --user_board_repo_paths

**Applies to**
Link

```
--user_board_repo_paths
```
Specifies an existing user board repository for DIMM board files. This value is pre-pended to the board_part_repo_paths property
of the Vivado project.

### --user_ip_repo_paths

**Applies to**
Link

```
--user_ip_repo_paths <repo_dir>
```
Specifies the directory location of one or more user IP repository paths to be searched first for IP used in the kernel design. This value
is appended to the start of the ip_repo_paths used by the Vivado tool to locate IP cores. IP definitions from these specified paths are
used ahead of IP repositories from the hardware platform (.xsa) or from the AMD IP catalog.

**Tip:** Multiple --user_ip_repo_paths can be specified on the v++ command line.
The following lists show the priority order in which IP definitions are found during the build process, from high to low.
**Note:** All of following entries can possibly include multiple directories in them.


#### Displayed in the footer

```
For the system hardware build (-t hw):
```
1. IP definitions from --user_ip_repo_paths.
2. Kernel IP definitions (vpl --iprepo switch value).
3. IP definitions from the IP repository associated with the platform.
4. IP cache from the installation area (for example, <Install_Dir>/Vitis/2025.2/data/cache/).
5. AMD IP catalog from the installation area (for example, <Install_Dir>/Vitis/2025.2/data/ip/)
For the hardware emulation build (-t hw_emu):
1. IP definitions and User emulation IP repository from --user_ip_repo_paths.
2. Kernel IP definitions (vpl --iprepo switch value).
3. IP definitions from the IP repository associated with the platform.
4. IP cache from the installation area (for example, <Install_Dir>/Vitis/2025.2/data/cache/).
5. $::env(XILINX_VITIS)/data/emulation/hw_em/ip_repo
6. $::env(XILINX_VIVADO)/data/emulation/hw_em/ip_repo
7. AMD IP catalog from the installation area (for example, <Install_Dir>/Vitis/2025.2/data/ip/)

For example:

```
v++ --user_ip_repo_paths ./myIP_repo ...
```
### -v | --version

```
-v
```
Prints the version and build information for the v++ command. For example:

```
v++ -v
```
### --vivado

**Applies to**
Link

Specifies properties and parameters to configure the Vivado synthesis and implementation environment prior to building the device
binary. See --vivado Options for more information.

###### v++ Compilation Options

The v++ command provides several features for compilation of PL kernels and AI Engine graphs. These options can be broken down
into the following categories:

```
--compile: These are the classic top-down compilation commands for the Vitis acceleration flow. This flow is still supported, and
in some cases necessary. For more information refer to the section on v++ Command.
--mode aie: This compilation mode in the v++ compiler enables the generation of the AI Engine graph application, and also
supports the use of the x86simulator and aiesimulator tools for analysis. These options are described in detail in v++ Mode
AI Engine.
--mode hls: This compilation mode in the v++ compiler enables the bottom-up generation of an HLS component, either for the
AMD Vivado™ IP flow or the Vitis Kernel flow as described in Vitis High-Level Synthesis User Guide (UG1399). The options to
configure and analyze an HLS component are described in v++ Mode HLS.
```
v++ General Compilation Options

The general v++ compilation options can appear on the command-line or in a configuration file. It can include the -c (or --compile)
option, a --mode when generating an AI Engine component or HLS component, the --platform or --part, the build --target,
and a --config file as shown below:

```
v++ -c --mode aie --platform vek385_base --target hw \
-config ./aie_config.cfg <input_files...>
```

#### Displayed in the footer

##### ★

###### !!

##### ★

**Tip:** Relative paths used on the command line are relative to the current working directory where the command is launched. Relative
paths in the config file are relative to the location of the config file.
Command options for v++ can be generally be used in a config file or on the command-line. However, the following options are only
available for use on the command-line:

**-c [ --compile ]**
Run compilation mode.

**--config <file_name>**
Specifies the config file path and name.

**--mode [ aie | hls ]**
Specifies the compilation mode as either compile an AI Engine component, or an HLS component.
**Important:** The absence of --mode puts the v++ -c command in the traditional top-down compilation mode for the Vitis kernel
flow as described in Compiling C/C++ PL Kernels in the _Data Center Acceleration using Vitis_ (UG1700).

**-h [ --help ]**
Print usage message for options. Can be combined with the --mode option to list the available options for the AI Engine or HLS
components.

**-v [ --version ]**
Prints version information.

**--input_files <arg>**
Specify input file(s). Input file(s) can also be specified positionally without using the --input_files option.

**-o [ --output ] <arg>**
Applies to: Link, Package. Specify the output file name and path. Default: a.xclbin for the v++ --link command.

**-f [--platform]**
This is a path to a Vitis platform file that defines the hardware and software components available for AI Engine components, HLS
components, and Applications. The argument can be a hardware platform (XSA) as recommended for hardware target in Versal
devices, or a base platform (XPFM).

**--log_dir <arg>**
Specify a directory to copy internally generated log files.

**--macro_dir <arg>**
Specify a macro to define a base directory: <macro_name>=<base_directory> Macros with a $ sign can be used to specify a
relative path.

**--report_dir <arg>**
Specify a directory to copy report files.

**--work_dir <arg>**
Optionally specify a working directory for the build and output files. For --mode aie the work directory defaults to ./Work. For -
-mode hls it defaults to the top-level function specified by hls.syn.top. This option can only be specified on the command
line.

**--aie_legacy**
Use legacy options for aiecompiler.

The following options can either be used in a config file or on the command-line:

```
Tip: As these are general options they do not need to be placed under a header (such as [AIE] or [HLS]) in the config file.
```
**-t [ --target ] <arg>**
The --target argument has different values when used with or without --mode.
Used with --mode the target defines the compilation target of the AI Engine or HLS component.
x86: Specifies compilation for x86 simulation of an AI Engine component, or for C-simulation of the HLS component.
hw: Specifies compilation for use with AI Engine simulator for AI Engine components, for C/RTL Co-simulation for HLS
components, or to run on the physical device for either components.
Used without --mode: the target defines the tradition build targets (hw_emu, hw) of the Application flow as described in
Creating an Integration Project Component.
Specify a target as hw_emu, or hw for the classic v++ compilation mode; or as x86 simulation for AI Engine compilation mode or
hw for AI Engine simulation. The default target is hw.


#### Displayed in the footer

##### ★

##### ★

**--part <arg>**
Specify part family or part value. Note this option is mutually exclusive with --platform or --hls.board.

**-D [ --define ] <name=definition>**
Predefine <name> as a macro with <definition>.

**-I [ --include ] <arg>**
Include the specified directory in the list of directories to be searched for header files.

v++ Mode AI Engine

The AI Engine compilation mode provides for the development, optimization, analysis, and export of AI Engine graph applications
(libadf.a). The AI Engine compilation mode uses the following command:

```
v++ -c --mode aie --target hw --platform vek385_base \
--work_dir ./myWork --config ./config.cfg <Input File>
```
```
v++ -c --mode aie --target hw: Indicates compilation of an AI Engine component for the HW target (default) indicating the
generation of the libadf.a file for use in AI Engine simulation and to run on the physical device. You can also specify x86sim for
the target to build the component for x86 functional simulator. The contents of the output directory depend on the target.
--platform vek385_base: Specifies the platform or physical device to build the component for.
--work_dir ./myWork: Specifies the work directory to use for building the AI Engine component.
```
```
Tip: By default, the compiler writes all outputs to a directory called Work in a sub-directory of the current directory where the
tool was launched, and creates a file called libadf.a in the current directory.
--config ./config.cfg: Specifies the AI Engine component config file containing a variety of options. The options are
available to the AI Engine compiler, x86 Simulator, and AI Engine Simulator. The options are part of the analysis and profiling tools
for the AI Engine component.
<Input File> specifies the data flow graph code that defines the main() application for the AI Engine graph. The input flow
graph uses a data flow graph language. For a description of the data flow graph, refer to Introduction to Graph Programming in the
AI Engine Kernel and Graph Programming Guide (UG1079).
```
Refer to the v++ -c --mode aie command options in the following sections. Add the AI Engine options to a configuration file under
the [AIE] section.

**Tip:** Some [AIE] options can be specified on the command line instead of in a config file, although using a config file is the
recommended approach. To use an [AIE] option on the command line add --aie. to the start of the command name from the
following sections. For example, to specify the constraints from the command line use --aie.constraints
The AI Engine component (with --target hw) can run through the aiesimulator using the following command:

```
aiesimulator --pkg-dir=./Work --i=../..
```
--pkg-dir specifies the Work directory where compilation occurs. --i specifies the input directory path for the simulator.
After compilation, the AI Engine component (with --target x86sim) can then run through the x86simulator using the following
command.

```
x86simulator --pkg-dir=./Work --i=../../
```
AI Engine Options

The following options apply to the AI Engine compilation process.

**constraints**
Specifies one or more constraint JSON files for location and bounding box, etc.

```
constraints=constraints.json
```

#### Displayed in the footer

###### ✎

###### ✎

###### ✎

###### !!

##### ★

**enable-partition**
The Vitis command line option to create an AI Engine partition. The partition is within the AI Engine array and specifies the starting
column, the number of columns and the partition's name.

```
enable-partition=<START_COLUMN>:<NUM_OF_COLUMN>:<PARTITION_NAME>
```
```
Note: For more details regarding the use of this setting, see Compiling AI Engine Graph for Independent Partitions in the AI
Engine Tools and Flows User Guide (UG1076).
```
**heapsize**
Heap size (in bytes) that the AI Engine uses.
In an AI Engine, the size of the heap determines the maximum amount of dynamically allocated memory at any given time.
The heap size for each AI Engine is measured in bytes. The default size is 1024 bytes.
The stack, heap, and sync buffer allocate up to the tile data memory. The sync buffer is a memory of size 32 bytes. It stores graph
run iteration information. Before changing the heap size to a different value, ensure that the sum of the stack, heap, and sync
buffer sizes does not exceed the tile data memory. For example, this size is 32768 bytes in an AI Engine device.
This option is also useful for allocating remaining file-scoped data not explicitly connecting to the user graph.

```
heapsize=512
```
**pl-freq**
Specifies the interface frequency (in MHz) for all PL kernels and PLIOs. The default frequency is a quarter of the AI Engine clock
frequency. The maximum frequency is half of the AI Engine clock frequency. You can provide the PL frequency specific to the
interface in the graph. You can override this value at v++ --link time.

```
pl-freq=500
```
```
Note: AMD recommend setting port frequency explicitly to avoid ambiguity. This is specifically important when using Vitis
Subsystem design flow.
```
**pl-register-threshold**
Specifies the frequency threshold in MHz for registered AI Engine-PL crossings. The default frequency is one-eighth of the AI
Engine clock frequency.

```
pl-register-threshold=300
```
```
Note: For more details regarding the use of this setting, see AI Engine Kernel and Graph Programming Guide (UG1079).
Important: The system ignores values above a quarter of the AI Engine array and uses a quarter instead.
```
**stacksize**
Stack size (in bytes) used by each AI Engine tile. The default stack size is set to 1024 bytes. Used as a standard compiler calling
convention including stack-allocated local variables and register spilling.
The stack is a region of memory used by programs to store temporary data during execution. In the case of an AI Engine, every
tile relies on a specific amount of stack memory to run the kernel mapped to it. The amount of memory allocated as stack space is
measured by the number of bytes assigned to each tile. This size of the stack is an essential consideration when designing and
optimizing the performance of the AI Engine.
By default, the stack size for each tile is set at 1024 bytes. This value takes into account factors like stack-allocated local variables
and register spilling. The factors are due to a register no longer having room for data, so it is "spilled" onto the stack temporarily to
free up register space. It is essential to optimize the stack size for each tile to prevent potential issues that could arise. These
issues include memory overflows that can lead to errors and other performance problems.

```
Tip: The system allocates up to 32768 bytes of data memory to the stack, heap, and sync buffer (32 bytes). Before changing
the stack size, ensure the sum of the stack, heap, and sync buffer sizes does not exceed 32768 bytes.
```
```
stacksize=512
```
CDO Options

The CDO options apply to generator code for graph configuration and initialization in configuration data object (CDO) format. The
system uses the options during SystemC-RTL simulation and during actual hardware execution.


#### Displayed in the footer

##### ★

**broadcast-enable-core**
Enables all AI Engine cores associated with a graph using broadcast. This option reserves one broadcast channel in the array for
core enabling purpose. This is enabled by default. You can be disable it by setting the argument to false.

```
broadcast-enable-core=false
```
Compiler Debugging Options

The Debugging options specify features of the compiler to enable troubleshooting errors during the build process. These features
include increased log details and consistency checking in the source code.

**adf-api-log-level**
ADF API log-level. Available values are as follows:
0: errors
1: level-0 + warnings
2: level-1 + info messages
3: level-2 + debug messages
The default is 2.

```
adf-api-log-level=3
```
**kernel-linting**
Perform consistency checking between graphs and kernels. Accepted values are true and false. The default is false.

```
kernel-linting=true
```
**quiet**
Suppress output of the compiler. Accepted values are true and false. The default is false.

```
quiet=true
```
**verbose**
Verbose output of the compiler. Accepted values are true and false. The default is false.

```
verbose=true
```
```
Tip: Can be used with log-level to increase the verbosity of the logs. For example:
```
```
log-level=4
```
Design Rule Check Options

The AI Engine mapper and router support configuring a list of Design Rule Checks (DRCs). Enter the DRC commands below into the
config file outside of any heading (for example [AIE]).

**drc.disable**
Disable the DRC rule check for the specified ID. If you disabled a check, it does not execute.

```
drc.disable=AIE-ROUTER-3
```
**drc.enable**
Enable a previously disabled DRC rule check for the specified ID.

```
drc.enable=AIE-ROUTER-3
```

#### Displayed in the footer

###### !!

###### ✎

**drc.severity**
Change the severity of a DRC rule check. The format of the argument is <ID>:<severity>[:context]
Where:
<ID> is the DRC rule ID which can be found on DRC messages reported by the tool.
<severity> represents the new severity to assign to the specified ID.
[context]

```
drc.severity=AIE-ROUTER-3:warning
```
**drc.waive**
Waive the Design Rule Check for the specified ID. The system still performs a check, but marks it as waived.

```
drc.waive=AIE-ROUTER-3
```
File Options

This section describes file options for file management activities through the config file. These options include specifying include files
and output file names.

**include**
Includes additional directories in the include path for the compiler front-end processing. Specify one or more include directories.

**output**
Specifies an output.json file the front-end produces for input data flow graph file. The output file passes to the back-end for
mapping and AI Engine device code generation. Other types of inputs ignores this file.

**output-archive**
Specifies the output archive name that contains compiled AI Engine artifacts (default: libadf.a). The output archive writes to the
component directory above the Work directory.

```
output-archive=myGraph.a
```
Module Specific Options

Options that apply to kernel implementation on AI Engine tiles.
**Important:** Only modified AI Engine kernels are recompiled in subsequent compilations of the AI Engine graph. Any un-modified
kernels are not recompiled. It is possible for the application of these options to require recompilation of the touching kernel source.

**Xchess arg**
Passes kernel specific options to the AI Engine compiler, which in turn uses the CHESS compiler to compile code for each AI
Engine. Specify the option with the string <kernel>:<optionid>=<value>. The compilation of source files on the kernel
mapped AI Engine includes the option string.

```
Xchess=main:darts.xargs=-nb
```
**Xelfgen arg**
Passes additional command-line options to the ELF generation phase of the compiler. Currently, the options run as a make
command to build all AI Engine ELF files. Provide the values for this option in quotes, useful if you want to pass special characters.
For example, Xelfgen="CLANG_OPTS='-g1 -mllvm --issue-limit=6".
For example, to limit the number of parallel compilations to four, use Xelfgen="-j4".
**Note:** If errors with bad_alloc appear in the log, or if the Vitis IDE crashes, it can be due to insufficient memory on your
workstation. A possible workaround is to limit the parallelism used by the compiler during code generation. Pass this option to the
Elf Generator Makefile (-j1 or -j2).

```
Xelfgen=-j2
```

#### Displayed in the footer

**Xmapper arg**
Passes additional command-line options to the mapper phase of the compiler. Useful for when the design is either failing to
converge in the mapping or routing phase, or for better performance via reduction in memory bank conflict.

```
Xmapper=DisableFloorplanning
```
**Xpreproc arg**
Passes general options to the PREPROCESSOR phase for all source code compilations AIE/PS/PL/x86sim of the AI Engine
component (e.g., -D...).

```
Xpreproc=-D<var>=<value>
```
**Xpslinker arg**
Pass general option to the PS LINKER phase of the AI Engine component (e.g., -L... -l...).

```
Xpslinker=-L<libpath> -l<libname>
```
**Xrouter arg**
Pass general option to the ROUTER phase.

```
Xrouter=dmaFIFOsInFreeBankOnly
```
**Xx86sim arg**
Pass x86sim-specific option to the compiler.

```
Xx86sim=clangStaticAnalyzer
```
**fast-floats**
Enable fast implementation for linear floating point scalar operations like add, sub, mul, and compare. Accepted values are true
and false. The default is false.

```
fast-floats=true
```
**fast-nonlinearfloats**
Enable fast implementation for non-linear floating point scalar operations like sine/cosine, sqrt, and inv. Accepted values are
true and false. The default is false.

```
fast-nonlinearfloats=true
```
**fastmath**
Enable fast implementations of float2fix, fplt, and fpge. Accepted values are true and false. The default is false.

```
fastmath=true
```
**float-accuracy**
Option available only for AI Engine-ML. It selects the required floating-point compute accuracy emulated with multiple bfloat16
numerical type operations.

```
float-accuracy=<arg>
```
```
Available argument values are as follows:
safe: Accuracy is slightly better than single precision floating-point (FP32).
fast: Improved performance with similar accuracy to FP32 (default).
low: Best performance with better accuracy than FP16 and bfloat16.
```
Event Tracing Options

This section describes the options to define the event-tracing characteristics of the compiled kernels and graph. These options prepare
the design for event tracing during runtime. See Event Trace Build Flow in the _AI Engine Tools and Flows User Guide_ (UG1076) for
more information.


#### Displayed in the footer

###### ✎

**event-trace**
Event trace configuration value.

```
event-trace=runtime
```
```
Note: This option selects events traced in hardware through an xsdb command line or a line in the file xrt.ini.
```
**event-trace-port**
Sets the AI Engine event tracing port. Accepted values are plio and gmio.
The gmio option uses the AI Engine-to-NoC pathway to capture trace data.
The alternative is to use plio. This option uses the AI Engine-PL pathway to capture trace data. The option uses programming
logic resources to capture data from AI Engine to DDR. The default value is gmio. This value is the recommended event-trace-
port configuration.

```
event-trace-port=plio
```
**graph-iterator-event**
Generates user_event() whenever the graph iterator increments. Provides the ability to delay the start of the hardware event
trace based on the graph iteration.

**num-trace-streams**
Specifies the number of trace streams. Allows up to 16 streams. The default value is 4.

```
num-trace-streams=8
```
**trace-plio-width**
PLIO width for trace streams: 32, 64 (default), 128

Optimization Options

**xlopt**
Enable kernel optimizations based on the specified value:
0: No kernel optimizations.
1: Computation of heap size, generation of guidance based on LLVM IR analysis, and insertion of loop pragmas.
2: The same optimizations as 1 with the addition of loop peeling for unrolled loops, and automatic inlining.

```
xlopt=2
```
**Xxloptstr**
Option string to enable or disable optimizations in XLOpt level 1,2.
-xlinline-threshold=T: set inlining threshold to T (default T = 5000, only for optlevel=2).
-annotate-pragma: insertion of loop unrolling, pipelining, and flattening pragmas (default = true)
-align-global-array=0,1 if set to 1, global arrays are aligned on 2 times the width of the physical memory bank. This automatic
alignment optimizes load/store performance. Load/Store ports bitwidth is twice the width of a physical bank and aligned on
even index physical bank. This option automatically aligns the data to the even index bank address.

```
Xxloptstr=-annotate-pragma
```
Miscellaneous Options

**swfifo-threshold**
Above this threshold, the system implements a FIFO as a DMA FIFO in an AI Engine application. The system generates an error
in an AI Engine-ML application. To avoid an error message in AI Engine-ML, increase the threshold. Increasing the threshold
requires additional resources to route the design.


#### Displayed in the footer

###### ✎

**evaluate-fifo-depth**
This option analyzes re-convergent data paths. Data can be sent on multiple paths. Sometimes data can re-converge and result in
a deadlock. Resolve deadlocks by adding FIFOs to the appropriate data paths.
The steps for evaluating and resolving deadlocks as a result of re-convergent data paths is as follows:

1. Compile the design with this option.
2. Run aiesimulator on the design.
3. Open Vitis Unified IDE with the simulation run_summary.
    Take note of the FIFO depth in theEstimated FIFO column in the Nets table. This FIFO depth value is the recommended
    value (in 32-bit words) that the fifo_depth(net) constraint uses on a specific net connection in the graph. The purpose is
    to resolve the deadlock situation.
4. Apply the recommended FIFO depth value using the fifo_depth constraint on specified nets on the graph, and recompile
    the design. In AI Engine-ML, if the recommendation is above the current fifo depth threshold, increase it using the swfifo-
    threshold option.

**disable-multirate**
Disable multirate in ADF graphs. Accepted values are true and false. The default is false.

```
disable-multirate=true
```
**known-tripcount**
This option converts an unknown trip count to a known trip count. The option takes no arguments and is disabled when omitted.

```
known-tripcount
```
**no-init**
This option disables initialization of window buffers in AI Engine data memory. This option enables faster loading of the binary
images into the SystemC-RTL co-simulation framework. Accepted values are true and false. The default is false.

```
no-init=true
```
**nodot-graph**
By default, the AI Engine compiler produces DOT and PNG files to visualize the user-specified graph and its partitioning onto the
AI Engines. This option can be used to eliminate the dot graph output. Accepted values are true and false. The default is false.

```
nodot-graph=true
```
**compile-testbench-only**
In HW compilation, the AI Engine compiler allows compilation of main only (graph.cpp test bench) using this flag. This helps to
compile graph.cpp separately. The option is useful in scenarios where only main() or the test bench is changed and there are no
changes in graph and kernels. This helps save compilation time of the complete AI Engine design and only compile test bench file
whenever necessary.

```
v++ --compile --mode aie --target=hw --aie.compile-testbench-only graph.cpp
```
**Note:** The AI Engine compiler assumes that the initial compilation of the AI Engine graph was successful when using compile-
testbench-only. Ensure that only the test bench code was modified and that no changes were made to graph or kernels.

v++ Mode HLS

The HLS compilation mode provides access to numerous features for the development, optimization, analysis, and export of Vitis
kernels (.xo) or Vivado IP (.xci) files. Access the HLS mode command with the following command:

```
v++ -c --mode hls -h [options] <input_files...>
```
Specify the HLS compilation options in a configuration file for the v++ command using the --config option. Place the HLS options
under a section head of [HLS] in the config file. For example, the following config file specifies the part, the source file, the test bench
files, and the flow target. part is not specified under the [HLS] header because this is a general option for the v++ compiler.

```
part=xcvu11p-flga2577-1-e
```

#### Displayed in the footer

###### !!

###### !!

###### !!

###### ✎

```
[hls]
clock=8
flow_target=vitis
syn.file=../../src/dct.cpp
syn.top=dct
tb.file=../../src/out.golden.dat
tb.file=../../src/in.dat
tb.file=../../src/dct_test.cpp
tb.file=../../src/dct_coeff_table.txt
syn.output.format=xo
clock_uncertainty=15%
```
The following sections describe the HLS mode command options.

HLS General Options

```
Important: The following options must appear in the HLS configuration file under the [hls] header.
```
**clock**
Specifies the clock period in ns or MHz (ns is default). If you don't provide a period, the system uses a default period of 10 ns.

```
clock=8ns
```
```
Important: If your HLS configuration file uses platform= instead of part= then you must also specify freqhz= instead of
clock= as shown here to change the default clock frequency of the platform.
```
**clock_uncertainty**
Specifies the clock period as a margin of uncertainty by HLS. The system subtracts the margin of uncertainty from the clock period
to create an effective clock period. The clock uncertainty is in units of ns, or as a percentage of the clock period. The clock
uncertainty defaults to 27% of the clock period. When specifying a value, the default unit is ns but % or MHz can also be used.

```
clock_uncertainty=15%
```
**flow_target**
Sets the flow target to synthesize either a Vitis kernel (vitis) or a Vivado IP (vivado). The Application Acceleration flow uses the
AMD Vitis™ kernel, while the embedded design flow uses the Vivado IP.
**Important:** Vivado IP and Vitis kernels have differences in the interface definition.

**relative_roots**
Specify an ordered list of one or more absolute paths. The list is searched to resolve relative file paths of source files and cflags
include paths with similar behavior to PATH environment variable. The first entry of the list can also be used as the root of any
output files paths that are relative.
All entries in the relative_roots list must be an absolute path or be a path that starts with one of the following which have special
meaning:
file: directory that contains the config file where the relative_root is specified
cwd: current working directory of the v++/vitis-run process
The list of relative_roots can be specified via multiple entries in a config file. Each successive value appends to the list of entries.
Within each entry multiple paths can be specified with a semicolon separator. Whitespace before and after the semicolon is
ignored/removed.

```
[hls]
relative_roots=/tmp/aaa ; /tmp/bbb ; file/../ccc ; cwd
syn.file=top.cpp
```
```
Note: If relative_roots is specified on the v++/vitis-run command it will be ignored and cause a warning message.
```

#### Displayed in the footer

###### !!

### C-Synthesis Sources

**syn.cflags**
Defines compilation flags applying to all syn.file defined source files for use during synthesis.

```
syn.cflags=-I../../src/
```
**syn.csimflags**
Defines compilation flags to be applied to all syn.file source files for use during C-simulation or RTL/Co-simulation.

**syn.file**
Specifies the file path and name of a source file to be used during synthesis of the HLS component. Multiple files require multiple
syn.file statements.
Specify the file paths as either absolute or relative. Relative paths are relative to the location of the config file, whether inside the
HLS component or outside the component.

```
syn.file=../../src/dct.cpp
```
**syn.file_cflags**
Applies a compilation flag for synthesis to the specified source file. Specify the file path and name first, followed by a comma,
followed by the cflags:

```
syn.file_cflags=../../src/dct.cpp,-I../../src/
```
**syn.file_csimflags**
Applies a compilation flag for simulation to the specified source file. Specify the file path and name first, followed by a comma,
followed by the csimflags.

```
syn.file_csimflags=../../src/dct.cpp,-Wno-unknown-pragmas
```
**syn.blackbox.file**
Specifies the JSON file for an RTL blackbox. The HLS compiler uses this file during synthesis and when running RTL/Co-
simulation.

```
syn.blackbox.file=../../RTL/fft.json
```
**syn.top**
Specifies the name of the function synthesized as the top-level function for the HLS component. Helps identify the top function in
source code that defines multiple functions.

```
syn.top=dct
```
```
Important: Functions that the top-level function calls also become part of the HLS component.
```
### Test Bench Sources

**tb.cflags arg**
Defines compilation flags for all source files defined in tb.file for simulation or co-simulation.

```
tb.cflags=-Wno-unknown-pragmas
```
**tb.file arg**
Specifies the file path and name of a test bench source file for HLS component simulation or co-simulation. Multiple files require
multiple tb.file statements.
The file paths can be either absolute or relative. Relative paths are relative to the location of the config file, whether inside the HLS
component or outside the component.

```
tb.file=../../src/dct_test.cpp
```

#### Displayed in the footer

**tb.file_cflags arg**
Applies a compilation flag for simulation or co-simulation to the specified test bench source file. Specify the file path and name
first, followed by a comma, followed by the cflags:

```
syn.file_cflags=../../src/dct.cpp,-Wno-unknown-pragmas
```
Array Partition Configuration

The syn.array_partition commands specify the default behavior for array partitioning for the entire design. You can override
these settings for specific arrays using the syn.directive.array_partition.

**syn.array_partition.complete_threshold <value>**
Sets the threshold for completely partitioning arrays. Arrays with fewer elements than the specified value are partitioned into
individual elements.

```
syn.array_partition.complete_threshold=12
```
**syn.array_partition.throughput_driven [ auto | off ]**
Enables automatic partial and/or complete array partitioning.
auto: Enables automatic array partitioning with smart trade-offs between area and throughput. This is the default value.
off: Disables automatic array partitioning.

```
syn.array_partition.throughput_driven=off
```
Array Stencil Configuration

The syn.array_stencil command specifies the default behavior of array stenciling for the entire design. You can override these
settings for specific arrays using the syn.directive.array_partition.

**syn.array_stencil.throughput_driven**
Enables automatic array stenciling with trade-offs between area and throughput.
auto: Enables automatic array stenciling with smart trade-offs between area and throughput.
off: Disables automatic array stenciling. This is the default value.

```
syn.array_partition.throughput_driven=auto
```
C-Simulation Configuration

The csim options apply to the C-simulation process for validating the design's C/C++ language. Refer to Running C Simulation in the
_Vitis High-Level Synthesis User Guide_ (UG1399) for more information.

**csim.O**
Enables optimizing compilation, which eliminates debug constructs. The default value is false. Compiles in debug mode to enable
debugging.

```
csim.O=true
```
**csim.argv**
Specifies an argument list for the behavioral test bench. The <arg> passes to the main() function in the C test bench.

```
csim.argv=arg1 arg2
```
**csim.clean**
Enables clean build. The default is false. Without this option, the design compiles incrementally.

```
csim.clean=true
```

#### Displayed in the footer

**csim.profile_tripcount**
Enables trip count profiling.

```
csim.profile_tripcount=true
```
**csim.ldflags**
Specifies the options passed to the linker for simulation. This option is typically used to pass include path information or library
information for the C/C++ test bench.

```
csim.ldflags=ldExample
```
**csim.mflags**
Provides options passed to the compiler for C simulation for speeding up compilation.

```
csim.mflags=mExample
```
**csim.profile_tripcount**
Enables loop trip count profiling, default is false.

```
csim.profile_tripcount=true
```
**csim.setup**
Creates a simulation binary in the csim directory of the current HLS component. You can launch simulation later from the compiled
executable. The default value is false. Simulation runs after completing setup.

```
csim.setup=true
```
Co-Simulation Configuration

The cosim options apply to the C/RTL Co-Simulation process for validating the RTL produced by HLS synthesis. The options include
using the C/C++ test bench used earlier in C-simulation. The options also include using the RTL design in behavioral simulation as
described in C/RTL Co-Simulation in Vitis HLS in the _Vitis High-Level Synthesis User Guide_ (UG1399).

**cosim.O**
Enables optimizing compilation which eliminates debug constructs. The default value is false. Performs compilation in debug mode
to enable debugging. Enabling optimized compilation of the C/C++ test bench and RTL wrapper increases compilation time, but
results in better runtime performance.

```
cosim.O=true
```
**cosim.argv**
Specifies an argument list for the behavioral test bench. The specified <arg> passes to the main() function in the C test bench.

```
cosim.argv=arg1 arg2
```
**cosim.compiled_library_dir**
Specifies the compiled library directory used during simulation with third-party simulators. The <arg> is the path name to the
compiled library directory. Compile the library ahead of time using the compile_simlib command as explained in the _Vivado
Design Suite Tcl Command Reference Guide_ (UG835).

```
cosim.compiled_library_dir=../../simLib
```
**cosim.coverage**
Enables the coverage feature during simulation with the VCS simulator.

```
cosim.coverage=true
```

#### Displayed in the footer

**cosim.disable_binary_tv**
Disables the binary test vector format in co-simulation.

```
cosim.disable_binary_tv=true
```
**cosim.disable_deadlock_detection**
Disables deadlock detection, and opening the Cosim Deadlock Viewer in co-simulation.

```
cosim.disable_deadlock_detection=true
```
**cosim.disable_dependency_check**
Disables dependency checks when running co-simulation.

```
cosim.disable_dependency_check=true
```
**cosim.enable_dataflow_profiling**
Enables the dataflow channel profiling to track channel sizes during co-simulation. You must enable this feature to capture
dataflow data as described in the Dataflow viewer section of the _Vitis High-Level Synthesis User Guide_ (UG1399).

```
cosim.enable_dataflow_profiling=true
```
**cosim.enable_fifo_sizing**
Enables automatic FIFO channel size tuning for dataflow profiling during co-simulation.

```
cosim.enable_fifo_sizing=true
```
**cosim.enable_tasks_with_m_axi**
Enables stable m_axi interfaces for use with hls::task.

```
cosim.enable_tasks_with_m_axi=true
```
**cosim.hwemu_trace_dir**
Specifies the location of test vectors generated during hardware emulation for a test bench during co-simulation. The
syn.rtl.cosim_trace_generation command generates the test vectors as described in RTL Configuration. This argument
specifies the kernel and instance name of the Vitis kernel in the hardware emulation simulation results. The goal is to help you
locate the test vectors for the HLS component.

```
cosim.hwemu_trace_dir=../../dct/dct_2
```
**cosim.ldflags <arg>**
Specifies the options passed to the linker for simulation. Passes include path information or library information for the C/C++ test
bench.

```
cosim.ldflags=ldExample
```
**cosim.mflags <arg>**
Provides options passed to the compiler for C simulation. Can speed up compilation.

```
cosim.mflags=mExample
```
**cosim.random_stall**
Enables random stalling of top level interfaces during co-simulation.

```
cosim.random_stall=true
```
**cosim.rtl**
Specifies either Verilog or VHDL as the language to use for C/RTL co-simulation. The default is Verilog.

```
cosim.rtl=vhdl
```

#### Displayed in the footer

**cosim.setup**
The system creates the simulation binary in the cosim directory of the current HLS component. However, this does not perform
simulation. You can launch simulation later from the compiled executable. The default is value is false. Co-simulation runs after
setup is complete.

```
cosim.setup=true
```
**cosim.stable_axilite_update**
Enables s_axilite to configure registers that are stable compared with the prior transaction.

```
cosim.stable_axilite_update=true
```
**cosim.tool**
Specifies the HDL simulator for co-simulating the RTL with the C test bench. The Vivado simulator (xsim) is the default, unless
otherwise specified.
auto
vcs
modelsim
riviera
isim
xsim
ncsim
xceilum

```
cosim.tool=modelsim
```
**cosim.trace_level**
Determines the level of waveform trace data to save during C/RTL co-simulation.
none does not save trace data. This is the default.
all results in all port and signal waveforms being saved to the trace file.
port only saves waveform traces for the top-level ports.
port_hier save the trace information for all ports in the design hierarchy.

```
cosim.trace_level=port
```
```
The trace file saves in the sim/Verilog or sim/VHDL folder of the component when the simulation executes. The location depends
on the selection in the cosim.rtl option.
```
**cosim.user_stall**
Specifies the JSON stall file used during co-simulation. The stall file can be generated using the cosim_stall command.

```
cosim.user_stall=../../stall.json
```
**cosim.wave_debug**
Opens the Vivado simulator GUI to view waveforms and simulation results. Enables waveform viewing of all processes in the
generated RTL. The proccesses include dataflow and sequential processes. Use this option only when using the Vitis simulator for
co-simulation. You must set cosim.tool=xsim. See Viewing Simulation Waveforms for more information.

```
cosim.wave_debug=true
```
Compile Options

The syn.compile commands specify the default behavior for compilation of the HLS component.

**syn.compile.design_size_max_warning**
Specifies the design size that triggers a warning related to slow compilation or poor QoR.

```
syn.compile.design_size_maximum_warning=300000
```

#### Displayed in the footer

###### !!

**syn.compile.enable_auto_rewind**
When set to true, enables an alternative HLS implementation for pipelined loops that uses automatic loop rewind. See the
Rewinding Pipelined Loops for Performance section of the _Vitis High-Level Synthesis User Guide_ (UG1399) for additional
information.

```
syn.compile.enable_auto_rewind=1
```
**syn.compile.ignore_long_run_time**
Disables the long runtime warning.

```
syn.compile.ignore_long_run_time=1
```
**syn.compile.name_max_length <arg>**
Specifies the maximum length of function names. If the length is over the threshold, the system truncates the last part of the name.

```
syn.compile.name_max_length=13
```
**syn.compile.no_signed_zeros**
The no_signed_zeros option ignores the signedness of floating point zero. By ignoring the signedness, the compiler can
perform aggressive optimizations on floating point operations.

```
syn.compile.no_signed_zeros=1
```
**syn.compile.performance_budgeter**
By enabling this option, the top-level performance pragma conducts a comprehensive, design-wide performance analysis. Much
like selecting the most efficient route from point A to point B among many possibilities, the analysis considers multiple optimization
strategies across the entire design. This analysis helps identify the best path to meet performance targets.
enable: Activates the performance budget for a comprehensive, design-wide performance analysis.
disable: Deactivates the performance budget, preventing any comprehensive analysis.
auto: Automatically enables design-wide performance analysis if the system detects a performance pragma.

**syn.compile.pipeline_flush_in_task arg**
Specifies that pipelines in hls::tasks is flushing (flp) by default to reduce the probability of deadlocks in C/RTL Co-simulation.
This option applies to pipelines that achieve an II=1 with the default option of ii1. However, you can also specify it as applying
always to enable flushing pipelines in either hls::tasks or dataflow, or can be completely disabled using never.
**Important:** Flushing pipelines (flp) are compatible with the rewind option specified in the PIPELINE pragma or directive when
the syn.compile.enable_auto_rewind option is also specified.
**always**
Always make pipelines flushable in hls::tasks or dataflow regardless of II.
**never**
Never make pipeline flushable unless specifically overridden by other directives or pragmas.
**ii1**
Make pipelines that achieve II=1 flushable in hls::tasks. This is the default setting.

```
syn.compile.pipeline_flush_in_task=always
```
**syn.compile.pipeline_loops arg**
Loops with a tripcount equal to or greater than the value you specify in this setting is pipelined automatically.

```
syn.compile.pipeline_loops=20
```
**syn.compile.pipeline_style arg**
Set default pipeline style, this is a preference not a hard constraint. Refer to Flushing Pipelines and Pipeline Types in _Vitis High-
Level Synthesis User Guide_ (UG1399) for more information. The three styles are: stallable (stp), flushable (flp), and free-
running (frp). The default is free-running.

```
syn.compile.pipeline_style=flp
```

#### Displayed in the footer

###### !!

##### ★

###### !!

**syn.compile.pragma_strict_mode**
Enable errors instead of warnings for unrecognized and improper pragma syntax.

```
syn.compile.pragma_strict_mode=1
```
**syn.compile.unsafe_math_optimizations**
The unsafe_math_optimizations option ignores the signedness of floating point zero and enables associative floating point
operations. Allows compiler to do aggressive optimizations on floating point operations.

```
syn.compile.unsafe_math_optimizations=1
```
```
Important: Using this option can change the result of any floating point calculations and result in a mismatch in C/RTL co-
simulation. Ensure your test bench is tolerant of differences and checks for a margin of difference, not exact values.
```
**syn.compile_use_csim_directives**
Use directives generated by C simulation during C synthesis. Default is false.

```
syn.compile_use_csim_directives=true
```
Dataflow Configuration

### Synthesis: Dataflow Options

The syn.dataflow commands configure the dataflow analysis for the whole design. These settings specify the default memory
channel and FIFO depth used by syn.directive.dataflow.

**syn.dataflow.default_channel**
By default, when configuring RAM memory in ping-pong fashion and use dataflow pipelining, the Vitis tool uses the data between
functions or loops. When using streaming (where the data is always read and written in consecutive order), a FIFO memory is
more efficient. You can select the FIFO memory as the default memory type.
The available channels are fifo and pingpong. The default is pingpong.

```
syn.dataflow.default_channel=fifo
```
```
Tip: Arrays must be set to streaming using the set_directive_stream command to perform FIFO accesses.
```
**syn.dataflow.disable_fifo_sizing_opt**
Disable FIFO sizing optimizations that increase resource usage; this can improve performance and reduce deadlocks.

```
syn.dataflow.disable_fifo_sizing_opt=1
```
**syn.dataflow.fifo_depth**
An integer value specifying the default depth of FIFOs. This option has no effect when using pingpong memories. By default, the
depth of FIFOs in the channel sets to the size of the largest producer or consumer (whichever is largest). In some cases, this
approach can be too conservative and results in FIFOs larger than needed. This option can specify the depth when you know the
FIFOs are larger than required.

```
syn.dataflow.fifo_depth=6
```
```
Important: Incorrect use of this option can result in a design that fails to operate correctly.
```

#### Displayed in the footer

**syn.dataflow.override_user_fifo_depth**
Specify a depth for every hls::stream, overriding any user settings.

```
syn.dataflow.override_user_fifo_depth=12
```
```
This setting is useful for checking if a deadlock is due to insufficient FIFO depths in the design. By setting the override to a very
large value, if there is no deadlock, you can:
Use the FIFO depth profiling options of co-simulation.
Use the GUI to find the minimum depth that ensures performance and avoids deadlocks.
A large value for example is the maximum depth printed by co-simulation at the end of simulation.
```
**syn.dataflow.scalar_fifo_depth**
An integer value specifying the minimum depth of the scalar value propagation FIFOs. See the Specifying Compiler-Created FIFO
Depth section of the _Vitis High-Level Synthesis User Guide_ (UG1399). These FIFOs are used to forward the value of scalar
arguments of the dataflow regions to processes which have predecessors in the region itself. They do not affect functional
correctness. An sufficient automatically computed size can result in loss of performance and even deadlock.

```
syn.dataflow.scalar_fifo_depth=4
```
```
When this option is not specified, the minimum depth is the value of the syn.dataflow.fifo_depth option, or it is 2. As a rule
of thumb, a good value is the average number of times the process forwarding the scalar value can start before the last process
that reads it starts.
```
**syn.dataflow.start_fifo_depth**
An integer value specifying the minimum depth of the start propagation FIFOs. See Specifying Compiler-Created FIFO Depth in
the _Vitis High-Level Synthesis User Guide_ (UG1399). These FIFOs forward the ap_start handshake signal to processes that
have predecessors in the region. They do not affect functional correctness, but an insufficient automatically computed size can
result in loss of performance.

```
syn.dataflow.start_fifo_depth=5
```
```
When you do not specify this option, the minimum depth is either the value of the syn.dataflow.fifo_depth option, or the
value 2. As a rule of thumb, a good value is the expected average number of times a process should be allowed to start in
advance compared to its successors.
```
**syn.dataflow.strict_mode**
Sets the severity for dataflow canonical form messages. The available modes are: error, warning, off. The default is
warning.

```
syn.dataflow.strict_mode=error
```
**syn.dataflow.strict_stable_sync**
Force synchronization of stable ports with ap_done. Refer to the Stable Arrays section of the _Vitis High-Level Synthesis User
Guide_ (UG1399) for more information.

```
syn.dataflow.strict_stable_sync=1
```
**syn.dataflow.task_level_fifo_depth**
Default task-level FIFO depth. For FIFOs automatically created to transfer scalars between processes. A FIFO is synchronized by
ap_ctrl_chain. The write is the ap_done of the producer, the read is the ap_ready of the consumer. Like a PIPO in terms of
synchronization, and like a FIFO in terms of access.

```
syn.dataflow.task_level_fifo_depth=7
```
Debug Options

The syn.debug commands enable debugging in the HLS component and specify where to write debugging output.


#### Displayed in the footer

##### ★

##### ★

**syn.debug.enable**
Enable debug file generation. If you do not enable it, the HLS you can optimize the component during compilation. However, the
component cannot support debug.

```
syn.debug.enable=1
```
```
Tip: This option relates to the v++ -c -g option. You must manually enable this option in the HLS component when
debugging at the Application level.
```
**syn.debug.directory**
Specifies the location of to write the output of HLS debugging. If you do not specify it, the location sets to hls/.debug.

```
syn.debug.directory=../../debug
```
Interface Configuration

The syn.interface commands configure the default interface settings applied to the HLS component. These settings can be
overridden for specific top-level interface ports using syn.directive.interface.

**syn.interface.clock_enable**
Add a clock-enable port (ap_ce) to the design. The clock enable prevents all clock operations when it is active-Low, and disables
all sequential operations. The default is false.

```
syn.interface.clock_enable=1
```
**syn.interface.default_slave_interface**
Specify the default slave interface. The two modes are s_axilite and none. The default is s_axilite.

```
syn.interface.default_slave_interface=none
```
**syn.interface.m_axi_addr64**
Enable 64-bit addressing for all m_axi interfaces. This option is enabled by default. When disabled, the m_axi interfaces uses 32-
bit addressing.

```
syn.interface.m_axi_addr64=1
```
**syn.interface.m_axi_alignment_byte_size**
Specify default alignment byte size for all m_axi interfaces. The default when this option is not specified is 64 bytes for Vitis kernel
flow. The default is 1 byte for the Vivado IP flow. See the Vivado/Vitis Flows section of the _Vitis High-Level Synthesis User Guide_
(UG1399).

```
syn.interface.m_axi_alignment_byte_size=16
```
```
Tip: A value of 0 is not valid.
```
**syn.interface.m_axi_auto_id_channel**
Enable automatic assignment of channel IDs for m_axi interfaces. By default, this option is disabled. Refer to the AXI4 Master
Interface section of the _Vitis High-Level Synthesis User Guide_ (UG1399) for additional information.

```
syn.interface.m_axi_auto_id_channel=1
```
**syn.interface.m_axi_auto_max_ports**
Enable automatic creation of separate m_axi interface adapters for each argument or port on the interface. This option is disabled
by default, reducing the m_axi interfaces to minimum requirements. Refer to the section M_AXI Bundles of the _Vitis High-Level
Synthesis User Guide_ (UG1399) or more information.

```
syn.interface.m_axi_auto_max_ports=1
```

#### Displayed in the footer

**syn.interface.m_axi_buffer_impl**
Specify the implementation resource for all buffers internal to the m_axi adapters. The choices are auto, lutram, bram, uram.
The default is bram.

```
syn.interface.m_axi_buffer_impl=lutram
```
**syn.interface.m_axi_cache_impl**
Specify the implementation resource for cache added to the m_axi adapters. The choices are auto, lutram, bram, uram. The
default is auto.

```
syn.interface.m_axi_cache_impl=lutram
```
**syn.interface.m_axi_conservative_mode**
Configure all m_axi adapters to work in conservative mode. The Vitis tool waits to issue a write request until the associated write
data is entirely available. Typically, the tool waits until the data is buffered into the adapter or already emitted. The tool uses a
buffer inside the M_AXI adapter to store all the data for a burst (both in case of reading and writing). This feature is enabled by
default, and might slightly increase write latency but can resolve deadlock due to concurrent requests (read or write) on the
memory subsystem. Disable conservative mode by setting it to false.

```
syn.interface.m_axi_conservative_mode=0
```
**syn.interface.m_axi_flush_mode**
Configure all m_axi adapters to be flushable, writing or reading garbage data. This option only applies when a burst is interrupted
due to pipeline blocking. For example, when missing data inputs when not in conservative mode or missing output space. By
default, this option is disabled.

```
syn.interface.m_axi_flush_mode=1
```
**syn.interface.m_axi_latency**
Specify the expected latency of the m_axi interface globally. Allows the design to initiate a bus request a number of cycles
(latency) before the read or write is expected. The default value is 64 for the Vitis Kernel flow. The default value is for the Vivado IP
flow. See the section Defaults of Vivado and Vitis Flows in the _Vitis High-Level Synthesis User Guide_ (UG1399).

```
syn.interface.m_axi_latency=5
```
**syn.interface.m_axi_max_bitwidth**
Specifies the maximum bitwidth for the m_axi interfaces data channel. The default is 1024 bits. The specified value must be a
power-of-two, between 8 and 1024. This decreases throughput if the actual accesses are bigger than the required interface. The
reason is due to the throughput being split into a multi-cycle burst of accesses.

```
syn.interface.m_axi_max_bitwidth=128
```
**syn.interface.m_axi_max_read_burst_length**
Specifies a global maximum number of data values read during a burst transfer for all m_axi interfaces. The default is 16.

```
syn.interface.m_axi_max_read_burst_length=12
```
**syn.interface.m_axi_max_widen_bitwidth**
Enable automatic port width resizing to widen bursts for the m_axi interface, up to the chosen bitwidth. The specified value must
be a power of 2 between 8 and 1024, and must align with the -m_axi_alignment_size. The default value is 512 for the Vitis
Kernel flow, and 0 for the Vivado IP flow.

```
syn.interface.m_axi_max_widen_bitwidth=64
```
**syn.interface.m_axi_max_write_burst_length**
Specifies a global maximum number of data values written during a burst transfer for all m_axi interfaces. The default is 16.

```
syn.interface.m_axi_max_write_burst_length=12
```

#### Displayed in the footer

**syn.interface.m_axi_min_bitwidth**
Specifies the minimum bitwidth for m_axi interfaces data channel. The default is 8 bits. The value must be a power of 2, between
8 and 1024. This does not necessarily increase throughput if the actual accesses are smaller than the required interface.

```
syn.interface.m_axi_min_bitwidth=64
```
**syn.interface.m_axi_num_read_outstanding**
Specifies how many read requests can be made to the m_axi interface without a response, before the design stalls. This implies
internal storage in the design, and a FIFO of size:

```
num_read_outstanding*max_read_burst_length*word_size
```
```
The default value is 16.
```
```
syn.interface.m_axi_num_read_outstanding=8
```
**syn.interface.m_axi_num_write_outstanding**
Specifies how many write requests can be made to the m_axi interface without a response, before the design stalls. This implies
internal storage in the design, and a FIFO of size:

```
num_write_outstanding*max_write_burst_length*word_size
```
```
The default value is 16.
```
```
syn.interface.m_axi_num_write_outstanding=8
```
**syn.interface.m_axi_offset**
Specify the default offset mechanism for all m_axi interfaces. The options are:
off: No offset port is generated.
slave: Generates an offset port and automatically maps it to an s_axilite interface. This option is the default value.
direct: Generates a scalar input offset port for directly passing the address offset into the IP through the offset port.

```
syn.interface.m_axi_offset=slave
```
**syn.interface.register_io**
Globally enables registers for all scalar inputs, outputs, or all scalar ports on the top function. Arrays are always registered. The
options are off, scalar_in, scalar_out, scalar_all. The default is off.

```
syn.interface.register_io=scalar_out
```
**syn.interface.s_axilite_auto_restart_counter**
Enables the auto-restart behavior for kernels. Use 1 to enable the auto-restart feature, or 0 to disable it which is the default. When
enabled, the tool establishes the auto-restart bit in the ap_ctrl_chain control protocol for the s_axilite interface. For more
information refer to Auto-Restarting Mode in the _Vitis High-Level Synthesis User Guide_ (UG1399).

```
syn.interface.s_axilite_auto_restart_counter=1
```
**syn.interface.s_axilite_data64**
Enable 64 bit data width for s_axilite interface. Use 1 to enable 64 bit data width and 0 to disable it. The default is 32 bit data
width.

```
syn.interface.s_axilite_data64=1
```
**syn.interface.s_axilite_interrupt_mode**
Specify the interrupt mode for s_axilite interface to be Clear on Read (cor) or Toggle on Write (tow). The Vitis tool can
complete clear on Read interrupts in a single transaction, while tow requires two. Tow is the default interrupt mode.

```
syn.interface.s_axilite_interrupt_mode=cor
```

#### Displayed in the footer

##### ★

**syn.interface.s_axilite_mailbox**
Enables the creation of a mailbox for non-stream non-stable s_axilite arguments. The mailbox feature sets and manages the
auto-restart kernels. See the Auto-Restarting Mode section in the _Vitis High-Level Synthesis User Guide_ (UG1399).
The available options are:
in: Enable mailbox for only input arguments
out: Enable mailbox for only output arguments
both: Enable mailbox for input and output arguments
none: No mailbox created. This option is the default value.

```
syn.interface.s_axilite_mailbox=in
```
**syn.interface.s_axilite_status_regs**
Enables exposure of ECC error bits in the s_axilite register file. Achieves exposure through two clear-on-read (COR) counters
per block RAM or URAM with ECC enabled. The options are ecc to enable or off to disable the feature. The default is set to
disabled.

```
syn.interface.s_axilite_status_regs=ecc
```
**syn.interface.s_axilite_sw_reset**
Enables SW reset in s_axilite adapter.

```
syn.interface.s_axilite_sw_reset=1
```
Package Options

### Output Options

The package.output options specifies the nature of the exported files that the synthesized RTL design generates. These options include
the following:

**package.output.file**
Specifies the output file path and name for the exported file. If you do not specify this option, the Vitis Unified IDE names the
output file after the HLS component. See the following for an example command:

```
package.output.file=../kernel.xo
```
**package.output.format**
Specifies the exported format of the output generated after synthesis. The supported values are:
package.output.format=ip_catalog: A format suitable for adding to the Vivado IP catalog.
package.output.format=xo: A format accepted by the v++ compiler for linking in the Vitis application acceleration flow.
package.output.format=syn_dcp: Synthesized checkpoint file for Vivado Design Suite. Using this option executes RTL
synthesis. Vivado implementation can be optionally added using vivado.flow=impl
package.output.format=sysgen: Generates an IP and .zip archive for use in System Generator.
package.output.format=rtl: Outputs the RTL files from C synthesis. Does not package the IP or XO used for
downstream processes.

```
Tip: The RTL format is useful for development, analysis, and debug of the HLS component. However, to export files you
must specify one of the other output formats.
```
**package.output.syn**
Enables the running of the Package step as part of the C synthesis step to generate the required export files during synthesis.

### IP Options

The package.ip commands specify details of the IP that the HLS component generates. These are settings to define the IP Vendor,
Library, Name, and Version (VLNV) for example to identify it in the IP catalog.


#### Displayed in the footer

**package.ip.description**
Provides a description for the catalog entry for the generated IP, used when packaging the IP.

```
package.ip.description=details of IP
```
**package.ip.display_name**
Provides a display name for the catalog entry for the generated IP, used when packaging the IP.

```
package.ip.display_name=randy1
```
**package.ip.library**
Provides the library component of the <Vendor>:<Library>:<Name>:<Version> (VLNV) identifier for generated IP.

```
package.ip.library=testLib
```
**package.ip.name**
Provides the name component of the <Vendor>:<Library>:<Name>:<Version> (VLNV) identifier for the generated IP.

```
package.ip.name=randy1
```
**package.ip.taxonomy**
Specifies the taxonomy for the catalog entry for the generated IP, used when packaging the IP. Taxonomy is a category to help
identify the purpose or application of the IP.

```
package.ip.taxonomy=video
```
**package.ip.vendor**
Provides the vendor component of the <Vendor>:<Library>:<Name>:<Version> (VLNV) identifier for generated IP.

```
package.ip.vendor=randyCom
```
**package.ip.version**
Provides the version component of the <Vendor>:<Library>:<Name>:<Version> (VLNV) identifier for generated IP.

```
package.ip.version=2.3
```
**package.ip.xdc_file**
Specifies an XDC file. The implementation in the Vivado tool uses the contents of the packaged IP.

```
package.ip.xdc_file=../../ip.xdc
```
**package.ip.xdc_ooc_file**
Specifies an out-of-context (OOC) XDC file. The contents of this file are included in the packaged IP and used during out-of-
context Vivado synthesis for the exported IP.

```
package.ip.xdc_ooc_file=../../ooc,xdc
```
Operator Configuration

The syn.op command lets you configure the default implementation style, latency, and precision for different operators used for the
HLS component. You can add multiple syn.op commands to a config file to specify the details of different operators. If an operator is
not specified, the Vitis tool determines the default values for the component.
You can override the default settings specified by the syn.op command by using the syn.directive.bind_op command for specific
variables.


#### Displayed in the footer

**syn.op**
The following is the syntax for the syn.op command.

```
syn.op=op:mul impl:dsp
syn.op=op:add impl:fabric latency:6
syn.op=op:fmacc precision:high
syn.op=op:hdiv latency:5
```

#### Displayed in the footer

##### ★

##### ★

syn.op=: Starts the command
op:<operator>: Specifies the op keyword followed by the operator being defined.
mul: integer multiplication operation
add: integer add operation
sub: integer subtraction operation
fadd: single precision floating-point add operation
fsub: single precision floating-point subtraction operation
fdiv: single precision floating-point divide operation
fexp: single precision floating-point exponential operation
flog: single precision floating-point logarithmic operation
fmul: single precision floating-point multiplication operation
frsqrt: single precision floating-point reciprocal square root operation
frecip: single precision floating-point reciprocal operation
fsqrt: single precision floating-point square root operation
dadd: double precision floating-point add operation
dsub: double precision floating-point subtraction operation
ddiv: double precision floating-point divide operation
dexp: double precision floating-point exponential operation
dlog: double precision floating-point logarithmic operation
dmul: double precision floating-point multiplication operation
drsqrt: double precision floating-point reciprocal square root operation
drecip: double precision floating-point reciprocal operation
dsqrt: double precision floating-point square root operation
hadd: half precision floating-point add operation
hsub: half precision floating-point subtraction operation
hdiv: half precision floating-point divide operation
hmul: half precision floating-point multiplication operation
hsqrt: half precision floating-point square root operation
facc: single precision floating-point accumulate operation
fmacc: single precision floating-point multiply-accumulate operation
fmadd: single precision floating-point multiply-add operation

**Tip:** Comparison operators, such as dcmp, implements in LUTs. The operators cannot implement outside of the fabric, or
map to DSPs. You cannot configure operators with the syn.op or syn.directive.bind_op commands.
impl:<value>: Specifies the implementation (impl) keyword followed by the value for the specified operator. When impl
is not specified, the default is for the tool to determine the best implementation for a given operator. Supported values
include:
all: All implementations. This is the default setting.
dsp: Use DSP resources
fabric: Use non-DSP resources
meddsp: Floating Point IP Medium Usage of DSP resources
fulldsp: Floating Point IP Full Usage of DSP resources
maxdsp: Floating Point IP Max Usage of DSP resources
primitivedsp: Floating Point IP Primitive Usage of DSP resources
auto: enable inference of combined facc | fmacc | fmadd operators
none: disable inference of combined facc | fmacc | fmadd operators
latency:<value>: Specifies the latency keyword followed by the value. Defines the default latency for the binding of the
operator to the implementation resource. The valid value range varies for each implementation (impl) of the operation. The
default is -1, which lets the tool apply the standard latency for the implementation resource.

**Tip:** You can specify the latency for a specific operation without specifying the implementation. This leaves the tool free to
choose the implementation while managing the latency.
precision:<value>: Used for floating point operators (facc, fmacc, and fmadd), this specifies the precision keyword
followed by one of the following:


#### Displayed in the footer

```
low: Use a low precision (60 bit and 100 bit integer) accumulation implementation when available. This option is only
available on certain non-AMD Versal™ devices, and can cause RTL/Co-Sim mismatches due to insufficient precision
with respect to C++ simulation. However, devices can be pipelined with an II=1 without source code changes. This
option uses approximately three times the resources of standard precision floating point accumulation.
high: Use high precision (one extra bit) fused multiply-add implementation when available. This option is useful for
high-precision applications. The options is very efficient on Versal devices, although it can cause RTL/Co-Sim
mismatches due to the extra precision with respect to C++ simulation. The option uses more resources than standard
precision floating point accumulation.
standard: standard precision floating point accumulation and multiply-add is suitable for most uses of floating-point,
and is the default setting. It always uses a true floating-point accumulator that can be pipelined with II=1 on Versal
devices, and II that is typically between 3 and 5 (depending on clock frequency and target device) on non-Versal
devices.
```
RTL Configuration

The syn.rtl commands configure various attributes of the compiled RTL, the type of reset used, and the encoding of the state
machines. The commands also allow you to use specific identification in the RTL. By default, these options are apply to the top-level
design and all RTL blocks within the design.

**syn.rtl.cosim_trace_generation**
Generates test vectors during hardware emulation in the Vitis tool flow. The option applies when the kernel synthesizes as a Vitis
kernel. The process is useful C/RTL Co-simulation test bench in future design iterations.

```
syn.rtl.cosim_trace_generation=1
```
**syn.rtl.deadlock_detection**
Enables simulation or synthesis deadlock detection in the top-level RTL of an exported IP/XO file. The options are as follows:
none: Disables deadlock detection.
sim: Enables deadlock detection only for simulation/emulation. This is the default setting.
hw: Enables detection in synthesized RTL IP. Adds ap_local_deadlock and ap_local_block signals to the IP to enable
local and global deadlock detection.

```
syn.rtl.deadlock_detection=hw
```
**syn.rtl.deadlock_diagnosis**
Enables deadlock detection diagnosis for Vitis kernels (.xo) during hardware emulation of the Application.

```
syn.rtl.deadlock_diagnosis=1
```
**syn.rtl.fsm_encoding**
Specifies the 'fsm_encoding' RTL attribute value to guide RTL synthesis. The options are as follows:
auto: Allow the RTL Synthesis tool to determine the best state machine encoding
gray: Uses gray state machine encoding.
johnson: Uses johnson state machine encoding.
one_hot: Uses one_hot state machine encoding.
sequential: Uses sequential state machine encoding.
none: Disables state machine encoding. The state machine is synthesized exactly using the state code specified in the RTL
(which is one_hot).

```
syn.rtl.fsm_encoding=gray
```

#### Displayed in the footer

###### !!

**syn.rtl.fsm_safe_state**
Specify the 'fsm_safe_state' RTL attribute value to guide RTL synthesis. This attribute can affect the quality of synthesis results,
typically resulting in less performance with greater area. The options are as follows:
auto_safe_state: Implies Hamming-3 encoding.
default_state: Return the state machine to the default state.
power_on_state: Return the state machine to the POWER_ON state.
reset_state: Return the state machine to the RESET state.
none: Attribute not added to RTL, the state machine will not include safe state logic.

```
syn.rtl.fsm_safe_state=auto_safe_state
```
**syn.rtl.header**
Specify a file whose contents will be inserted at the beginning of all generated RTL files. This allows you to ensure that the
generated RTL files contain user specified content.

```
syn.rtl.header=../../myHeader.txt
```
**syn.rtl.kernel_profile**
Add top level event and stall ports required by kernel profiling.

```
syn.rtl.kernel_profile=1
```
```
Important: This option relates to the v++ -c --profile.stall command. You must manually add this option to the HLS
component to ensure the stall profiling is available for the linked application.
```
**syn.rtl.module_auto_prefix**
Specifies the top level function name as the prefix value for generated RTL modules. This option is ignored if
syn.rtl.module_prefix is also specified. This option is enabled by default.

```
syn.rtl.module_auto_prefix=1
```
**syn.rtl.module_prefix**
Specifies a prefix to be used for all generated RTL module names. Use this to override the defaul module prefix of the top-level
function.

```
syn.rtl.module_prefix=newTop
```
**syn.rtl.mult_keep_attribute**
Enables keep attribute.

```
syn.rtl.mult_keep_attribute=1
```
**syn.rtl.register_all_io**
Uses a register by default for all I/O signals.

```
syn.rtl.register_all_io=1
```
**syn.rtl.register_reset_num**
Number of registers to add to reset signal.

```
syn.rtl.register_reset_num=2
```

#### Displayed in the footer

##### ★

###### ✎

**syn.rtl.reset**
Variables initialized in the C/C++ code are always initialized to the same value in the RTL and therefore in the bitstream. This
initialization is performed only at power-on. It is not repeated when a reset is applied to the design.
Apply this setting with the -reset option to determine how registers and memories are reset.
none: Does not add resets to the design.
control: Resets control registers, such as those used in state machines and those used to generate I/O protocol signals.
This is the default setting.
state: Resets control registers and registers or memories derived from static or global variables in the C/C++ code. Any
static or global variable initialized in the C/C++ code resets to its initialized value.
all: Resets all registers and memories in the design. Any static or global variable initialized in the C/C++ code is reset to its
initialized value.

```
syn.rtl.reset=state
```
**syn.rtl.reset_async**
Causes all registers to use an asynchronous reset. If you do not specify this option, a synchronous reset is used.

```
syn.rtl.reset_async=1
```
**syn.rtl.reset_level**
Defines the polarity of the reset signal to be either active-Low or active-High. The default setting is active-High.

```
syn.rtl.reset_level=low
```
```
Tip: The AXI protocol requires an active-Low reset. If your design uses AXI interfaces, the Vitis tool defines this reset level with
a warning if the syn.rtl.reset_level is active-High.
```
Schedule Setting

The syn.schedule command configures the default type of scheduling performed.

**syn.schedule.enable_dsp_full_reg**
Specifies that the DSP signals need to be fully registered. This option is enabled by default.

```
syn.schedule.enable_dsp_full_reg=0
```
Storage Configuration

Sets the global default options for the HLS micro-architecture binding of FIFO storage elements to memory resources.
You can override the default configuration defined by syn.storage for FIFO storage by syn.directive.bind_storage for a specific
design element. You can also override the default configuration by specifying the storage_type option for syn.directive.interface for
objects on the interface.

**syn.storage**
See the following for the syntax of the syn.storage command.

```
syn.storage=fifo impl=auto auto_srl_max_bits=512 auto_srl_max_depth=3
```
```
syn.storage=fifo: Starts the command to configure FIFOs.
Note: FIFO is the only type supported at this time.
impl=<value>: Specifies the implementation (impl) keyword followed by the value. When impl is not specified, the
default is auto. The default value allows the Vitis tool to determine the best implementation for a given operator. The option
supports auto, bram, lutram, uram, memory, and srl values.
auto_srl_max_bits=<value>: Only valid when for impl:auto (the default). Specifies the maximum allowed SRL total
bits (depth * width) for auto implementations. The default is 1024.
auto_srl_max_depth=<value>: Specifies the maximum allowed SRL depth for auto-srl implementation. The default is 2.
```

#### Displayed in the footer

##### ★

Implementation Configuration

The syn.vivado commands configure the Vivado synthesis and implementation runs used to derive resource utilization and timing
estimates. These settings generally do not affect the RTL created for the HLS component, but can affect the example hardware design
used for estimates.

**vivado.clock**
Override the HLS clock constraint used in the Vivado out-of-context run for determination of timing analysis. This does not affect
the output IP or XO files.

```
vivado.clock=37
```
**vivado.flow**
Run the Vivado flow for synthesis only (syn), or for implementation (impl). Running synthesis will be faster than running
implementation, but will lack some of details of the implementation run. The default setting is impl.

```
vivado.flow=syn
```
```
Tip: This is different from the flow_target option that defines the target as being either the Vitis kernel flow or the Vivado IP
flow.
```
**vivado.impl_strategy**
Specify a Vivado synthesis or implementation strategy. Strategy names can be found in Defining Implementation Strategies in the
_Vivado Design Suite User Guide: Implementation_ (UG904). This is used for resource usage and timing analysis and will not affect
the output files.

```
vivado.impl_strategy=Performance_Explore
```
**vivado.max_timing_paths**
Specify the max number of timing paths to report when the timing is not met in the Vivado synthesis or implementation.

```
vivado.max_timing_paths=12
```
**vivado.optimization_level**
Vivado optimization level, sets other vivado_* options. This will not apply to IP/XO output. The choices are 0, 1, 2, 3. This only
applies for report generation and will not apply to the exported IP. The default setting is 0.

```
vivado.optimization_level=2
```
**vivado.pblock**
Specify a Pblock range to use during implementation for reporting purposes.

```
vivado.pblock={SLICE_X8Y105:SLICE_X23Y149}
```
**vivado.phys_opt**
Run Vivado physical optimization at the specified implementation stage. The choices are no optimization (none), placement
optimization (place), routing optimization (route), or both placement and routing (all). This option only applies to the
implementation run used to generate resource and timing estimates.

```
vivado.phys_opt=route
```
**vivado.report_level**
Specify the report level for Vivado synthesis and implementation run. The valid values and the associated reports are:
0: Post-synthesis utilization. Post-implementation utilization and timing.
1: Post-synthesis utilization, timing, and analysis. Post-implementation utilization, timing, and analysis.
2: Post-synthesis utilization, timing, analysis, and failfast. Post-implementation utilization, timing, and failfast. This is the
default setting.

```
vivado.report_level=1
```

#### Displayed in the footer

###### !!

###### ✎

**vivado.rtl**
Specifies RTL language (verilog/vhdl) to select in Vivado flow.

```
vivado.rtl=vhdl
```
**vivado.synth_design_args**
Specifies arguments for the Vivado synth_design command. Available synth_design arguments can be found in the _Vivado
Design Suite Tcl Command Reference Guide_ (UG835).

```
vivado.synth_design_args=arg1 arg2
```
**vivado.synth_strategy**
Specifies a Vivado synthesis strategy to use when generating the example design to use for resource and timing estimates.
Specific synthesis strategies can be found in _Vivado Design Suite User Guide: Synthesis_ (UG901).

```
vivado.synth_strategy=Synthesis_Defaults
```
Unroll Setting

The syn.unroll setting provides a global tripcount threshold below which loops are automatically unrolled. Loops with a greater
tripcount can be unrolled using syn.directive.unroll.

**syn.unroll.tripcount_threshold**
All loops which have fewer iterations than the specified value are automatically unrolled. The default value is 0, which means that
loops are not automatically unrolled.

```
syn.unroll.tripcount_threshold=6
```
v++ Mode VSS

### Link

The v++ mode vss selects a link flow that creates a Vitis Subsystem (VSS) artifact. A VSS is a platform-independent, reusable design
component that you can customize with AI Engine and PL content. Using v++ --mode vss, you package your design elements (for
example, AI Engine graphs and PL kernels, and optionally PS domains) into a subsystem that is intended to be integrated into a larger
system using Vitis tools. This flow does not produce a deployable hardware bitstream; instead, it generates a subsystem package (with
metadata and binaries) that can be instantiated and linked within a system context on the target platform later.
--mode (-m):

```
-m [ --mode ] arg Specify a mode.
[aie|hls] Must be used with --compile/-c option.
[vss] Must be used with --link/-l option.
```
See Linking a VSS Component in the _Embedded Design Development Using Vitis_ (UG1701) for details.

HLS Optimization Directives

**Important:** The following options must appear in the HLS configuration file under the [hls] header.
Directives, or the syn.directive.xxx commands allow you to customize the synthesis results for the same source code across
multiple implementations. Change the directives to change the results. The syn.directive.xxx commands are for use in config files
with the Vitis Unified IDE. See Building and Running an HLS Component in the _Vitis High-Level Synthesis User Guide_ (UG1399).
**Note:** You can also use pragmas in your source code, rather than directives in your config file. Doing so has the advantage of the
pragmas being stored directly in your source code. Refer to the HLS Pragmas section of the _Vitis High-Level Synthesis User Guide_
(UG1399) for more information.
Directives applied through a configuration file must include a <location> as an argument to the directive. The <location> defines
what element of the source code the directive applies to, such as function, loop, region, or variable.
The example syntax for syn.directive.xxx commands include the location and the arguments for the directive. For example:


#### Displayed in the footer

##### ★

###### !!

```
syn.directive.pipeline=dct2d II=4
```
Where dct2d is the location (function name) to apply the PIPELINE directive to, and II=4 is one of the possible arguments to the
directive.

syn.directive.aggregate

### Description

This directive collects the data fields of a struct into a single wide scalar. Vitis HLS performs a similar operation as
syn.directive.array_reshape for arrays inside the struct. Vitis HLS completely partitions and reshapes the array into a wide
scalar and packs it with other elements of the struct.

**Tip:** Vitis HLS restructures arrays of structs as arrays of aggregated elements. The optimization does not support structs that
contain other structs.
You can infer the bit alignment of the resulting new wide-word from the declaration order of the struct elements. The first element takes
the least significant sector of the word and so forth until all fields are mapped.

### Syntax

```
syn.directive.aggregate=[OPTIONS] <location> <variable>
```
```
<location> is the location (in the format function[/label]) that contains the packed variable.
<variable> is the struct variable to be packed.
```
### Options

**compact=[bit | byte | none | auto]**
Specifies the alignment of the aggregated struct. Alignment can be on the bit-level (packed), the byte-level (padded), none, or
automatically determined by the tool which is the default behavior.

### Examples

The following example aggregates the variable AB. The variable is a struct pointer containing three 8-bit fields (typedef struct
{unsigned char R, G, B;}pixel) in function func. The fields are abggregated into a new 24-bit pointer, aligning data at the bit-
level.

```
syn.directive.aggregate=func AB compact=bit
```
### See Also

```
syn.directive.array_reshape
syn.directive.disaggregate
```
syn.directive.alias

### Description

Specifies that two or more M_AXI pointer arguments point to the same underlying buffer in memory (DDR or HBM). Indicates aliasing
between the pointers by setting the distance or offset between them.
**Important:** syn.directive.alias applies to top-level function arguments mapped to M_AXI interfaces.
Vitis HLS considers different pointers to be independent channels and generally does not provide any dependency analysis. However,
in cases where the host allocates a single buffer for multiple pointers, this relationship can be communicated through
syn.directive.alias. Communicating this way maintains dependency analysis. Communicating through syn.directive.alias
also enables data dependence analysis in Vitis HLS by defining the distance between pointers in the buffer.
Requirements for syn.directive.alias:


#### Displayed in the footer

```
You must assign all ports to M_AXI interfaces and also assign them to different bundles. See example below for details.
You can only use each port one time in a syn.directive.alias command.
All ports assigned to syn.directive.alias must have the same depth.
Each port can have one offset if you specify the offset.
You must specify the offset for the M_AXI port as slave or direct. offset=off is not supported.
```
### Syntax

```
syn.directive.alias=[OPTIONS] <location> <ports>
```
```
<location> is the location string in the format function[/label] that the ALIAS pragma applies to.
<ports> specifies the ports to alias.
```
### Options

**distance=<integer>**
Specifies the difference between the pointer values passed to the ports in the list.

**offset=<string>**
Specifies the offset of the pointer passed to each port in the ports list with respect to the origin of the array.

### Example

For the following function top:

```
void top(int *arr0, int *arr1, int *arr2, int *arr3, ...) {
#pragma HLS interface M_AXI port=arr0 bundle=hbm0 depth=0x40000000
#pragma HLS interface M_AXI port=arr1 bundle=hbm1 depth=0x40000000
#pragma HLS interface M_AXI port=arr2 bundle=hbm2 depth=0x40000000
#pragma HLS interface M_AXI port=arr3 bundle=hbm3 depth=0x40000000
```
The following command defines aliasing for the specified array pointers, and defines the distance between them:

```
syn.directive.alias=top arr0,arr1,arr2,arr3 distance=10000000
```
Alternatively, the following command specifies the offset between pointers, to accomplish the same effect:

```
syn.directive.alias=top arr0,arr1,arr2,arr3 offset=00000000,10000000,20000000,30000000
```
### See Also

```
syn.directive.interface
```
syn.directive.allocation

### Description

Specifies instance restrictions for resource allocation.
syn.directive.allocation can limit the number of RTL instances and hardware resources used to implement specific functions,
loops, or operations. If the C/C++ source has four instances of a function foo_sub, syn.directive.allocation ensures there is
only one instance of foo_sub in the final RTL. All four instances are implemented using the same RTL block. This reduces resources
used by the function, but negatively impacts performance by sharing those resources.
The syn.directive.allocation command also limits operations in the C/C++ code (additions, multiplications, array reads, and
writes).


#### Displayed in the footer

###### !!

### Syntax

```
syn.directive.allocation=[OPTIONS] <location> <instances>
```
```
<location> is the location string in the format function[/label].
<instances> is a function or operator. The function can be any function in the original C/C++ code that has not been either
inlined by the syn.directive.allocation command or inlined automatically by Vitis HLS.
```
### Options

**limit=<integer>**
Sets a maximum limit on the number of instances (of the type defined by the type option) to be used in the RTL design.

**type=[function|operation]**
The instance type can be function (default) or operation. For a complete list of supported operations refer to Operator
Configuration.

### Examples

Given a design foo_top with multiple instances of function foo, limits the number of instances of foo in the RTL to 2.

```
syn.directive.allocation=limit=2 type=function foo_top foo
```
Limits the number of multipliers used in the implementation of My_func to 1. This limit does not apply to any multipliers that can reside
in sub-functions of My_func. To limit the multipliers used in the implementation of any sub-functions, specify an allocation directive on
the sub-functions or inline the sub-function into function My_func.

```
syn.directive.allocation=limit=1 type=operation My_func mul
```
### See Also

```
syn.directive.inline
```
syn.directive.array_partition

### Description

**Important:** syn.directive.array_partition and syn.directive.array_reshape are not supported for M_AXI Interfaces
on the top-level function. Instead, you can use the hls::vector data types as described in the Vector Data Types section of the _Vitis
High-Level Synthesis User Guide_ (UG1399).
syn.directive.array_partition partitions an array into smaller arrays or individual elements.
This partitioning can do the following.

```
Results in RTL with multiple small memories or multiple registers instead of one large memory
Increases the number of read and write ports for the storage
Capacity to improve the throughput of the design
Requires more memory instances or registers
```
### Syntax

```
syn.directive.array_partition=[OPTIONS] <location> <array>
```
```
<location> is the location (in the format function[/label]) that contains the array variable.
<array> is the array variable to be partitioned.
```

#### Displayed in the footer

### Options

**dim=<integer>**
Specifies the dimension of the array for partitioning.
The dimension is relevant for multi-dimensional arrays only.
All dimensions are partitioned with the specified options if using a value of 0.
Any other value partitions only that dimension. For example, for a value of 1, only the first dimension is partitioned.

**type=(block|cyclic|complete)**
block partitioning creates smaller arrays from consecutive blocks of the original array. This effectively splits the array into _N_
equal blocks where _N_ is the integer defined by the -factor option.
cyclic partitioning creates smaller arrays by interleaving elements from the original array. For example, if -factor 3 is
used, the following is the result.
The directive assigns element 0 to the first new array.
Element 1 to the second new array.
Element 2 to the third new array.
Element 3 to the first new array again.
The complete partitioning decomposes the array into individual elements. For a one-dimensional array, this corresponds to
resolving a memory into individual registers. For multi-dimensional arrays, specify the partitioning of each dimension, or use
-dim 0 to partition all dimensions.
The default is complete.

**factor=<integer>**
Used for block or cyclic partitioning only, this option specifies the number of smaller arrays that are to be created.

**off=true**
Disables the ARRAY_PARTITION feature for the specified variable. Does not work with dim, factor, or type.

### Example 1

Partitions array AB[13] in function func into four arrays. Because four is not an integer factor of 13:

```
Three arrays have three elements.
One array has four elements (AB[9:12]).
```
```
syn.directive.array_partition=func AB type=block factor=4
```
Partitions array AB[6][4] in function func into two arrays, each of dimension [6][2].

```
syn.directive.array_partition=func AB type=block factor=2 dim=2
```
Partitions all dimensions of AB[4][10][6] in function func into individual elements.

```
syn.directive.array_partition=func AB type=complete dim=0
```
### Example 2

You can address partitioned arrays in your code by the new structure of the partitioned array. See the following code example when
using the following directive.

```
syn.directive.array_partition=top b type=complete dim=1
```
The code can be structured as follows:

```
struct SS
{
int x[N];
int y[N];
};
```

#### Displayed in the footer

###### !!

```
int top(SS *a, int b[4][6], SS &c) {...}
```
```
syn.directive.interface mode=ap_memory top b[0]
syn.directive.interface mode=ap_memory top b[1]
syn.directive.interface mode=ap_memory top b[2]
syn.directive.interface mode=ap_memory top b[3]
```
### See Also

```
syn.directive.array_reshape
```
syn.directive.array_reshape

### Description

**Important:** syn.directive.array_partition and syn.directive.array_reshape are not supported for M_AXI Interfaces
on the top-level function. Instead you can use the hls::vector data types as described in Vector Data Types in _Vitis High-Level
Synthesis User Guide_ (UG1399).
syn.directive.array_reshape combines array partitioning with vertical array mapping to create a single new array with fewer
elements but wider words.
The syn.directive.array_reshape command has the following features:

```
Splits the array into multiple arrays (like syn.directive.array_partition).
Automatically recombine the arrays vertically to create a new array with wider words.
```
### Syntax

```
syn.directive.array_reshape=[OPTIONS] <location> <array>
```
```
<location> is the location (in the format function[/label]) that contains the array variable.
<array> is the array variable to be reshaped.
```
### Options

**dim=<integer>**
Specifies the dimension of the array for partitioning.
The dimension is relevant for multidimensional arrays only.
All dimensions are partitioned with the specified options if using a value of 0.
Any other value partitions only that dimension. For example, if using a value of 1, only the first dimension is partitioned.

**type=(block|cyclic|complete)**
block reshaping creates smaller arrays from consecutive blocks of the original array. This effectively splits the array into _N_
equal blocks where _N_ is the integer defined by the -factor option and then combines the _N_ blocks into a single array with
word-width*N. The default is complete.
cyclic reshaping creates smaller arrays by interleaving elements from the original array. For example, if -factor 3 is
used, element 0 is assigned to the first new array, element 1 to the second new array, element 2 is assigned to the third new
array, and then element 3 is assigned to the first new array again. The final array is a vertical concatenation (word
concatenation, to create longer words) of the new arrays into a single array.
complete reshaping decomposes the array into temporary individual elements and then recombines them into an array with
a wider word. For a one-dimension array this is equivalent to creating a very-wide register. For example, if the original array
was _N_ elements of _M_ bits, the result is a register with N*M bits. This option is the default.

**factor=<integer>**
Used for block or cyclic partitioning only, this option specifies the number of smaller arrays that are to be created.


#### Displayed in the footer

###### ✎

##### ★

**object
Note:** Relevant for container arrays only.
Applies reshape on the objects within the container. If the option is specified, all dimensions of the objects are reshaped. However,
all dimensions of the container are maintained.

**off=true**
Disables the ARRAY_RESHAPE feature for the specified variable.

### Example 1

Reshapes 8-bit array AB[17] in function func into a new 32-bit array with five elements.
Because four is not an integer factor of 17:

```
Index 17 of the array, AB[17], is in the lower eight bits of the reshaped fifth element.
The upper eight bits of the fifth element are unused.
```
```
syn.directive.array_reshape=type=block factor=4 func AB
```
Partitions array AB[6][4] in function func, into a new array of dimension [6][2], in which dimension 2 is twice the width.

```
syn.directive.array_reshape=type=block factor=2 dim=2 func AB
```
Reshapes 8-bit array AB[4][2][2] in function func into a new single element array (a register), 4*2*2*8 (= 128)-bits wide.

```
syn.directive.array_reshape=type=complete dim=0 func AB
```
### Example 2

You can address reshaped arrays in your code by the new structure of the array. See the code below when using the following directive.

```
syn.directive.array_reshape=top b type=complete dim=0
```
The code can be structured as follows:

```
struct SS
{
int x[N];
int y[N];
};
```
```
int top(SS *a, int b[4][6], SS &c) {...}
```
And the array interface defined as:

```
syn.directive.interface=mode=ap_memory top b[0]
```
### See Also

```
syn.directive.array_partition
syn.directive.interface
```
syn.directive.bind_op

### Description

**Tip:** This pragma or directive does not play a part in DSP multi-operation matching (MULADD, AMA, ADDMUL and corresponding
Subtraction operations). In fact the use of BIND_OP for assigning an operator to DSP can prevent the HLS compiler from matching
multi-operation expressions.


#### Displayed in the footer

###### !!

##### ★

###### ✎

##### ★

Vitis HLS implements the different operations in the code using specific hardware resources (or implementations). The tool
automatically determines the resource to use. You can also define syn.op to specify the implementation (see Operator Configuration).
The syn.directive.bind_op command indicates the variable an operation to map to a specific implementation in the RTL. For
example mul, add, or sub can map to impl in the RTL. For example, to indicate that a specific multiplier operation (mul) is
implemented in the device fabric rather than a DSP, you can use the syn.directive.bind_op command.
You can also specify the latency with syn.directive.bind_op using the latency option as described below.
**Important:** To use the latency option, the operation must have an available multi-stage implementation. The HLS tool provides a
multi-stage implementation for all basic arithmetic operations (add, subtract, multiply, and divide), and all floating-point operations.

### Syntax

```
syn.directive.bind_op=[OPTIONS] <location> <variable>
```
```
<location> is the location (in the format function[/label]) which contains the variable.
<variable> is the variable to be assigned. The variable in this case is one that is assigned the result of the operation that is the
target of this directive.
```
### Options

**op=<value>**
Defines the operation to bind to a specific implementation resource. Supported functional operations include: mul, add, sub
Supported floating point operations include: fadd, fsub, fdiv, fexp, flog, fmul, frsqrt, frecip, fsqrt, dadd, dsub,
ddiv, dexp, dlog, dmul, drsqrt, drecip, dsqrt, hadd, hsub, hdiv, hmul, and hsqrt

```
Tip: Floating-point operations include single precision (f), double-precision (d), and half-precision (h).
```
**impl=<value>**
Defines the implementation to use for the specified operation.
Supported implementations for functional operations include fabric and dsp.
Supported implementations for floating point operations include: fabric, meddsp, fulldsp, maxdsp, and primitivedsp.
**Note:** primitivedsp is only available on Versal devices.

**latency=<int>**
Defines the default latency for the implementation of the operation. The valid latency varies according to the specified op and
impl. The default is -1, which lets Vitis HLS choose the latency.
The tables below reflect the supported combinations of operation, implementation, and latency.

**Table: Supported Combinations of Functional Operations, Implementation, and Latency**

```
Operation Implementation Min Latency Max Latency
```
```
add fabric 0 4
```
```
add dsp 0 4
```
```
mul fabric 0 4
```
```
mul dsp 0 4
```
```
sub fabric 0 4
```
```
sub dsp 0 0
```
**Tip:** Comparison operators, such as dcmp, are implemented in LUTs and cannot be implemented outside of the fabric, or mapped to
DSPs, and so are not configurable with the config_op or bind_op commands.

**Table: Supported Combinations of Floating Point Operations, Implementation, and Latency**

```
Operation Implementation Min Latency Max Latency
```

#### Displayed in the footer

**Operation Implementation Min Latency Max Latency**

```
fadd fabric 0 13
```
```
fadd fulldsp 0 12
```
```
fadd primitivedsp 0 3
```
```
fsub fabric 0 13
```
```
fsub fulldsp 0 12
```
```
fsub primitivedsp 0 3
```
```
fdiv fabric 0 29
```
```
fexp fabric 0 24
```
```
fexp meddsp 0 21
```
```
fexp fulldsp 0 30
```
```
flog fabric 0 24
```
```
flog meddsp 0 23
```
```
flog fulldsp 0 29
```
```
fmul fabric 0 9
```
```
fmul meddsp 0 9
```
```
fmul fulldsp 0 9
```
```
fmul maxdsp 0 7
```
```
fmul primitivedsp 0 4
```
```
fsqrt fabric 0 29
```
```
frsqrt fabric 0 38
```
```
frsqrt fulldsp 0 33
```
```
frecip fabric 0 37
```
```
frecip fulldsp 0 30
```
```
dadd fabric 0 13
```
```
dadd fulldsp 0 15
```
```
dsub fabric 0 13
```
```
dsub fulldsp 0 15
```
```
ddiv fabric 0 58
```
```
dexp fabric 0 40
```
```
dexp meddsp 0 45
```
```
dexp fulldsp 0 57
```
```
dlog fabric 0 38
```
```
dlog meddsp 0 49
```
```
dlog fulldsp 0 65
```

#### Displayed in the footer

##### ★

```
Operation Implementation Min Latency Max Latency
```
```
dmul fabric 0 10
```
```
dmul meddsp 0 13
```
```
dmul fulldsp 0 13
```
```
dmul maxdsp 0 14
```
```
dsqrt fabric 0 58
```
```
drsqrt fulldsp 0 111
```
```
drecip fulldsp 0 36
```
```
hadd fabric 0 9
```
```
hadd meddsp 0 12
```
```
hadd fulldsp 0 12
```
```
hsub fabric 0 9
```
```
hsub meddsp 0 12
```
```
hsub fulldsp 0 12
```
```
hdiv fabric 0 16
```
```
hmul fabric 0 7
```
```
hmul fulldsp 0 7
```
```
hmul maxdsp 0 9
```
```
hsqrt fabric 0 16
```
### Examples

In the following example, a two-stage pipelined multiplier using fabric logic is specified to implement the multiplication for variable c of
the function foo.

```
int foo (int a, int b) {
int c, d;
c = a*b;
d = a*c;
return d;
}
```
And the syn.directive.bind_op command is as follows:

```
syn.directive.bind_op=op=mul impl=fabric latency=2 foo c
```
**Tip:** The HLS tool selects the implementation to use for variable d because no syn.directive.bind_op is specified for that
variable.

### See Also

```
syn.directive.bind_storage
```

#### Displayed in the footer

###### !!

###### !!

##### ★

syn.directive.bind_storage

### Description

Vitis HLS assigns arrays in the code using specific memory resources (or types). The tool automatically determines the resource to use.
Alternatively, you can define syn.storage to generally specify the memory type, as explained in Storage Configuration.
The syn.directive.bind_storage command assigns a specific variable in the code (an array, or function argument) to a specific
memory type (type) in the RTL. For example, you can use the syn.directive.bind_storage command to specify which type of
memory, and which implementation to use for an array variable. You can control whether the array implements as a single or a dual-port
RAM.
**Important:** This feature is important for arrays on the top-level function interface. The memory type associated with the array
determines the number and type of ports needed in the RTL. See the Arrays on the Interface section of the _Vitis High-Level Synthesis
User Guide_ (UG1399). For variables assigned to top-level function arguments, you must assign the memory type and implementation
using the storage_type and storage_impl options of syn.directive.interface.
You can also use the latency option of syn.directive.bind_storage to specify the latency of the implementation. For block
RAMs on the interface, the latency option allows you to model off-chip, non-standard SRAMs at the interface. An example is
modeling support for an SRAM with a latency of 2 or 3. For internal operations, the latency option allows the operation
implementation using more pipelined stages. These additional pipeline stages can help resolve timing issues during RTL synthesis.
**Important:** To use the latency option, the memory must have an available multi-stage implementation. The HLS tool provides a
multi-stage implementation for all block RAMs.

### Syntax

```
syn.directive.bind_storage=[OPTIONS] <location> <variable>
```
```
<location> is the location (in the format function[/label]) which contains the variable.
<variable> is the variable to be assigned.
```
```
Tip: If the variable is an argument of a top-level function, then use the storage_type and storage_impl options of
syn.directive.interface.
```
### Options

**type=<value>**
Defines the type of memory to bind to the specified variable.


#### Displayed in the footer

##### ★

```
Supported types include: fifo, ram_1p, ram_1wnr, ram_2p, ram_s2p, ram_t2p, rom_1p, rom_2p, and rom_np.
```
```
Table: Storage Types
Type Description
```
```
FIFO A FIFO. Vitis HLS determines how to implement this in the RTL, unless the -impl option
is specified.
```
```
RAM_1P A single-port RAM. Vitis HLS determines how to implement this in the RTL, unless the -
impl option is specified.
```
```
RAM_1WNR A RAM with 1 write port and N read ports, using N banks internally.
```
```
RAM_2P A dual-port RAM that allows read operations on one port and both read and write
operations on the other port.
```
```
RAM_S2P A dual-port RAM that allows read operations on one port and write operations on the other
port.
```
```
RAM_T2P A true dual-port RAM with support for both read and write on both ports.
```
```
ROM_1P A single-port ROM. Vitis HLS determines how to implement this in the RTL, unless the -
impl option is specified.
```
```
ROM_2P A dual-port ROM.
```
```
ROM_NP A multi-port ROM.
```
```
Tip: If you specify a single port RAM for a PIPO, then the binder will typically allocate a merged-PIPO, with only one bank, and
reader and writer accessing different halves of the bank. The info message reported by the HLS compiler says "Implementing
PIPO using a single memory for all blocks." This is often cheaper for small PIPOs, where all banks can share a single RAM block.
However, if you specify a dual-port RAM for a PIPO to get higher bandwidth and the scheduler uses both ports either in the
producer or in the consumer, then the binder will typically allocate a split-PIPO, where the producer will use 1 port and the
consumer 2 ports of different banks. The info message reported by the HLS compiler in this case says "Implementing PIPO using
a separate memory for each block."
```
**impl=<value>**
Defines the implementation for the specified memory type. Supported implementations include: bram, bram_ecc, lutram, uram,
uram_ecc, srl, memory, and auto as described below.

```
Table: Supported Implementation
```
```
Name Description
```
```
MEMORY Generic memory for FIFO, lets the Vivado tool choose the implementation.
```
```
URAM UltraRAM resource
```
```
URAM_ECC UltraRAM with ECC
```
```
SRL Shift Register Logic resource
```
```
LUTRAM Distributed RAM resource
```
```
BRAM Block RAM resource
```
```
BRAM_ECC Block RAM with ECC
```
```
AUTO Vitis HLS automatically determine the implementation of the variable.
```

#### Displayed in the footer

```
Table: Supported Implementations by FIFO/RAM/ROM
```
```
Type Command/Pragma Scope Supported Implementations
```
```
FIFO bind_storage 1 local AUTO , BRAM, LUTRAM,
URAM, MEMORY, SRL
```
```
FIFO config_storage global AUTO , BRAM, LUTRAM,
URAM, MEMORY, SRL
```
```
RAM* | ROM* bind_storage local AUTO BRAM, BRAM_ECC,
LUTRAM, URAM,
URAM_ECC
```
```
RAM* | ROM* config_storage 2 global N/A
```
```
RAM_1P set_directive_interface
s_axilite -
storage_impl
```
```
local AUTO , BRAM, URAM
```
```
config_interface -
m_axi_buffer_impl
```
```
global AUTO, BRAM , LUTRAM,
URAM
```
1. When there is no implementation, the directive uses AUTOSRL behavior as a default. However, this value cannot be
    specified.
2. config_storage only supports FIFO types.

**latency=<int>**
Defines the default latency for the binding of the storage type to the implementation. The valid latency varies according to the
specified type and impl. The default is -1, which lets Vitis HLS choose the latency.

**Table: Supported Combinations of Memory Type, Implementation, and Latency**

```
Type Implementation Min Latency Max Latency
```
```
FIFO BRAM 1 4
```
```
FIFO LUTRAM 1 4
```
```
FIFO MEMORY 1 4
```
```
FIFO SRL 1 4
```
```
FIFO URAM 1 4
```
```
RAM_1P AUTO 1 3
```
```
RAM_1P BRAM 1 3
```
```
RAM_1P LUTRAM 1 3
```
```
RAM_1P URAM 1 3
```
```
RAM_1WNR AUTO 1 3
```
```
RAM_1WNR BRAM 1 3
```
```
RAM_1WNR LUTRAM 1 3
```
```
RAM_1WNR URAM 1 3
```
```
RAM_2P AUTO 1 3
```
```
RAM_2P BRAM 1 3
```
```
RAM_2P LUTRAM 1 3
```

#### Displayed in the footer

###### !!

##### ★

```
Type Implementation Min Latency Max Latency
```
```
RAM_2P URAM 1 3
```
```
RAM_S2P BRAM 1 3
```
```
RAM_S2P BRAM_ECC 1 3
```
```
RAM_S2P LUTRAM 1 3
```
```
RAM_S2P URAM 1 3
```
```
RAM_S2P URAM_ECC 1 3
```
```
RAM_T2P BRAM 1 3
```
```
RAM_T2P URAM 1 3
```
```
ROM_1P AUTO 1 3
```
```
ROM_1P BRAM 1 3
```
```
ROM_1P LUTRAM 1 3
```
```
ROM_2P AUTO 1 3
```
```
ROM_2P BRAM 1 3
```
```
ROM_2P LUTRAM 1 3
```
```
ROM_NP BRAM 1 3
```
```
ROM_NP LUTRAM 1 3
Important: syn.directive.bind_storage only support combinations of memory types and implementations in the prior table.
```
### Examples

In the following example, the coeffs[128] variable is an argument of the function func1. The directive specifies that coeffs uses a
single port RAM implemented on a BRAM core from the library.

```
syn.directive.bind_storage=func1 coeffs type=RAM_1P impl=bram
```
```
Tip: The ports created in the RTL to access the values of coeffs are defined in the RAM_1P core.
```
### See Also

```
syn.directive.bind_op
```
syn.directive.dataflow

### Description

This directive performs all operations sequentially in a C/C++ description. In the absence of any directives that limit resources (such as
set_directive_allocation), Vitis HLS seeks to minimize latency and improve concurrency. Data dependencies can limit this. For
example, functions or loops that access arrays must finish all read/write accesses to the arrays before they complete. This prevents the
next function or loop that consumes the data from starting operation.
However, it is possible for the operations in a function or loop to start operation before the previous function or loop completes all its
operations. syn.directive.dataflow specifies that dataflow optimization be performed on the functions or loops, improving the
concurrency of the RTL implementation. When syn.directive.dataflow is specified, the HLS tool analyzes the dataflow between
sequential functions or loops and creates channels. The channels are based on ping-pong RAMs or FIFOs that allow consumer
functions or loops to start operation before the producer functions or loops complete. Functions or loops can operate in parallel,
decreasing latency and improving the throughput of the RTL.


#### Displayed in the footer

##### ★

###### !!

**Tip:** The syn.dataflow.xxx command specifies the default memory channel and FIFO depth used by
syn.directive.dataflow as explained in Dataflow Configuration.
If no initiation interval is specified, Vitis HLS attempts to minimize the initiation interval and start operation as soon as data is available.
The initiation interval is number of cycles between the start of one function or loop and the next. For the DATAFLOW optimization to
work, the data must flow through the design from one task to the next. The following coding styles prevent the HLS tool from performing
the DATAFLOW optimization.

```
Single-producer-consumer violations
Feedback between tasks
Conditional execution of tasks
Loops with multiple exit conditions
```
**Important:** If any of these coding styles are present, the HLS tool issues a message and does not perform DATAFLOW optimization.
Finally, the DATAFLOW optimization has no hierarchical implementation. If a sub-function or loop contains additional tasks that might
benefit from the DATAFLOW optimization, apply optimization to the loop and the sub-function. You can also inline the sub-function.

### Syntax

```
syn.directive.dataflow=<location> disable_start_propagation
```
```
<location> is the location (in the format function[/label]) at which dataflow optimization is to be performed.
disable_start_propagation disables the creation of a start FIFO used to propagate a start token to an internal process.
Such FIFOs can sometimes be a bottleneck for performance.
```
### Examples

Specifies DATAFLOW optimization within function foo.

```
syn.directive.dataflow=foo
```
### See Also

```
syn.directive.allocation
```
syn.directive.dependence

### Description

Vitis HLS detects dependencies within loops. Dependencies within the same iteration of a loop are loop-independent dependencies.
Dependencies between different iterations of a loop are loop-carried dependencies.
There is impact on the dependencies when you schedule operations (especially during function and loop pipelining).

**Loop-independent dependence**
Accesses the same element in a single loop iteration.

```
for (i=1;i<N;i++) {
A[i]=x;
y=A[i];
}
```
**Loop-carried dependence**
Accesses the same element from a different loop iteration.

```
for (i=1;i<N;i++) {
A[i]=A[i-1]*2;
}
```

#### Displayed in the footer

###### !!

###### !!

###### ✎

###### ✎

Under certain circumstances, the dependence analysis can be too conservative and fail to filter out false dependencies. An example of
this circumstance is during variable dependent array indexing. Another example is when an external requirement needs to be enforced
(for example, two inputs do not share the same index). The syn.directive.dependence command allows you to explicitly define
the dependencies and eliminate a false dependence.
**Important:** Specifying a false dependency when the dependency is not false can result in incorrect hardware. Ensure dependencies
are correct (true or false) before specifying them.

### Syntax

```
syn.directive.dependence=[OPTIONS] <location>
```
**<location>**
The location in the code, specified as function[/label], where the dependence is defined.

### Options

**class=(array | pointer)**
Specifies a class of variables in which the dependence needs clarification. This is mutually exclusive with the variable option.

**variable=<variable>**
Defines a specific variable to apply the dependence directive. Mutually exclusive with the class option.
**Important:** You cannot specify a dependence for function arguments that are bundled with other arguments in an m_axi
interface. This is the default configuration for m_axi interfaces on the function. You also cannot specify a dependence for an
element of a struct, unless the struct has been disaggregated.

**dependent=(true | false)**
This argument should be specified to indicate whether a dependence is true and needs to be enforced, or is false and should
be removed. However, when not specified, the tool will return a warning that the value was not specified and will assume a value
of false.

**direction=(RAW | WAR | WAW)
Note:** Relevant only for loop-carried dependencies.
Specifies the direction for a dependence:
**RAW (Read-After-Write - true dependence)**
The write instruction uses a value used by the read instruction.
**WAR (Write-After-Read - anti dependence)**
The read instruction gets a value that is overwritten by the write instruction.
**WAW (Write-After-Write - output dependence)**
Two write instructions write to the same location, in a certain order.

**distance=<integer>
Note:** Relevant only for loop-carried dependencies where -dependent is set to true.
Specifies the inter-iteration distance for array access.

**type=(intra | inter)**
Specifies whether the dependence is:
Within the same loop iteration (intra), or
Between different loop iterations (inter) (default).

### Examples

Removes the dependence between Var1 in the same iterations of loop_1 in function func.

```
syn.directive.dependence=variable=Var1 type=intra \
dependent=false func/loop_1
```
The dependence on all arrays in loop_2 of function func informs Vitis HLS that all reads must happen _after_ writes in the same loop
iteration.


#### Displayed in the footer

###### !!

```
syn.directive.dependence=class=array type=intra \
dependent=true direction=RAW func/loop_2
```
### See Also

```
syn.directive.disaggregate
syn.directive.pipeline
```
syn.directive.disaggregate

### Description

The syn.directive.disaggregate command lets you deconstruct a struct variable into its individual elements. The contents of
the struct itself determine the number and type of elements.
**Important:** Structs used as arguments to the top-level function are aggregated by default, but can be disaggregated with this
directive or pragma.

### Syntax

```
syn.directive.disaggregate=<location> <variable>
```
```
<location> is the location (in the format function[/label]) where the variable to disaggregate is found.
<variable> specifies the struct variable name.
```
### Options

This command has no options.

### Example 1

The following example shows disaggregates the struct variable a in function top:

```
syn.directive.disaggregate=top a
```
### Example 2

You can address disaggregated structs by the using standard C/C++ coding style. The following shows the code when using the
directives.

```
syn.directive.disaggregate=top a
syn.directive.disaggregate=top c
```
The code can be structured as follows.

```
struct SS
{
int x[N];
int y[N];
};
```
```
int top(SS *a, int b[4][6], SS &c) {
```
```
syn.directive.interface=mode=s_axilite top a->x
syn.directive.interface=mode=s_axilite top a->y
```

#### Displayed in the footer

###### !!

```
syn.directive.interface=mode=ap_memory top c.x
syn.directive.interface=mode=ap_memory top c.y
```
```
Important: Notice the different methods shown above for accessing an element of a pointer (a) versus an element of a reference (c)
```
### Example 3

The following example shows the Dot struct containing the RGB struct as an element. If you apply syn.directive.disaggregate
to variable Arr, then only the top-level Dot struct is disaggregated.

```
struct Pixel {
char R;
char G;
char B;
};
```
```
struct Dot {
Pixel RGB;
unsigned Size;
};
```
```
#define N 1086
void DUT(Dot Arr[N]) {
...
}
```
```
syn.directive.disaggregate=DUT Arr
```
If you want to disaggregate the whole struct, Dot and RGB, then you can assign the set_directive_disaggregate as shown
below.

```
void DUT(Dot Arr[N]) {
#pragma HLS disaggregate variable=Arr->RGB
...
}
```
```
syn.directive.disaggregate=DUT Arr->RGB
```
The following code illustrates the results.

```
void DUT(char Arr_RGB_R[N], char Arr_RGB_G[N], char Arr_RGB_B[N], unsigned Arr_Size[N]) {
...
}
```
### See Also

```
syn.directive.aggregate
```
syn.directive.expression_balance

### Description

Sometimes C/C++ code has a sequence of operations, resulting in a long chain of operations in RTL. With a small clock period, this can
increase the latency in the design. By default, the Vitis HLS tool rearranges the operations using associative and commutative
properties. The rearrangement creates a balanced tree that can shorten the chain, potentially reducing latency in the design at the cost
of extra hardware.
Expression balancing rearranges operators to construct a balanced tree and reduce latency.


#### Displayed in the footer

##### ★

```
For integer operations expression balancing is on by default but can be disabled.
For floating-point operations, expression balancing is off by default but can be enabled.
```
The syn.directive.expression_balance command allows this expression balancing to be turned off, or on, within a specified
scope.

### Syntax

```
syn.directive.expression_balance=[OPTIONS] <location>
```
```
<location> is the location (in the format function[/label]). It specifies where expression balancing should be disabled or
enabled.
```
### Options

**off**
Turns off expression balancing at the specified location.
Specifying the syn.directive.expression_balance command enables expression balancing in the specified scope. Adding
the off option disables it.

### Examples

Disables expression balancing within function My_Func.

```
syn.directive.expression_balance=off My_Func
```
Explicitly enables expression balancing in function My_Func2.

```
set_directive_expression_balance=My_Func2
```
syn.directive.function_instantiate

### Description

By default:

```
Functions remain as separate hierarchy blocks in the RTL, or are decomposed (inlined) into higher-level functions.
All instances of a function, at the same level of hierarchy, uses the same RTL implementation (block).
```
The syn.directive.function_instantiate command creates a unique RTL implementation for each instance of a function.
Each instance can be optimized around a specific argument or variable.
By default, the following code results in a single RTL implementation of function func_sub for all three instances, or if func_sub is a
small function it is inlined into function func.

**Tip:** By default, the Vitis HLS tool automatically inlines small functions. This is true even for function instantiations. Using the
set_directive_inline off option can prevent this automatic inlining.

```
char func_sub(char inval, char incr)
{
return inval + incr;
}
void func(char inval1, char inval2, char inval3,
char *outval1, char *outval2, char * outval3)
{
*outval1 = func_sub(inval1, 1);
*outval2 = func_sub(inval2, 2);
*outval3 = func_sub(inval3, 3);
}
```

#### Displayed in the footer

###### !!

Using the directive as shown in the example section below results in three versions of function func_sub, each independently
optimized for variable incr.

### Syntax

```
syn.directive.function_instantiate=<location> <variable>
```
```
<location> is the location (in the format function[/region label]) where the instances of a function are to be made
unique.
<variable> specifies the function argument as a constant in the various function instantiations.
```
### Options

This command has no options.

### Examples

For the example code shown above, the following Tcl (or pragma placed in function func_sub) allows each instance of function
func_sub to be independently optimized with respect to input incr.

```
syn.directive.inline off=func_sub
syn.directive.function_instantiate=func_sub incr
```
### See Also

```
syn.directive.allocation
syn.directive.inline
```
syn.directive.inline

### Description

syn.directive.inline removes a function as a separate entity in the RTL hierarchy. After inlining, the function dissolves into the
calling function and no longer appears as a separate level of hierarchy.
**Important:** Inlining a child function also dissolves any pragmas or directives applied to that function. In Vitis HLS, any directives
applied in the child context are ignored.
In some cases, inlining a function allows operations within the function to be shared and optimized more effectively with the calling
function. However, an inlined function cannot be shared or reused, so if the parent function calls the inlined function multiple times, this
can increase the area and resource utilization.
By default, inlining is only performed on the next level of function hierarchy.

### Syntax

```
syn.directive.inline=[OPTIONS] <location>
```
```
<location> is the location (in the format function[/label]) where inlining is to be performed.
```
### Options

**off**
By default, Vitis HLS performs inlining of smaller functions in the code. Using the off option disables inlining for the specified
function.

**recursive**
By default, only one level of function inlining is performed. The functions within the specified function are not inlined. The
recursive option inlines all functions recursively within the specified function hierarchy.


#### Displayed in the footer

##### ★

##### ★

### Examples

The following example inlines function func_sub1, but no sub-functions called by func_sub1.

```
syn.directive.inline=func_sub1
```
This example inlines function func_sub1, recursively down the hierarchy, excluding function func_sub2:

```
syn.directive.inline=recursive func_sub1
syn.directive.inline=off func_sub2
```
### See Also

```
syn.directive.allocation
```
syn.directive.interface

### Description

Only the top-level function supports the syn.directive.interface directive. You cannot use this directive for sub-functions of the
HLS component. The INTERFACE pragma or directive specifies how function arguments creates RTL ports during interface synthesis.
See the Defining Interfaces section of the _Vitis High-Level Synthesis User Guide_ (UG1399). The Vitis HLS tool automatically determines
the I/O protocol used by any sub-functions.
Ports in the RTL implementation come from the following elements.

```
The data type and direction of the arguments of the top-level function and function return.
The flow_target for the HLS component.
The default interface configuration settings as specified by syn.interface.xxx commands (see Interface Configuration).
syn.directive.interface.
```
Each function argument can have its own I/O protocol (such as valid handshake or acknowledge handshake).

**Tip:** Global variables required on the interface must be explicitly defined as an argument of the top-level function as described in the
Global Variables section of the _Vitis High-Level Synthesis User Guide_ (UG1399). However, if a global variable is accessed, but all read
and write operations are local to the design, the resource is created in the design. There is no need for an I/O port in the RTL.
The interface also defines the execution control protocol of the HLS component. See the Block-Level Control Protocols section of the
_Vitis High-Level Synthesis User Guide_ (UG1399) for details. The control protocol controls when the HLS component (or block) starts
execution, and when the block completes operation, is idle and ready for new inputs.

### Syntax

```
syn.directive.interface=[OPTIONS] <location> <port>
```
```
<location> is the location (in the format function[/label]) where the function interface or registered output can be
specified.
<port> is the parameter (function argument) to synthesize the interface. You do not need the port name with these block control
modes: ap_ctrl_chain, ap_ctrl_hs, or ap_ctrl_none.
```
### Options

**Tip:** Many of the options below have default values defined with syn.interface.xxx commands. You can define local values for
the interface defined here to override the default values.


#### Displayed in the footer

##### ★

###### ✎

###### ✎

**mode=<mode>**
The following three categories illustrate the modes and how the tool implements them in RTL.

1. Port-Level Protocols:
    **ap_none**
       No port protocol. The interface is a simple data port.
    **ap_stable**
       No protocol. The interface is a simple data port. The Vitis HLS tool assumes the data port is always stable after reset.
       Allows optimizations to remove unnecessary registers.
    **ap_vld**
       Implements the data port with an associated valid signal to indicate when the data is valid for reading or writing.
    **ap_ack**
       Implements the data port with an associated acknowledge signal to acknowledge writing or reading of data.
    **ap_hs**
       Implements the data port with both valid and acknowledge signals to provide a two-way handshake to indicate when
       the data is valid for reading and writing. Acknowledges that the data was read or written
    **ap_ovld**
       Implements the output data port with an associated valid signal to indicate when the data is valid for reading or
       writing.

```
Tip: For ap_ovld Vitis HLS implements the input argument or the input half of any read/write arguments with mode
ap_none.
ap_memory
Implements array arguments as a standard RAM interface. If you use the RTL design in Vivado IP integrator, separate
ports make up the interface.
ap_fifo
Implements the port with a standard FIFO interface using data input and output ports with associated active-Low FIFO
empty and full ports.
Note: You can only use the ap_fifo interface on read arguments or write arguments. ap_fifo mode does not
support bidirectional read/write arguments.
```
2. AXI Interface Protocols:
    **s_axilite**
       Implements the port as an AXI4-Lite interface. The tool produces an associated set of C driver files when exporting the
       generated RT for the HLS component.
    **m_axi**
       Implements the port as an AXI4 interface. You can use the syn.interface.m_axi_addr64 command to specify
       either 32-bit (default) or 64-bit address ports and to control any address offset.
    **axis**
       Implements the port as an AXI4-Stream interface.
3. Block-Level Control Protocols:
    **ap_ctrl_chain**
       Implements a set of block-level control ports to perform the following actions.
          Star the design operation with start
          Indicate when the design is idle
          Indicate when the design is done
          New input data with ready
    **ap_ctrl_hs**
       Implements a set of block-level control ports to start the design operation. Also indicates when the design is idle,
       done, and ready for new input data.
    **ap_ctrl_none**
       No block-level I/O protocol.
          **Note:** Using the ap_ctrl_none mode can prevent the design from being verified using C/RTL co-simulation.


#### Displayed in the footer

##### ★

###### !!

##### ★

**bundle=<string>**
By default, the HLS tool groups (or bundles) function arguments that are compatible into a single interface port in the RTL code. All
interface ports with compatible options, such as mode, offset, and bundle, are grouped into a single interface port.

```
Tip: This default can be changed using the syn.interface.m_axi_auto_max_ports command.
The bundle=<string> option lets you define bundles to group ports together, overriding the default behavior. The <string>
specifies the bundle name. A combination of the mode and bundle provides the port name in the generated RTL. The name can
also specify the port name.
Important: You must specify bundle names using lower-case characters.
```
**clock=<string>**
By default, the AXI4-Lite interface clock is the same clock as the system clock. This option specifies a separate clock for an AXI4-
Lite interface. If the bundle option groups multiple top-level function arguments into a single AXI4-Lite interface, you only need to
specify the clock option on one of bundle members.

**channel=<string>**
To enable multiple channels on an m_axi interface specify the channel ID. Multiple m_axi interfaces can be combined into a
single m_axi adapter using separate channel IDs.

**depth=<int>**
Specifies the maximum number of samples for the test bench to process. This setting indicates the maximum size of the FIFO
needed in the verification adapter that the HLS tool creates for RTL co-simulation.

```
Tip: While depth is usually an option, it is needed to specify the size of pointer arguments for RTL co-simulation.
```
**interrupt=<int>**
Only ap_vld/ap_hs uses this option. This option enables managing of I/O in interrupt. This option creates the corresponding bits
in the ISR and IER in the s_axilite register file. The integer value N=16..31 specifies the bit position in both registers. By
default, the bit position is assigned contiguously from 16.

**latency=<value>**
This option can be used on ap_memory and m_axi interfaces.
In an ap_memory interface, the option specifies the read latency of the RAM resource driving the interface. The default is a
read operation of 1 clock cycle. This option allows modeling of external RAM with more than 1 clock cycle of read latency.
In an m_axi interface the option specifies the expected latency of the AXI4 interface, allowing the design to initiate a bus
request the specified number of cycles (latency) before the read or write is expected. If this figure it too low, the design can
be ready too soon and can stall waiting for the bus. If this figure is too high, bus access can be idle waiting on the design to
start the access.

**max_read_burst_length=<int>**
For use with the m_axi interface, this option specifies the maximum number of data values read during a burst transfer. Refer to
the AXI Burst Transfers section of the _Vitis High-Level Synthesis User Guide_ (UG1399) for more information.

**max_write_burst_length=<int>**
For use with the m_axi interface, this option specifies the maximum number of data values written during a burst transfer.

**max_widen_bitwidth=<int>**
Specifies the maximum bit width available for the interface when automatically widening the interface. This overrides the default
value specified by the syn.interface.m_axi_max_bitwidth command. Refer to Automatic Port Width Resizing in the _Vitis
High-Level Synthesis User Guide_ (UG1399) for more information.

**name=<string>**
Specifies a name for the port for the generated RTL to use. A combination of the mode and bundle specifies the port name. The
exception is if name specifies the name.

**num_read_outstanding=<int>**
For use with the m_axi interface, this option specifies how many read requests can be made to the AXI4 bus, without a response,
before the design stalls. This implies internal storage in the design, and a FIFO of size:

```
num_read_outstanding*max_read_burst_length*word_size
```

#### Displayed in the footer

##### ★

##### ★

##### ★

##### ★

**num_write_outstanding=<int>**
For use with the m_axi interface. This option specifies how many write requests the AXI4 bus can take, without a response,
before the design stalls. This implies internal storage in the design, and a FIFO of size:

```
num_write_outstanding*max_write_burst_length*word_size
```
**offset=<string>**
Controls the address offset in AXI4-Lite (s_axilite) and AXI4 memory mapped (m_axi) interfaces for the specified port.
In an s_axilite interface, <string> specifies the address in the register map.
In an m_axi interface, this option overrides the global option specified by the config_interface -m_axi_offset option
and <string>. See the following for details.
off: Does not generate an offset port.
direct: Generates a scalar input offset port.
slave: Generates an offset port and automatically map it to an AXI4-Lite slave interface. This is the default offset.

**register**
Registers the signal and any associated protocol signals. Instructs the signals to persist until at least the last cycle of the function
execution. The syn.interface.register_io command controls default registering of all interfaces on the top function, while
this option lets you override the default for the current interface. This option applies to the following interface modes:
s_axilite
ap_fifo
ap_none
ap_stable
ap_hs
ap_ack
ap_vld
ap_ovld

```
Tip: The register option cannot be used on the return port of the function (port=return). Use the
syn.directive.latency instead.
```
**register_mode=(both|forward|reverse|off)**
This option applies to AXI4-Stream interfaces, and specifies if registers are placed on the following places.
Forward path (TDATA and TVALID), or
The reverse path (TREADY) on both paths, or
None of the ports signals (off).
The default is register_mode=both.

```
Tip: AXI4-Stream side-channel signals are data signals. The signals register whenever the TDATA is register.
```
**storage_impl=<impl>**
For use with mode=s_axilite only, this options defines a storage implementation to assign to the interface. Supported <impl>
values include auto, bram, and uram. The default is auto.

```
Tip: uram is a synchronous memory with a single clock for two ports available only on certain devices. Therefore uram cannot
be specified for an s_axilite adapter with two clocks, or when the specified part does not support uram.
```
**storage_type=<type>**
For use with mode=ap_memory or mode=bram only, this options defines a storage type (for example, RAM_T2P) to assign to the
variable.
Supported types include: ram_1p, ram_1wnr, ram_2p, ram_s2p, ram_t2p, rom_1p, rom_2p, and rom_np.

```
Tip: For objects on the interface of the top-level function, you can define this option using syn.directive.bind_storage.
```
### Example 1

This example disables function-level handshakes for function func.

```
syn.directive.interface=mode=ap_ctrl_none func return
```

#### Displayed in the footer

##### ★

### Example 2

Argument InData in function func has a ap_vld interface. The input need to be registered.

```
set_directive_interface=func InData mode=ap_vld register
```
### See Also

```
syn.directive.bind_storage
syn.directive.latency
```
syn.directive.latency

### Description

Specifies a maximum or minimum latency value, or both, on a function, loop, or region.
Vitis HLS always aims for minimum latency. The following illustrates the behavior of the tool after specifying minimum and maximum
latency values.

```
Latency is less than the minimum: If Vitis HLS can achieve less than the minimum specified latency, it extends the latency to the
specified value. This action can enable increased sharing.
Latency is greater than the minimum: The constraint is satisfied. The tool performs no further optimizations.
Latency is less than the maximum: The constraint is satisfied. The tool performs no further optimizations.
Latency is greater than the maximum: If Vitis HLS cannot schedule within the maximum limit, it increases effort to achieve the
specified constraint. If it still fails to meet the maximum latency, it issues a warning. Vitis HLS then produces a design with the
smallest achievable latency.
```
**Tip:** You can also use syn.directive.latency to limit the efforts of the tool to find an optimum solution. Specify latency
constraints for scopes within the code. Loops, functions, or regions, reduces the possible solutions within that scope and can improve
tool compilation time. Refer to the Improving Synthesis Runtime and Capacity section of the _Vitis High-Level Synthesis User Guide_
(UG1399) for more information.
To limit the total latency of all loop iterations, apply the latency directive to a region that encompasses the entire loop. For example:
syn.directive.latency Region_All_Loop_A max=10

```
Region_All_Loop_A: {
Loop_A: for (i=0; i<N; i++)
{
..Loop Body...
}
}
```
In this case, even if the loop is unrolled, the latency directive sets a maximum limit on all loop operations.

### Syntax

```
syn.directive.latency=[OPTIONS] <location>
```
```
<location> is the location (function, loop or region) (in the format function[/label]) to be constrained.
```
### Options

**max=<integer>**
Limits the maximum latency.

**min=<integer>**
Limits the minimum latency.


#### Displayed in the footer

###### ✎

###### !!

### Examples

Function foo has a minimum latency of 4 and a maximum latency of 8.

```
syn.directive.latency=min=4 max=8 foo
```
In function foo, loop loop_row is specified to have a maximum latency of 12.

```
syn.directive.latency=max=12 foo/loop_row
```
syn.directive.loop_flatten

### Description

Flattens nested loops into a single loop hierarchy.
In the RTL implementation, it costs a clock cycle to move between loops in the loop hierarchy. Flattening nested loops allows
optimization of loops to a single loop. This saves clock cycles, potentially allowing for greater optimization of the loop body logic.
**Recommended:** Apply this directive to the inner-most loop in the loop hierarchy. You can only flatten perfect and semi-perfect loops
in this manner.

**Perfect loop nests**
Only the innermost loop has loop body content.
There is no logic specified between the loop statements.
All loop bounds are constant.

**Semi-perfect loop nests**
Only the innermost loop has loop body content.
There is no logic specified between the loop statements.
The outermost loop bound can be a variable.

**Imperfect loop nests**
When the inner loop has variables bounds, or if the loop body is not exclusively inside the inner loop, do the following. Try to
restructure the code, or unroll the loops in the loop body to create a perfect loop nest.

### Syntax

```
syn.directive.loop_flatten=[OPTIONS] <location>
```
```
<location> is the location (inner-most loop), in the format function[/label].
```
### Options

**off**
Prevents loop flattening from taking place. Can prevent some loops from being flattened while all others in the specified location
are flattened.
**Important:** The presence of the LOOP_FLATTEN pragma or directive enables the optimization. The addition of off disables it.

### Examples

Flattens loop_1 in function foo and all (perfect or semi-perfect) loops above it in the loop hierarchy, into a single loop.

```
set_directive_loop_flatten=foo/loop_1
```
Prevents loop flattening in loop_2 of function foo.

```
set_directive_loop_flatten=off foo/loop_2
```

#### Displayed in the footer

###### !!

### See Also

```
syn.directive.loop_flatten
```
syn.directive.loop_merge

### Description

Merges all loops into a single loop. Merging loops:

```
Reduces the number of clock cycles required in the RTL to transition between the loop-body implementations.
Allows implementation of loops in parallel (if possible).
```
The rules for loop merging are:

```
If the loop bounds are variables, they must have the same value (number of iterations).
If loops bounds are constants, the tool uses the maximum constant value as the bound of the loop being merged.
Loops with both variable bounds and constant bounds cannot be merged.
The code between merging loops cannot have side effects. Multiple executions of this code need to generate the same results.
a=b is allowed
a=a+1 is not allowed.
Loops cannot be merged when they contain FIFO reads. Merging changes the order of the reads. Reads from a FIFO or FIFO
interface must always be in sequence.
```
### Syntax

```
syn.directive.loop_merge=[options] <location>
```
```
<location> is the location (in the format function[/label]) at which the loops reside.
```
### Options

**force**
Forces loops to be merged even when Vitis HLS issues a warning. Ensures that the merged loop functions correctly.

### Examples

Merges all consecutive loops in function foo into a single loop.

```
syn.directive.loop_merge=foo
```
All loops inside loop_2 of function foo (but not loop_2 itself) are merged by using the force option.

```
syn.directive.loop_merge=force foo/loop_2
```
### See Also

```
syn.directive.loop_flatten
syn.directive.unroll
```
syn.directive.loop_tripcount

### Description

The _loop tripcount_ is the total number of iterations performed by a loop. Vitis HLS reports the total latency of each loop (the number of
cycles to execute all iterations of the loop). This loop latency is therefore a function of the tripcount (number of loop iterations).
**Important:** syn.directive.loop_tripcount is for analysis only, and does not impact the results of synthesis.


#### Displayed in the footer

##### ★

The tripcount can be a constant value. It can depend on the value of variables used in the loop expression (for example, x<y) or control
statements used inside the loop.
Vitis HLS cannot determine the tripcount in some cases. These cases include, for example, those in which the variables used to
determine the tripcount are:

```
Input arguments, or
Variables calculated by dynamic operation
```
In the following example, the value of input num_samples determines the maximum iteration of the for-loop. The value of
num_samples is not defined in the C function, but comes into the function from the outside.

```
void foo (num_samples, ...) {
int i;
...
loop_1: for(i=0;i< num_samples;i++) {
...
result = a + b;
}
}
```
syn.directive.loop_tripcount allows you to specify minimum, maximum, and average iterations for a loop if the loop latency is
unknown. The tool can analyze how the loop latency contributes to the total design latency in reports, helping you determine
appropriate design optimizations.

**Tip:** If you use a C assert macro to limit the size of a loop variable, Vitis HLS performs the following. Vitis HLS uses the macro to
both define loop limits for reporting and create hardware that is exactly sized to these limits.

### Syntax

```
syn.directive.loop_tripcount=[OPTIONS] <location>
```
```
<location> is the location of the loop (in the format function[/label]) at which the tripcount is specified.
```
### Options

**avg=<integer>**
Specifies the average number of iterations.

**max=<integer>**
Limits the maximum number of iterations.

**min=<integer>**
Limits the minimum number of iterations.

### Examples

loop_1 in function foo has a minimum tripcount of 12, and a maximum tripcount of 16:

```
syn.directive.loop_tripcount=min=12 max=16 avg=14 foo/loop_1
```
syn.directive.occurrence

### Description

The OCCURRENCE pragma or directive specifies the following when pipeling functions and loops. The code in a pipelined function call
within the pipelined function or loop executes at a lower rate than the surrounding function or loop. Executing at a lower rate allows the
pipelined call to pipeline at a lower rate and potentially shared within the top-level pipeline. For example:


#### Displayed in the footer

###### !!

##### ★

```
A loop iterates N times.
Part of the loop is protected by a conditional statement and only executes M times, where N is an integer multiple of M.
The code protected by the conditional is said to have an occurrence of N/M.
```
Identifying a region with an OCCURRENCE rate allows the functions and loops in this region to be pipelined with an initiation interval
that is slower than the enclosing function or loop.

### Syntax

```
syn.directive.occurrence=[OPTIONS] <location>
```
```
<location> specifies the block of code that contains the pipelined function call(s) with a slower rate of execution.
```
### Options

**cycle=<** **_int_** **>**
Specifies the occurrence _N/M_ where:
_N_ is the number of times the enclosing function or loop is executed.
_M_ is the number of times the conditional region is executed.
**Important:** _N_ must be an integer multiple of _M_.

**off=true**
Disable occurrence for the specified function.

### Examples

Region Cond_Region in function foo has an occurrence of 4. It executes at a rate four times slower than the code that encompasses
it.

```
syn.directive.occurrence=cycle=4 foo/Cond_Region
```
### See Also

```
syn.directive.pipeline
```
syn.directive.performance

### Description

**Tip:** syn.directive.performance applies to loops and loop nests, and requires a known loop tripcount to determine the
performance. If your loop has a variable tripcount then you must also specify syn.directive.tripcount.
The syn.directive.performance lets you specify a high-level constraint, target_ti or target_tl. The directive allows you to
define the number of clock cycles between successive starts of a loop. The directive lets the tool infer lower-level UNROLL, PIPELINE,
ARRAY_PARTITION, and INLINE directives for achieving the desired result. The syn.directive.performance does not guarantee
the specified value will be achieved, and so it is only a target.
The target_ti is the interval between successive starts of the loop. It is also the interval between the start of the first iteration of the
loop, and the next start of the first iteration of the loop. In the following code example, a target_ti=T means the target interval for the
start of loop L2 between two consecutive iterations of L1 is 100 cycles.

```
const int T = 100;
L1: for (int i=0; i<N; i++)
L2: for (int j=0; j<M; j++){
#pragma HLS PERFORMANCE target_ti=T
...
}
```

#### Displayed in the footer

###### ✎

###### ✎

The target_tl is the interval between start of the loop and end of the loop, or between the start of the first iteration of the loop and
the completion of the last iteration of the loop. In the preceding code example a target_tl=T means the target completion of loop L2
for a single iteration of L1 should be 100 cycles.
**Note:** syn.directive.inline applies automatically to functions inside any pipelined loop with II=1 to improve throughput. If you
apply the PERFORMANCE pragma, the tool also triggers auto-inline optimization. If you apply a directive that infers a pipeline with II=1,
the tool also triggers auto-inline optimization. You can disable this for specific functions by using syn.directive.inline off.
The transaction interval is the initiation interval (II) of the loop times the number of iterations, or tripcount: target_ti = II * loop tripcount.
Conversely, target_ti = FreqHz / Operations per second.
For example, assume an image processing function processes a single frame per invocation with a throughput goal of 60 fps. The
target throughput for the function is 60 invocations per second. If the clock frequency is 180 MHz, then target_ti is 180M/60, or 3
million clock cycles per function invocation.

### Syntax

Specify the directive for a labeled loop.

```
syn.directive.performance=<location> [Options]
```
Where:
<location> specifies the loop in the format function/loop_label.

### Options

**target_ti=<value>**
Specifies a target transaction interval that is the number of clock cycles for the loop to complete an iteration. The transaction
interval refers to the number of clock cycles from the first transaction of a loop, or nested loop to the start of the next transaction of
the loop. The <value> can be an integer, floating point, or constant expression that is resolved by the tool as an integer.
**Note:** The tool returns a warning if truncation occurs.

**target_tl=<value>**
Specifies a target latency that is the number of clock cycles for the loop to complete all iterations. The transaction latency is the
interval between the start of the first iteration of the loop, and the completion of the last iteration of the loop. The <value> can be
an integer, floating point, or constant expression that is resolved by the tool as an integer.

**unit=[sec | cycle]**
Specifies the unit associated with the target_ti or target_tl values. The unit can either be seconds, or clock cycles. When
the unit is specified as seconds, you can specify it with a value to indicate nanoseconds (ns), picoseconds (ps) or microseconds
(us).

### Example 1

The loop labeled loop_1 has a target transaction interval of 4 clock cycles:

```
syn.directive.performance=loop_1 target_ti=4
```
### See Also

```
syn.directive.inline
```
syn.directive.pipeline

### Description

Reduces the initiation interval (II) for a function or loop by allowing the concurrent execution of operations. See the Pipelining Paradigm
section of the _Vitis High-Level Synthesis User Guide_ (UG1399) for details. A pipelined function or loop can process new inputs every N
clock cycles, where N is the initiation interval (II). An II of 1 processes a new input every clock cycle.


#### Displayed in the footer

###### ✎

###### !!

As a default, the tool generates the minimum II for the design according to the clock period constraint. The emphasis is on meeting
timing, rather than on achieving II, unless you specified the -II.
syn.compile.pipeline_style defines the default type of pipeline command as described in Compile Options. You can override it
with the PIPELINE pragma or directive.
If Vitis HLS cannot create a design with the specified II, it issues a warning and creates a design with the lowest achievable II. You can
analyze the design with the warning messages to determine the steps for creating a design satisfying the required initiation interval.

### Syntax

```
syn.directive.pipeline=[OPTIONS] <location>
```
Where:

```
<location> is the location (in the format function[/label]) to be pipelined.
```
### Options

**II=<integer>**
Specifies the desired initiation interval for the pipeline. Vitis HLS tries to meet this request. Based on data dependencies, the
actual result can have a larger II.

**off**
Turns off pipeline for a specific loop or function. Use this option when using config_compile -pipeline_loops to globally
pipeline loops.

**rewind
Note:** Applicable only to a loop.
Optional keyword. Enables rewinding as described in Rewiding Pipelined Loops for Performance of _Vitis High-Level Synthesis
User Guide_ (UG1399). This enables continuous loop pipelining with no pause between one execution of the loop ending and the
next execution starting. Rewinding is effective only if there is one single loop (or a perfect loop nest) inside the top-level function.
The code segment before the loop:
Is considered as initialization.
Is executed only once in the pipeline.
Cannot contain any conditional operations (if-else).

**style=<stp | frp | flp>**
Specifies the type of pipeline to use for the specified function or loop. For more information on pipeline styles refer to the Flushing
Pipeline and Pipeline Types section of the _Vitis High-Level Synthesis User Guide_ (UG1399). The types of pipelines include:
**stp**
Stall pipeline. Runs only when input data is available otherwise it stalls. This is the default setting, and is the type of pipeline
used by Vitis HLS for both loop and function pipelining. Use this when you don't require a flushable pipeline. For example,
when there are no performance or deadlock issues due to stalls.
**flp**
Defines the pipeline as a flushable pipeline. This type of pipeline typically consumes more resources and can have a larger II
because you cannot share resources among pipeline iterations.
**frp**
Free-running, flushable pipeline. Runs even when input data is not available. Use this when you need better timing due to
reduced pipeline control signal fanout, or when you need improved performance to avoid deadlocks. However, this pipeline
style can consume more power as the pipeline registers are clocked even if there is no data.
**Important:** This constraint is not a hard constraint. The tool checks design conditions for enabling pipelining. It is possible for
some loops to not conform to a particular style and the tool reverts to the default style (stp) when necessary.

### Examples

Function func is pipelined with the specified initiation interval.

```
syn.directive.pipeline=func II=1
```

#### Displayed in the footer

##### ★

### See Also

```
syn.directive.dependence
```
syn.directive.protocol

### Description

Specifies a region of code, a protocol region, in which Vitis HLS does not insert clock operations unless the code explicitly specifies.
The tool does not insert any clocks between operations in the region, including clocks that read or write to function arguments. The
synthesized RTL strictly follows the order of read and writes.
A region of code can be created in the C/C++ code by enclosing the region in braces "{ }" and naming it. The following defines a region
named io_section:

```
io_section:{
...
lines of code
...
}
```
You can specify a clock operation in C code using an ap_wait() statement. You can specify a clock operation in C++ code using the
wait() statement.

```
Tip: The ap_wait and wait statements have no effect on the simulation of the design.
```
### Syntax

```
syn.directive.protocol=[OPTIONS] <location>
```
The <location> specifies the location (in the format function[/label]) where you define the protocol region.

### Options

**mode=[floating | fixed]**
floating: Lets code statements outside the protocol region overlap and execute in parallel with statements in the protocol
region in the final RTL. The protocol region remains cycle accurate, but outside operations can occur at the same time. This
is the default mode.
fixed: The fixed mode ensures that statements outside the protocol region do not execute in parallel with the protocol
region.

### Examples

The example code defines a protocol region, io_section in function foo. The following directive defines that region as a fixed mode
protocol region:

```
syn.directive.protocol=mode=fixed foo/io_section
```
syn.directive.reset

### Description

The RESET pragma or directive adds or disables reset ports for specific state variables (global or static).
Use the reset port to restore the registers and block RAM connecting to the port to an initial value. Do this anytime when you apply a
reset signal. Globally, the syn.rtl.reset configuration settings configure the presence and behavior of the RTL reset port. The reset
configuration settings include the ability to define the polarity of the reset, and specify whether the reset is synchronous or
asynchronous. More importantly, the reset configuration settings controls which registers are reset when the reset signal is applied. For
more information, see Controlling Initialization and Reset Behaviour in _Vitis High-Level Synthesis User Guide_ (UG1399).


#### Displayed in the footer

###### ✎

The RESET pragma provides more specific control over reset. For global or static variables, the RESET pragma explicitly enables a
reset when none is present. You can also remove the variable from the reset by turning off the pragma. This can be particularly useful
when static or global arrays are present in the design.
**Note:** For public variables of a class, you must use the RESET pragma as the reset configuration settings only apply to variables
declared at the function or file level. In addition, you must apply the RESET pragma to an object of the class in the top-function or sub-
function. You cannot apply it to private variables of the class.

### Syntax

Place the pragma in the C source within the boundaries of the variable life cycle.

```
syn.directive.reset=<location> [ variable=<a> | off=true ]
```
Where:

**location>**
Specifies the location (in the format function[/label]) where you define the variable.

**variable=<a>**
Specifies the variable to which you apply the RESET pragma.

**off or off=true**
Indicates that reset is not generated for the specified variable.

### Examples

Adds reset to variable a in function foo even when the global reset (syn.rtl.reset) setting is none or control.

```
syn.directive.reset=foo a
```
Removes reset from variable static int a in function foo even when the global reset setting is state or all.

```
syn.directive.reset=off foo a
```
The following example shows the use of the RESET pragma or directive with class variables and methods. When you declare a
variable in a class, it must be a public static variable in the class. You can create the RESET pragma for the variable in a method of the
class. You can also create the RESET pragma after creating an object in the top function or sub-function of the HLS design.

```
class bar {
public:
static int a; // class level static; must be public
int a1; // class-level non-static; must be public
bar(...) {
// #pragma reset does not work here for a or a1
}
// this is the function that is called in the top
void run(...) {
// #pragma reset does not work here for non-static variable a1
// but does work for static variable a
#pragma reset variable=a
}
};
static int c; // file level
int d; // file level
void top(...) {
#pragma HLS top
static int b; // function level
bar t;
static bar s;
```
```
// #pragma reset works here for b, c and d, as well as t and s members
```

#### Displayed in the footer

```
// except for t.a1 because it is not static
#pragma reset variable=b
#pragma reset variable=c
#pragma reset variable=d
#pragma reset variable=t.a
#pragma reset variable=s.a
#pragma reset variable=s.a1
t.run(...);
s.run(...);
}
```
The following shows the syn.directive.reset command specified for the function top.

```
syn.directive.reset=top variable=b
syn.directive.reset=top variable=c
syn.directive.reset=top variable=d
syn.directive.reset=top variable=t.a
syn.directive.reset=top variable=s.a
syn.directive.reset=top variable=s.a1
```
### See Also

```
RTL Configuration
```
syn.directive.stable

### Description

Apply the syn.directive.stable to arguments of a DATAFLOW or PIPELINE region. The pragma indicates that an input or output
of this region can be ignored when generating the synchronizations at entry and exit of the DATAFLOW region. Reading accesses of
that argument do not need to be part of the “first stage” of the task-level (resp. fine-grain) pipeline for inputs. Write accesses do not
need to be part of the last stage of the task-level (resp. fine-grain) pipeline for outputs.
This pragma can be specified at any point in the hierarchy, on a scalar or an array. The pragma automatically applies to all the
DATAFLOW or PIPELINE regions below that point. A DATAFLOW or PIPELINE region can start another iteration even though the value
of the previous iteration has not been read yet. For an output, a write of the next iteration can occur even when the previous iteration is
not complete.

### Syntax

```
syn.directive.stable=<location> <variable>
```
```
<location> is the function name or loop name for constraining the directive.
<variable> is the name of the array to be constrained.
```
### Examples

In the following example, without the STABLE directive, proc1 and proc2 synchronize to acknowledge the reading of their inputs
(including A). With the directive, A is no longer considered as an input requiring synchronization.

```
void dataflow_region(int A[...], int B[...] ...
proc1(...);
proc2(A, ...);
```
The directives for this example are described below.

```
syn.directive.stable=dataflow_region variable=A
syn.directive.dataflow dataflow_region
```

#### Displayed in the footer

###### !!

###### ✎

##### ★

### See Also

```
syn.directive.dataflow
syn.directive.pipeline
```
syn.directive.stream

### Description

By default, array variables implement as RAM.

```
Top-level function array parameters implement as a RAM interface port.
General arrays implement as RAMs for read-write access.
Arrays involved in sub-functions, or loop-based DATAFLOW optimizations implement as a RAM ping-pong buffer channel.
```
If the data stored in the array is consumed or produced in a sequential manner, it is more efficient to use streaming data, with FIFOs
instead of RAMs. When specifying an argument of the top-level function as INTERFACE mode=ap_fifo, the array implements as
streaming. See the Interfaces for Vivado IP Flow section of the _Vitis High-Level Synthesis User Guide_ (UG1399) or more information.
**Important:** To preserve the accesses, it might be necessary to prevent compiler optimizations (in particular dead code elimination) by
using the volatile qualifier. See the Type Qualifiers section of the _Vitis High-Level Synthesis User Guide_ (UG1399).

### Syntax

```
syn.directive.stream=[OPTIONS] <location> <variable>
```
```
<location> is the location (in the format function[/label]) which contains the array variable.
<variable> is the array variable to be implemented as a FIFO.
```
### Options

**depth=<integer>
Note:** Relevant only for array streaming in dataflow channels.
By default, the depth of the FIFO in the RTL is the same size as the array in the C code. This options allows you to modify the size
of the FIFO.
When implementing the array in a DATAFLOW region, you can use the -depth option to reduce the size of the FIFO. For
example, in a DATAFLOW region where all loops and functions are processing data at a rate of II = 1, you don't need a large FIFO.
Data is produced and consumed in each clock cycle. You can use the -depth to reduce the FIFO size to 2 to substantially reduce
the area of the RTL design.
The config_dataflow command with the -depth option provides the same functionality for all arrays in a DATAFLOW region.
The -depth option with set_directive_stream overrides the default in config_dataflow.

**type=<arg>**
Specifies a mechanism to select between FIFO, PIPO, synchronized shared (shared), and un-synchronized shared (unsync).
The supported types include:
fifo: A FIFO buffer with the specified depth.
pipo: A regular Ping-Pong buffer, with as many “banks” as the specified depth (default is 2).
shared: A shared channel, synchronized like a regular Ping-Pong buffer, with depth, but without duplicating the array data.
Consistency can be ensured by setting the depth small enough, which acts as the distance of synchronization between the
producer and consumer.

```
Tip: The default depth for shared is 1.
unsync: Does not have any synchronization except for individual memory reads and writes. Consistency (read-write and
write-write order) must be ensured by the design itself.
```
### Examples

Specifies array A[10] in function func to be streaming and implemented as a FIFO.


#### Displayed in the footer

```
syn.directive.stream=func A type=fifo
```
Array B in named loop loop_1 of function func is set to streaming with a FIFO depth of 12. In this case, place the pragma inside
loop_1.

```
syn.directive.stream=depth=12 type=fifo func/loop_1 B
```
Array C has streaming implemented as a PIPO.

```
syn.directive.stream=type=pipo func C
```
### See Also

```
syn.directive.dataflow
syn.directive.interface
Dataflow Configuration
```
syn.directive.top

### Description

Attaches a name to a function. The name can then be used by the set_top command to set the named function as the top. This is
typically used to synthesize member functions of a class in C++.

### Syntax

```
syn.directive.top=[OPTIONS] <location>
```
```
<location> is the function to be renamed.
```
### Options

**name=<string>**
Specifies the name of the function to be used by the syn.top command as described in HLS General Options.

### Examples

Function foo_long_name renames to DESIGN_TOP. The top-level is then specified as DESIGN_TOP. If the pragma is placed in the
code, the set_top command must still be issued in the top-level specified in the GUI project settings.

```
syn.directive.top=name=DESIGN_TOP foo_long_name
```
Followed by the syn.top=DESIGN_TOP command.

### See Also

```
syn.top in HLS General Options
```
syn.directive.unroll

### Description

Transforms loops by creating multiples copies of the loop body.
The loop induction variable specifies the number of iterations the loop executes. Logic inside the loop body (for example, break or
modifications to any loop exit variable) also impacts the number of iterations. A block of logic representing the loop body implements the
loop in RTL. The logic executes for the same number of iterations.


#### Displayed in the footer

The syn.directive.unroll command allows partial or full unrolling of the loop. Fully unrolling the loop creates as many copies of
the loop body in the RTL as there are loop iterations. Partially unrolling a loop by a factor _N_ , creates _N_ copies of the loop body and
adjusting the loop iteration accordingly.
If factor _N_ for partial unrolling is not an integer multiple of the original loop iteration count, check the original exit condition after each
loop body's unrolled fragment.
To unroll a loop completely, the loop bounds must be known at compile time. This is not required for partial unrolling.

### Syntax

```
syn.directive.unroll=[OPTIONS] <location>
```
```
<location> is the location of the loop (in the format function[/label]) to be unrolled.
```
### Options

**factor=<integer>**
Specifies a non-zero integer indicating that partial unrolling is requested.
The loop body repeats the integer number of times. The iteration information adjusts accordingly.

**skip_exit_check**
Effective only if a factor is specified (partial unrolling).
Fixed bounds
Does not perform exit condition check if the iteration count is a multiple of the factor.
If the iteration count is _not_ an integer multiple of the factor, the tool:
Prevents unrolling.
Issues a warning that the exit check is required to proceed.
Variable bounds
The exit condition check is removed. You must ensure that:
The variable bounds is an integer multiple of the factor.
Requires no exit check.

**off=true**
Disables unroll for the specified loop.

### Examples

Unrolls loop L1 in function foo. Places the pragma in the body of loop L1.

```
syn.directive.unroll=foo/L1
```
Specifies an unroll factor of 4 on loop L2 of function foo. Removes the exit check. Place the pragma in the body of loop L2.

```
syn.directive.unroll=skip_exit_check factor=4 foo/L2
```
### See Also

```
syn.directive.loop_flatten
syn.directive.loop_merge
```
HLS Pragmas

### Optimizations in Vitis HLS

In the AMD Vitis™ software platform, a kernel defined in the C/C++ language must be compiled into the register transfer level (RTL)
that can be implemented into the programmable logic of an AMD device. The v++ compiler calls the Vitis High-Level Synthesis (HLS)
tool to synthesize the RTL code from the kernel source code.


#### Displayed in the footer

The HLS tool is intended to work with the Vitis IDE project without interaction. However, the HLS tool also provides pragmas that can be
used to optimize the design, reduce latency, improve throughput performance, and reduce area and device resource usage of the
resulting RTL code. These pragmas can be added directly to the source code for the kernel.
The HLS pragmas include the optimization types specified in the following table.
For detailed pragma information, refer to the _Vitis High-Level Synthesis User Guide_ (UG1399).

**Table: Vitis HLS Pragmas by Type**

```
Type Attributes
```
```
Kernel Optimization
pragma HLS aggregate
pragma HLS bind_op
pragma HLS bind_storage
pragma HLS expression_balance
pragma HLS latency
pragma HLS reset
pragma HLS top
```
```
Function Inlining
pragma HLS inline
```
```
Interface Synthesis
pragma HLS interface
```
```
Task-level Pipeline
pragma HLS dataflow
pragma HLS stream
```
```
Pipeline
pragma HLS pipeline
pragma HLS occurence
```
```
Loop Unrolling
pragma HLS unroll
pragma HLS dependence
```
```
Loop Optimization
pragma HLS loop_flatten
pragma HLS loop_merge
pragma HLS loop_tripcount
```
```
Array Optimization
pragma HLS array_partition
pragma HLS array_reshape
```
```
Structure Packing
pragma HLS aggregate
pragma HLS dataflow
```
###### v++ Linking and Packaging Options

### Vitis Subsystem (VSS) Creation using the V++ Linker

A Vitis Subsystem (VSS) is created by the v++ linker using --mode vss. An example command is shown below:

```
v++ --link --mode vss --save-temps --part <part_name> --config ./src/vss_conn.cfg <list_of_xo>
<list_of_libadf_partitions> --out_dir <build_folder>
```

#### Displayed in the footer

##### ★

##### ★

The following are options used in VSS generation.

```
--part specifies a target part
--config defines connectivity and other directives
--out_dir specifies a folder containing metadata (.vss) and binary files
```
See Linking a VSS Component in the _Embedded Design Development Using Vitis_ (UG1701) for details.

### Linking

The individual v++ --link commands can be found in the v++ Command chapter. See Linking the System in the _Embedded Design
Development Using Vitis_ (UG1701).

### Packaging

The details of the v++ packaging process are provided in Integrating the System in the _Embedded Design Development Using Vitis_
(UG1701).
The individual v++ --package commands can be found in v++ Command.

###### --advanced Options

The --advanced.param and --advanced.prop options specify parameters and properties for use by the v++ command. When
compiling or linking, these options offer fine-grain control over the hardware generated by the Vitis core development kit, and the
hardware emulation process.
The arguments for the --advanced.xxx options are specified as <param_name>=<param_value>. For example:

```
v++ --link -–advanced.param compiler.enableXSAIntegrityCheck=true
-–advanced.prop kernel.foo.kernel_flags="-std=c++0x"
```
**Tip:** The order of precedence between the command line and the config file, and multiple entries in the config file are described in
Vitis Compiler Configuration File. However, the --advanced commands described here have the opposite precedence: config file
commands take precedence over command-line arguments, and the last command encountered takes precedence over any earlier
occurrences.

### --advanced.param

```
--advanced.param <param_name>=<param_value>
```
Specifies advanced parameters as described in the following table.

### Param Options

**Table: Param Options**

```
Parameter Name Valid Values Description
```
```
compiler.acceleratorBinaryContentType: String
Default Value:
<empty>
```
```
Design content to insert in the generated xclbin file. Valid options include
bitstream, pdi, or dcp.
bitstream and pdi are mutually exclusive. pdi applies to Versal
platforms, bitstreamapplies to non-Versal platforms.
```
```
Tip:
You can specify two values to have v++ generate two xclbin files: one
containing a DCP file, and the other containing either a bitstream or PDI file.
For example:
```
```
--advanced.param
compiler.acceleratorBinaryContent=dcp,bitstream
```

#### Displayed in the footer

##### ★

**Parameter Name Valid Values Description**

```
--advanced.param
compiler.acceleratorBinaryContent=dcp,pdi
```
```
This parameter is used while building the hardware target, this option
applies to:
```
```
v++ --link
vpl.impl
xclbinutil
```
compiler.axiDeadLockFreeType: Boolean
Default Value:
TRUE

```
Avoid dead locks. This option is enabled by default for Vitis HLS.
```
compiler.
deadlockDetection

```
Type: Boolean
Default Value:
FALSE
```
```
Enables detection of kernel deadlocks during the simulation run as part of
hardware emulation. The tool posts an Error message to the console and
the log file when the application is deadlocked:
```
```
// ERROR!!! DEADLOCK DETECTED at 42979000 ns!
SIMULATION WILL BE STOPPED! //
```
```
The message is repeated until the deadlock is terminated. You must
manually terminate the application to end the deadlock condition.
```
```
Tip: When deadlocks are encountered during simulation, you can open
the kernel code in Vitis HLS for additional deadlock detection and debug
capability.
Applies to:
```
```
v++ --compile
Vitis HLS
config_export
```
compiler.emulationMode=
<mode>

```
Type: String
func | rtl
```
```
Indicates that the kernel should be compiled as RTL code for use in
hardware emulation and hardware design, or as a C functional model with a
SystemC wrapper for use in hardware emulation.
This option applies to v++ --compile. The default is to compile the kernel
as RTL code.
```
compiler.enableIncrHwEmuType: Boolean
Default Value:
FALSE

```
Use to enable incremental compilation of the hardware emulation xclbin
when there are minor changes made to the platform. This enables a quick
rebuild of the device binary for hardware emulation when the platform has
been updated.
Applies to:
```
```
v++ --link
vpl.impl
```
compiler.
errorOnHoldViolation

```
Type: Boolean
Default Value:
TRUE
```
```
After the last step of Vivado implementation, during timing analysis check,
and clock scaling if needed. If hold violations are found, v++ quits and
returns an error by default, and does not generate an xclbin. This
parameter lets you over ride the default behavior.
Applies to:
```
```
v++ --link
vpl.impl
```

#### Displayed in the footer

**Parameter Name Valid Values Description**

compiler.
interfaceRdBurstLen

```
Type: Int Range
Default Value: 0
```
```
Specifies the expected length of AXI read bursts on the kernel AXI interface.
This is used with option compiler.interfaceRdOutstanding to
determine the hardware buffer sizes. Values are 1 through 256.
Applies to:
```
```
v++ --compile
Vitis HLS
config_interface
```
compiler.
interfaceWrBurstLen

```
Type: Int Range
Default Value: 0
```
```
Specifies the expected length of AXI write bursts on the kernel AXI
interface. This is used with option
compiler.interfaceWrOutstanding to determine the hardware buffer
sizes. Values are 1 through 256.
Applies to:
```
```
v++ --compile
Vitis HLS
config_interface
```
compiler.
interfaceRdOutstanding

```
Type: Int Range
Default Value: 0
```
```
Specifies how many outstanding reads to buffer are on the kernel AXI
interface. Values are 1 through 256.
Applies to:
```
```
v++ --compile
Vitis HLS
config_interface
```
compiler.
interfaceWrOutstanding

```
Type: Int Range
Default Value: 0
```
```
Specifies how many outstanding writes to buffer are on the kernel AXI
interface. Values are 1 through 256.
Applies to:
```
```
v++ --compile
Vitis HLS
config_interface
```
compiler.skipTimingCheckAndFrequencyScalingType: Boolean
Default Value:
FALSE

```
This parameter causes the Vivado tool to skip the timing check and optional
clock frequency scaling that occurs after the last step of implementation
process, which is either route_design or post-route phys_opt_design.
Applies to:
```
```
v++ --link
vpl.impl
```
compiler.userPreCreateProjectTcl

```
Type: String
Default Value:
<empty>
```
```
Specifies a Tcl script to run before creating the Vivado project in the Vitis
build process.
Applies to:
```
```
v++ --link
vpl.create_project
```
compiler.userPreSysLinkOverlayTcl

```
Type: String
Default Value:
<empty>
```
```
Specifies a Tcl script to run after opening the Vivado IP integrator block
design, before running the compiler-generated dr.bd.tcl script in the Vitis
build process.
Applies to:
```

#### Displayed in the footer

**Parameter Name Valid Values Description**

```
v++ --link
vpl.create_bd
```
compiler.userPostSysLinkOverlayTcl

```
Type: String
Default Value:
<empty>
```
```
Specifies a Tcl script to run after running the compiler-generated dr.bd.tcl
script.
Applies to:
```
```
v++ --link
vpl.update_bd
```
compiler.userPostDebugProfileOverlayTcl

```
Type: String
Default Value:
<empty>
```
```
Specifies a Tcl script to run after debug profile overlay insertion in Vivado IP
integrator block design in the vpl.update_bd step.
Applies to:
```
```
v++ --link
vpl.updated_bd
```
compiler.
worstNegativeSlack

```
Type: Float
Default Value: 0
```
```
During timing analysis check, this specifies the worst acceptable negative
slack for the design, specified in nanoseconds (ns). When negative slack
exceeds the specified value, the tool might try to scale the clock frequency
to achieve timing results. This specifies an acceptable negative slack value
instead of zero slack.
Applies to:
```
```
v++ --link
vpl.impl
```
compiler.
xclDataflowFifoDepth

```
Type: Int
Default Value: -1
```
```
Specifies the depth of FIFOs used in kernel data flow region.
Applies to:
```
```
v++ --compile
Vitis HLS
config_dateflow
```
hw_emu.aie_shim_sol_pathType: String
Default Value:
<empty>

```
For use by Versal platforms, this option specifies the path to the AI Engine
Interface Tile constraints file which is generated by the aiecompiler.
Used during simulation, compilation, and elaboration, the file provides a
logical mapping to the physical interface. This is needed for third-party
simulators like Mentor Graphics Questa Advanced Simulator or Cadence
Xcelium Logic Simulation.
```
hw_emu. compiledLibs Type: String
Default Value:
<empty>

```
Uses mentioned clibs for the specified simulator.
Applies to Hardware Emulation and Debug.
```
hw_emu. debugMode wdb
Default Value: wdb

```
The default value is WDB and runs simulation in waveform mode.
This option only works in combination with the -g or --debug options.
Applies to Hardware Emulation and Debug.
```
hw_emu.
enableProtocolChecker

```
Type: Boolean
Default Value:
FALSE
```
```
Enables the lightweight AXI protocol checker (lapc) during HW emulation.
This is used to confirm the accuracy of any AXI interfaces in the design.
Applies to Hardware Emulation and Debug.
```
hw_emu.json_device_file_pathType: String
Default Value:
<empty>

```
For use by Versal platforms, this option specifies the path to the AI Engine
JSON Device file located in the Vitis software installation area.
Used during simulation, compilation, and elaboration, the file specifies the
size of the AI Engine array. This is needed for third-party simulators like
```

#### Displayed in the footer

##### ★

```
Parameter Name Valid Values Description
Mentor Graphics Questa Advanced Simulator or Cadence Xcelium Logic
Simulation.
```
```
hw_emu.platformPath Type: String
Default Value:
<empty>
```
```
Specifies the path to the custom platform directory. The <platformPath>
directory should meet the following requirements to be used in platform
creation:
```
```
The directory should contain a subdirectory called ip_repo.
The directory should contain a subdirectory called scripts and this
scripts directory should contain a hw_em_util.tcl file. The
hw_em_util.tcl file should have the following two procedures defined in
it:
hw_em_util::add_base_platform
hw_em_util::generate_simulation_scripts_and_compile
```
```
Applies to Hardware Emulation and Debug.
```
```
hw_emu.post_sim_settingsType: String Specifies the path to a Tcl script that is used to configure the settings of the
Vivado simulator prior to running hardware emulation. This script is run after
the default configuration of the tool, but prior to launching simulation. You
can use the Tcl script to override specific settings, or to custom configure
the simulator as needed.
Applies to Hardware Emulation and Debug.
```
```
hw_emu.reduceHwEmuCompileTimeType: Boolean
Default Value:
FALSE
```
```
Move the generation of the top-level block design into the Generate Targets
step of v++ --link.
Applies to Hardware Emulation and Debug.
```
```
hw_emu. scDebugLevel none | waveform |
log |
waveform_and_log
Default Value:
waveform_and_log
```
```
Sets the TLM transaction debug level of the Vivado logic simulator (xsim).
```
```
NONE to disable TLM debug
LOG to dump TLM transaction log info into report file
WAVEFORM for enabling the TLM transaction waveform view
WAVEFORM_AND_LOG for both the Log Messages and Waveform
view
```
```
Applies to Hardware Emulation and Debug.
```
```
hw_emu.simulator XSIM | QUESTA
Default Value:
XSIM
```
```
Uses the specified simulator for the hardware emulation run.
Applies to Hardware Emulation and Debug.
```
```
package.enableEdfHwEmu Default values: 1 or
0
```
```
For designs that use base platforms prepared for Embedded Development
Framework (EDF), but does not have EDF as default flow (like VRK160
boards), you need to set --advanced.param
package.enableEdfHwEmu=1 when packaging. This switch uses QEMU
file sets and packaging outputs suitable for EDF. For VEK385, EDF flow is
default and you do not need to set this additional parameter.
```
For example:

```
--advanced.param compiler.addOutputTypes="hw_export"
```
```
Tip: This option can be specified in a configuration file under the [advanced] section head using the following format:
```
```
[advanced]
param=compiler.addOutputTypes="hw_export"
```

#### Displayed in the footer

###### ✎

### --advanced.prop

```
--advanced.prop <arg>
```
Specifies advanced kernel or solution properties for kernel compilation where <arg> is one of the values described the following table.

**Table: Prop Options**

```
Property Name Valid Values Description
```
```
kernel.<kernel_name>. kernel_flags Type: String
Default Value:
<empty>
```
```
Sets specific compile flags on the kernel
<kernel_name>.
```
```
solution. kernel_compiler_margin Type: Float
Default Value: 12.5% of
the kernel clock period.
```
```
The clock margin (in ns) for the kernel. This value
is subtracted from the kernel clock period prior to
synthesis to provide some margin for place and
route delays.
```
### --advanced.misc

```
--advanced.misc <arg>
```
Specifies advanced tool directives for kernel compilation.

###### --clock Options

The --clock options are used by the Vitis tool to assign the clock frequency during the compilation of the AI Engine graph, HLS
kernels and linking of the design.
For clock directives supported by Vitis compilation, the Vitis tool compiles the AI Engine graph and HLS components. There are
different modes to compile AI Engine graph and HLS components. The AI Engine runs at a single rate, although the compiler supports
directives to specify expected PLIO peer clock frequencies. If clock directives are not used, the Vitis tool decides the clock frequency
based on whether --part or --platform is used in the command.

**Table: Clock Directives for AI Engine Graph Compilation**

```
Mode Design Clock Directive Vitis Tool Consideration
```
```
v++ -c --mode aie --platform No Directive Platform default clock
```
```
v++ -c --mode aie --part No Directive 0.25 x (AI Engine PLL
Frequency)
```
```
v++ -c --mode aie --platform OR
--part
```
```
--pl-freq=200
--freqhz=200000000
--freqhz=200MHz
```
```
Compiles at 200 MHz
```
**Note:** The value of --freqhz supports either no units (in which case, Vitis considers the unit to be Hz) or MHz. The unit MHz is
case insensitive. The frequency should not be a hertz fractional value. For example, --freqhz=312.005MHz is supported whereas --
freqhz=312.0000005MHz is not supported. There must be no space between the value and the units (in MHz). freqhz is applicable
to all Vitis compilation modes and linking.
The following are the commands with different directives in CLI mode:

```
v++ -c --mode aie --platform <pfm.xpfm> --config <aie.cfg> --freqhz=200000000 ./aie_graph.cpp
v++ -c --mode aie --platform <pfm.xpfm> --config <aie.cfg> --freqhz=200MHz ./aie_graph.cpp
v++ -c --mode aie --platform <pfm.xpfm> --config <aie.cfg> --aie.pl-freq=200 ./aie_grapch.cpp
```
Different directives can be given through the configuration file as well:


#### Displayed in the footer

###### ✎

###### ✎

```
freqhz=200000000
```
```
freqhz=200MHz
```
The directive --aie.pl-freq (the same is applicable to --freqhz) can be used to address other PLIO design scenarios as shown
below:

```
To compile all AI Engine PLIO at 200MHz (Default): --aie.pl-freq=200
To compile specific PLIO at 200MHz: --aie.pl-freq=<PLIO_PORT_NAME>:<FREQ>
To compile multiple graphs at 200MHz: --aie.pl-freq=<AIE_GRAPH_NAME>.<PLIO_PORT_NAME>:<FREQ>
```
```
Note: --aie.pl-freq is declared in MHz, while --freqhz is declared in Hz.
```
**Table: Clocking Directives for HLS Compilation**

```
Mode Design Clock Directive Vitis Tool Consideration
```
```
v++ -c --mode hls --platform No Directive Platform default clock
```
```
v++ -c --mode hls --part No Directive Default clock=100 MHz
```
```
v++ -c --mode hls --platform OR
--part
```
```
--hls.clock=200
--hls.clock=0.5ns
--freqhz=200000000
--freqhz=200MHz
```
```
Compiles HLS kernel at 200
MHz
```
The following are the commands with different directives in CLI mode:

```
v++ -c --mode hls --platform <pfm.xpfm> --config <hls.cfg> --freqhz=200000000 --work_dir
<work_dir_path>
```
```
v++ -c --mode hls --platform <pfm.xpfm> --config <hls.cfg> --freqhz=200MHz --work_dir
<work_dir_path>
```
```
v++ -c --mode hls --platform <pfm.xpfm> --config <hls.cfg> --hls.clock=200 --work_dir
<work_dir_path>
v++ -c --mode hls --platform <pfm.xpfm> --config <hls.cfg> --hls.clock=5ns --work_dir
<work_dir_path>
```
Different directives can also be given via the configuration file:

```
freqhz=200000000
```
```
freqhz=200MHz
```
```
[hls]
clock=200
```
```
[hls]
clock=5ns
```
**Note:** Clocking directive support for v++ -c --mode hls is analogous to v++ -c -k.
For clock directives supported for Vitis linking, the Vitis tool links AI Engine graph, HLS components, and hardware configuration
(platform/part) using a configuration file. During the linking, clock directives are used to connect HLS components to the requested
clock frequency. In v++ linking, how the Vitis tool connects the requested clock frequency to HLS components and how the v++ linker
handles the scenario depends on the clock directives used. Refer to Managing Clock Frequencies in the _Data Center Acceleration
using Vitis_ (UG1700) for more information about the v++ linking behavior for the clocking directives discussed below.

**Table: Clocking Directives for v++ Linking:**

```
Mode Design Clock Directive Vitis Tool Consideration
```

#### Displayed in the footer

```
Mode Design Clock Directive Vitis Tool Consideration
```
```
v++ -l --platform No Directive Platform default clock
```
```
v++ -l --part No Directive For Versal, default clock is 300
MHz. Not supported for other
devices.
```
```
v++ -l --platform
--part
```
```
--clock.freqHz=200000000:
<cu_0>[.<clk_pin>]
--freqhz=200000000:<cu_0>[.
<clk_pin>]
--freqhz=200MHz:<cu_0>[.
<clk_pin>]
--clock.id=<id_value>:<cu_0>
[.<clk_pin>]
```
```
For freqhz/clock.freqHz:
Connect HLS kernel to 200
MHz
For clock.id: Connect HLS
kernel to clock source
<id_value of 200 MHz>
```
The following are the commands with directives in CLI mode:

```
v++ -l --platform <pfm.xpfm> ./libadf.a ./<kernel_name.xo> --config <system.cfg> --
freqhz=200000000:mm2s:aclk -o <fixed.xsa>
```
```
v++ -l --platform <pfm.xpfm> ./libadf.a ./<kernel_name.xo> --config <system.cfg> --
freqhz=200MHz:mm2s.aclk -o <fixed.xsa>
```
```
v++ -l --platform <pfm.xpfm> ./libadf.a ./<kernel_name.xo> --config <system.cfg> --
clock.freqHz=200000000:mm2s.aclk -o <fixed.xsa>
```
```
v++ -l --platform <pfm.xpfm> ./libadf.a ./<kernel_name.xo> --config <system.cfg> --clock.id=
<id_value> -o <fixed.xsa>
```
Different directives can be given through configuration file:

```
freqhz=200000000:mm2s.aclk
freqhz=200MHz:mm2s.aclk
```
```
[clock]
freqHz=200000000:mm2s.aclk
```
There are two ways to connect the kernel to the specified frequency in the v++ link phase. Clocks available in the platform can be
considered either as a clock frequency or as a clock source by the Vitis tool, which depends on the directive. The clock directive --
freqhz or --clock.freqHz can be used to connect the clock as a clock frequency and --clock.id can be used to connect the
clock as a clock source:

```
--clock.freqHz or --freqhz: These both directives work in a similar way and do not depend on platform clocking. You can
use these directives to connect a platform clock frequency to the kernel:
If a platform has multiple clocks available at the requested frequency using --freqhz, the Vitis tool selects any one of the
platform clock by considering the clock.tolerance value (default being 5%).
If the platform has multiple clock frequencies available at the requested clock frequency, and you want the Vitis tool to pick a
particular clock, you must use the directive --clock.id in place of --freqhz.
If a platform does not have a match for the requested frequency, the Vitis tool generates the requested clock frequency using
the available platform fixed clock.
--clock.id: This option can be used to connect a particular platform clock as a clock source to the kernel. The v++ linker can
add IPs such as FIFO, data width converter, and clock converter, depends on the Vivado design implemented.
```
The PFM.CLOCK status attribute specifies how the v++ linker treats the clock. It can either be fixed or fixed_non_ref (scalable
clocks are supported only on legacy Alveo platforms). The linker does not employ a fixed_non_ref clock as input reference for
generated clocks. To set the clock attribute, open the Vivado design, go to Platform Setup, under the Clock section set the status
property of the clock as shown below:

**Figure: Platform Setup for Clock**


#### Displayed in the footer

###### ✎

###### ✎

**Table: PFM.CLK Status Options**

```
Status Value Description
```
```
Scalable Scalable clocks used by XRT. Only supported in legacy Alveo
designs.
```
```
Fixed Used as a reference clock to connect to IP. Used in Embedded
design.
```
```
fixed_non_ref Cannot be used as a reference clock; can be used to connect
IPs in embedded design.
```
**Note:** Reference clocks are clocks which the Vitis tool uses (as a clock input) to generate other clocks using MMCM.
For example, suppose a clocking requirement is to generate the output clock with a divider setting of 1, 2, 4, and 8. It is recommended
to use the primitive MBUFGCE in place of BUFG to reduce the clock skew between synchronous clock outputs. The output clocks of
MBUFGCE cannot be used as a reference clock. Thus, you can set the attribute status=fixed_non_ref so that the Vitis tool cannot
use these clocks (output of MBUFGCE) as reference clocks.
**Note:** You can determine the available clock IDs for a target platform using the platforminfo utility as described in platforminfo
Utility. The platforminfo report includes the clock status of all the platform clocks.
To determine the fixed clocks available on the platform xilinx_vck190_base_bdc_202420_1.xpfm:
Go to plaftorminfo ../ xilinx_vck190_base_bdc_202420_1.xpfm and refer to the “Clock Information” section in the report:

```
=================
Clock Information
=================
Default Clock Index: 2
Clock Index: 0
Status: Fixed
Frequency: 104.166666
Clock Index: 1
Status: Fixed
Frequency: 156.250000
Clock Index: 2
Status: Fixed
Frequency: 312.500000
Clock Index: 3
Status: Fixed
Frequency: 78.125000
Clock Index: 4
Status: Fixed
Frequency: 208.333333
Clock Index: 5
Status: Fixed
Frequency: 416.666666
Clock Index: 6
Status: Fixed
Frequency: 625.000000
```

#### Displayed in the footer

###### !!

##### ★

###### ✎

### --clock.tolerance

```
--clock.tolerance <arg>
```
Specifies a clock tolerance. When specifying --clock.freqHz, you can also specify the tolerance with either a value or percentage of
the clock frequency. This updates the timing constraints to reflect the accepted tolerance. <arg> is specified as <tolerance>:
<cu_0>[.<clk_pin_0>][,<cu_n>[.<clk_pin_n>]]

**<tolerance>**
Can be specified either as a whole number, indicating the clock.freqHz ± the specified tolerance value, or as a
percentage of the clock frequency specified as a decimal value.
<cu_0>[.<clk_pin_0>][,<cu_n>[.<clk_pin_n>]]: Applies the defined clock tolerance to the specified CUs, and
optionally to the specified clock pin on the CU.

**<cu_0>[.<clk_pin_0>][,<cu_n>[.<clk_pin_n>]]**
Applies the defined clock tolerance to the specified CUs, and optionally to the specified clock pin on the CU.

**Important:** The default clock tolerance is 5% of the specified frequency when this option is not specified.
For example:

```
v++ --link --clock.tolerance 0.10:vadd_1,vadd_3
```
```
Tip: This option can be specified in a configuration file under the [clock] section head using the following format:
```
```
[clock]
tolerance=0.10:vadd_1,vadd_3
```
###### --connectivity Options

As discussed in Linking the System in the _Embedded Design Development Using Vitis_ (UG1701), there are a number of --
connectivity.XXX options that let you define the interconnection network of .xo kernels, AI Engine PLIO and GMIO interfaces, and
platform NoC, memories, and stream interfaces. These commands are an integral part of the build process, critical to the definition and
construction of the application.

### --connectivity.region

```
--connectivity.region <arg>
```
Use this option to specify an affinity for a PL kernel to a PFM.REGION declared in the extensible .xsa hardware platform. The typical
use of a PFM.REGION declaration is to match a Pblock defined in the hardware platform. An example is to demarcate a Super Logic
Region (SLR) within a Stacked Silicon Interconnect (SSI) device such as an xcvp2802 SoC.
You must use a separate --connectivity.region option to map each compute unit to a particular region. The build process
automatically places any compute unit that you do not explicitly map to a region through the --connectivity.region option in an
available region.
**Recommended:** AMD recommends that you specify region labels when working with SSI devices as this provides the greatest
placement control and helps ensure that the placer co-locates kernels with their associated resources in the same SLR. However, you
can also rely on automatic placement if regional constraints are not critical.
Valid values include:

```
<region_label>:<cu_name>
```
Where:

```
<region_label> is the string identifier that you specify in the platform's PFM.REGION attribute for the target SLR or logical
region.
<cu_name> is the name of the compute unit that you specify in the --connectivity.nk option. This defaults to
<kernel_name>_1 unless you specify a different name.
```
In the linker configuration file, you specify the region assignment as follows:


#### Displayed in the footer

##### ★

##### ★

```
[connectivity]
region=<region_label>:<cu_name_1>,<cu_name_2>
```
### --connectivity.nk

```
--connectivity.nk <arg>
```
Where <arg> is specified as <kernel_name>:#:<cu_name1>,<cu_name2>,...<cu_name#>.
This instantiates the specified number of instances ('compute units') of the specified kernel (kernel_name) in the generated FPGA
binary (.xclbin) file during the linking process. The cu_name is optional. If the cu_name is not specified, the instances of the kernel
are simply numbered: kernel_name_1, kernel_name_2, and so forth. By default, the Vitis compiler instantiates one compute unit for
each kernel.
For example:

```
v++ --link --connectivity.nk vadd:3:vadd_A,vadd_B,vadd_C
```
```
Tip: This option can be specified in a configuration file under the [connectivity] section head using the following format:
```
```
[connectivity]
nk=vadd:3:vadd_A,vadd_B,vadd_C
```
### --connectivity.stream_connect

```
--connectivity.sc <arg>
```
Create a streaming connection between two compute units through their AXI4-Stream interfaces. Use a separate --
connectivity.sc option for each streaming interface connection. The order of connection must be from a streaming output port of
the first kernel to a streaming input port of the second kernel. Valid values include:

```
<cu_name>.<streaming_output_port>:<cu_name>.<streaming_input_port>[:<fifo_depth>]
```
Where:

```
<cu_name> is the compute unit name specified in the --connectivity.nk option. Generally this is <kernel_name>_1 unless
a different name was specified.
<streaming_output_port>/<streaming_input_port> is the function argument for the compute unit port that is declared
as an AXI4-Stream.
[:<fifo_depth>] inserts a FIFO of the specified depth between the two streaming ports to prevent stalls. The value is specified
as an integer.
```
For example, to connect the AXI4-Stream port s_out of the compute unit mem_read_1 to AXI4-Stream port s_in of the compute unit
increment_1, use the following:

```
--connectivity.sc mem_read_1.s_out:increment_1.s_in
```
```
Tip: This option can be specified in a configuration file under the [connectivity] section head using the following format:
```
```
[connectivity]
sc=mem_read_1.s_out:increment_1.s_in
```
The inclusion of the optional <fifo_depth> value lets the v++ linker add a FIFO between the two kernels to help prevent stalls. This uses
BRAM resources from the device when specified, but eliminates the need to update the HLS kernel to contain FIFOs. The tool also
instantiates a Clock Converter (CDC) or Datawidth Converter (DWC) IP if the connections have different clocks, or different bus widths.


#### Displayed in the footer

##### ★

##### ★

### --connectivity.system_port

```
--connectivity.sp <arg>
```
Use this option to specify the assignment of kernel arguments to system ports within the platform. A primary use case for this option is
to connect kernel arguments to specific memory resources. A separate --connectivity.sp option is required to map each argument
of a kernel to a particular memory resource. Any argument not explicitly mapped to a memory resource through the --
connectivity.sp option is automatically connected to an available memory resource during the build process.
Valid values include:

```
<cu_name>.<kernel_argument_name>:<sptag[min:max]>
```
Where:

```
<cu_name> is the name of the compute unit as specified in the --connectivity.nk option. Generally this is
<kernel_name>_1 unless a different name was specified.
<kernel_argument_name> is the name of the function argument for the kernel, or the compute unit interface port.
<sptag> represents a system port tag, such as for memory controller interface names from the target platform. Valid <sptag>
names include DDR, PLRAM, and HBM.
[min:max] enables the use of a range of memory, such as DDR[0:2]. A single index is also supported: DDR[2].
```
**Tip:** The supported <sptag> and range of memory resources for a target platform can be obtained using the platforminfo
command. Refer to platforminfo Utility for more information.
The following example maps the input argument of an AI Engine GMIO to interleaved LPDDR C2_C3 on the vek385_base platform:

```
v++ --link -f vek385_base --connectivity.sp ai_engine_0:out_gmio:LPDDR23
```
```
Tip: This option can be specified in a configuration file under the [connectivity] section head using the following format:
```
```
[connectivity]
sp=vadd_1.A:DDR[0:3]
sp=vadd_1.B:HBM[0:31]
sp=vadd_1.C:PLRAM[2]
```
### --connectivity.noc.connect

```
--connectivity.noc.connect <arg>
```
Where <arg> is in the form of <compute_unit_name>.<kernel_interface_name>:<noc interface>, and specifies a
connection between the PL kernel interface and the Versal NoC. Valid values are internal memory controllers, or master interfaces on
the Versal NoC cell.
The Vitis compiler estimates kernel bandwidth requirements based on NoC connectivity and M_AXI properties (datawidth * clock freq)
across the dynamic region, and automatically sets NoC configuration settings for read and write bandwidth, scaling as needed to avoid
exceeding the available bandwidth.
For example:

```
[connectivity]
noc.read_bw=mm2s.M_AXI:2000.16
noc.write_bw=mm2s.M_AXI:2010.16
noc.connect=mm2s.M_AXI:M00_INI
```
### --connectivity.noc.read_bw

```
--connectivity.noc.read_bw <arg>
```
Where <arg> is in the form <compute_unit_name>.<kernel_interface_name>:<Bandwidth>.<Avg_burst_length>, and
specifies both the bandwidth and burst length of the connection. The bandwidth is specified in MB/s.


#### Displayed in the footer

##### ★

This option specifies expected read traffic characteristics on M_AXI interfaces to let you override the automatic Versal NoC
configuration.

### --connectivity.noc.write_bw

```
--connectivity.noc.write_bw <arg>
```
Where <arg> is in the form <compute_unit_name>.<kernel_interface_name>:<Bandwidth>.<Avg_burst_length>, and
specifies both the bandwidth and burst length of the connection. The bandwidth is specified in MB/s.
This option specifies expected write traffic characteristics on M_AXI interfaces to let you override the automatic Versal NoC
configuration.

### --connectivity.connect

```
--connectivity.connect <X:Y>
```
This option can be used to make connections through the Vivado IP integrator, but v++ does not perform any error checking on the
specified connections. Use this to specify general connections between kernels and non-AXI elements of the target platform, such as
connections to GT ports.
The X and Y connections must be specified as arguments compatible with either the IP integrator connect_bd_net or
connect_bd_intf_net commands. The specific format of <X:Y> is:

```
src/hierarchy_name/cell_name/pin_name:dst/hierarchy_name/cell_name/pin_name
```
These cannot include connections between AXI4-Stream interfaces which require the use of --conectivity.sc, or M_AXI interfaces
which require the use of --connectivity.sp as described above.

```
Tip: This option can be specified in a configuration file under the [connectivity] section head using the following format:
```
```
[connectivity]
connect=<X:Y>
```
###### --debug Options

This option enables debug IP core insertion in the device binary (.xclbin) for hardware debugging. This option lets you specify the type
of debug core to add, and which compute unit and interfaces to monitor with ChipScope™ as described in Debugging During Hardware
Execution in the _Data Center Acceleration using Vitis_ (UG1700). The --debug.xxx options lets you attach AXI protocol checkers and
System ILA cores at the interfaces to kernels or specific compute units (CUs) for debugging and performance monitoring purposes:

```
The System Integrated Logic Analyzer (ILA) provides transaction level visibility into an accelerated kernel or function running on
hardware. AXI traffic of interest can also be captured and viewed using the System ILA core.
The AXI Protocol Checker debug core is designed to monitor AXI interfaces on the accelerated kernel. When attached to an
interface of a CU, the AXI Protocol Checker actively checks for protocol violations and provides an indication of which violation
occurred.
```
The --debug.xxx commands can be specified in a configuration file under the [debug] section head using the following format as
an example:

```
[debug]
protocol=all:all # Protocol analyzers on all CUs
protocol=cu2:port3 # Protocol analyzer on port3 of cu2
chipscope=cu2 # ILA on cu2
```
The various options of --debug include the following:


#### Displayed in the footer

###### !!

### --debug.aie.chipscope

```
--debug.aie.chipscope <interface_name> | <adf_graph_arg_name>
```
Enables hardware debug for the Versal AI Engine through ChipScope. The <interface_name> argument applies to non-PL kernel
interfaces such as AI Engine PLIO interfaces, or AXIS interfaces. The <adf_graph_arg_name> specifies arguments of the graph.

### --debug.chipscope

```
--debug.chipscope <cu_name>[:<interface_name>]
```
Adds the System Integrated Logic Analyzer debug core to the specified CUs in the design.
**Important:** The --debug.chipscope option requires the <cu_name> to be specified and does not accept the keyword all. You
can optionally specify an <interface_name>.
For example, the following command adds an ILA core to the vadd_1 CU:

```
v++ --link --debug.chipscope vadd_1
```
### --debug.list_ports

Shows a list of valid compute units and port combinations in the current design. This is informational to help you with crafting a
command line or config file for the --debug command.
This option needs to be specified during linking, but does not run the linking process. The required elements of the command line are
shown in the following example, which returns the available ports when linking the specified kernels with the listed platform:

```
v++ --platform <platform> --link --debug.list_ports <kernel.xo>
```
### --debug.protocol

```
--debug.protocol all|<cu_name>[:<interface_name>]
```
Adds the AXI Protocol Checker debug core to the design. This can be specified with the keyword all, or the <cu_name> and optional
<interface_name> to add the protocol checker to the specified CU and interface.
For example:

```
v++ --link --debug.protocol all
```
###### --drc Options

This option lets you manage the Design Rule Check (DRC) messages returned by the Vivado Design Suite during implementation of
the design. Messages can be disabled or enabled, reduced in severity, or waived to allow the design to proceed.

### --drc.disable

```
--drc.disable <arg>
```
Lets you disable the DRC message specified by the DRC ID. Disabled DRCs are not checked.
For example:

```
v++ --link --drc.disable TIMING-18
```
### --drc.enable

```
--drc.enable <arg>
```

#### Displayed in the footer

Lets you enable the DRC message specified by the DRC ID. DRC checks that have been disabled can be re-enabled as needed.
For example:

```
v++ --link --drc.enable TIMING-18
```
### --drc.severity

```
--drc.severity <arg>
```
Change the severity of a DRC check. For example, instead of a {CRITICAL WARNING} a violation is only reported as a WARNING. The
DRC must be specified by ID, and the new severity to apply.
For example:

```
v++ --link --drc.severity TIMING-18:Warning
```
### --drc.waive

```
--drc.waive <arg>
```
Lets you waive the specified DRC ID. This lets the Vivado tool proceed through implemntation, ignoring DRCs that might otherwise
prevent completion of the design. A waived rule is still checked, but it is marked as waived.
For example:

```
v++ --link --drc.waive TIMING-18
```
###### --linkhook Options

The --linkhook.XXX options described below are used to specify Tcl scripts to run at specific steps during the Vitis linking process.
Valid steps can be determined using the --linkhook.list_steps command as described below.

### --linkhook.custom

```
--linkhook.custom <step name, path to script file>
```
Specifies a Tcl script to execute at a predefined point in the build process. The path to specify the script can be an absolute path, or
partial path relative to the build directory.
For example, the following command runs the specified Tcl script before the SysLink step in the build:

```
v++ -l --linkhook.custom preSysLink,./runScript.tcl
```
### -linkhook.do_first

```
--linkhook.do_first <step name, path to script file>
```
Specifies a Tcl script to execute before the given step name. The path to specify the script can be an absolute path, or partial path
relative to the build directory.
For example, the following command runs the specified Tcl script before the place_design step in the build:

```
v++ -l --linkhook.do_first vpl.impl.place_design,runScript.tcl
```
### -linkhook.do_last

```
--linkhook.do_last <step name, path to script file>
```

#### Displayed in the footer

##### ★

Specifies a Tcl script to execute immediately after the given step completes. The path to specify the script can be an absolute path, or
partial path relative to the build directory.
For example, the following command runs the specified Tcl script after the place_design step in the build:

```
v++ -l --linkhook.do_last vpl.impl.place_design,runScript.tcl
```
### -linkhook.list_steps

```
--linkhook.list_steps
```
List default and optional build steps that support script hooks for a specified target. This command requires the --target to be
specified in addition to the --link option.
For example:

```
v++ --target hw -l --linkhook.list_steps
```
The command returns both default steps that are always enabled during the build process, and optional steps that you can enable if
needed. For directions on enabling optional steps, refer to the following:

```
For data center acceleration: Managing Vivado Synthesis, Implementation, and Timing Closure in the Data Center Acceleration
using Vitis (UG1700)
For embedded design development: Managing Vivado Synthesis, Implementation, and Timing Closure in the Embedded Design
Development Using Vitis (UG1701)
```
###### --package Options

### Introduction

As described in Packaging for Vitis Flow in the _Embedded Design Development Using Vitis_ (UG1701), the v++ --package, or -p
step, generates and packages the final product at the end of the v++ compile and link build process. This is a required step for all
embedded platforms, including Versal devices, AI Engine, and AMD Zynq™ UltraScale+™ MPSoC devices.
The syntax of the --package command for Versal platforms is as follows:

```
v++ --package --platform fixed.xsa -o output.xclbin --package.<other options>
```
**Tip:** Package command options can be specified in a configuration file for use with the --config option, as discussed in the Vitis
Compiler Configuration File.
For non-Versal platforms the syntax for the --package command is:

```
v++ --package -t < hw_emu | hw> --platform <platform> input.xclbin \
[ -o output.xclbin --package.<options> ]
```
The various options to specify as --package.<options> as shown in the syntax above include the following:

### --package.aie_metadata_only

```
--package.aie_metadata_only {Lij: i=1,...,P, any j}
```
Where {Lij: i=1,...,P, any j} specifies the libadf.a library files for AIE partitions to include metadata only in the .xclbin

file for independent control against a loaded image. The Lij represents libraries. The index i represents the ith partition and j can

be any value. The default behavior is to include full AIE partition data and metadata.
Additional Outputs:

```
aie.pdi
aie.bif
```
Use Case: Creates a metadata-only .xclbin file for independent control against a loaded image. This allows runtime control of AIE
partitions without including the full partition data in the .xclbin file.


#### Displayed in the footer

###### ✎

###### !!

Example:

```
v++ -p -f fixed.xsa --package.aie_metadata_only {Lij: i=1,...,P, any j} -o aie-meta.xclbin
```
### --package.aie_overlay

```
--package.aie_overlay {Lij}
```
Where this option specifies that the package command should include the aie.pdi file for merged positional argument libadf.a files
and set the LOAD_PDI action mask. The {Lij} must represent adjacent partitions. The default behavior is to not create an AIE overlay.

Additional Outputs:

```
aie.pdi - Includes the PDI file for merged positional argument libadf.a files
aie.bif - Boot Image Format file
```
Important: The Lij libraries must represent adjacent partitions for this option to work correctly.

Use Case: Enables AIE overlay functionality, allowing users to load a pl.xclbin and then load/reload any number of aie.xclbin
overlays. This is particularly useful in DFX designs where AIE overlays can be dynamically loaded on top of a PL-only image.
Example:

```
v++ -p -f fixed.xsa {Lij} --package.aie_overlay -o aie.xclbin
```
```
Note: In DFX designs, AIE overlays are not permitted on top of full AIE+PL images because unloading tears down the .pdi image.
```
### --package.aie_resources_bin

```
--package.aie_resources_bin <arg>
```
Where <arg> specifies the AIE resources binary file.
**Important:** Detailed information about this option is not available in the current documentation. Consult the v++ command help or
additional Vitis documentation for complete details.

### --package.pl_metadata_only

```
--package.pl_metadata_only
```
Where this option specifies that only PL metadata should be included in the .xclbin file, required for PL hardware context for a loaded
image. The default behavior is to include full PL configuration data and metadata.
Use Case: Creates a PL metadata-only .xclbin file that is required for establishing a PL hardware context for a loaded image. This is
useful when the PL configuration has already been loaded and only the metadata is needed for runtime control.
Example:

```
v++ -p -f fixed.xsa --package.pl_metadata_only -o pl-meta.xclbin
```
Combined Use: This option can be combined with --package.aie_metadata_only to create a metadata-only .xclbin for runtime
control of both PL and AIE components of a loaded image:

```
v++ -p --package.pl_metadata_only --package.aie_metadata_only {Lij: i=1,...,P, any j} -o pl_aie-
metadata.xclbin
```
### --package.aie_debug_port

```
--package.aie_debug_port <arg>
```
Where <arg> specifies a TCP port in which the emulator listens for incoming connections from the debugger to debug Versal AI Engine
cores. The default port value is 10100.


#### Displayed in the footer

##### ★

For example:

```
v++ -l --package.aie_debug_port 1440
```
### --package.bl31_elf

```
--package.bl31_elf <arg>
```
Where <arg> specifies the absolute or relative path to Arm trusted FW ELF that executes on A72 #0 core. If this option is not specified,
then the Vitis compiler searches for the bl31 in the platform.
For example:

```
v++ -l --package.bl31_elf ./arm_trusted.elf
```
### --package.boot_mode

```
--package.boot_mode <arg>
```
Where <arg> specifies the <ospi | qspi | sd> boot mode used for running the application in emulation or on hardware. For
embedded platforms, the default boot mode is SD. Custom platforms boot mode can be configured to use QSPI or OSPI as
appropriate.

**Tip:** The xilinx_vck190_v202410_1 embedded base platform provided by AMD does not support the QSPI option.
For example:

```
v++ -l --package.boot_mode sd
```
### --package.defer_aie_run

```
--package.defer_aie_run
```
Where this option specifies that the Versal AI Engine cores are enabled by an embedded processor (PS) application. When not
specified, the tool generates CDO commands to enable the AI Engine cores during PDI load instead. By default this option is disabled,
or FALSE.
For example:

```
v++ -l --package.defer_aie_run
```
### --package.domain

```
--package.domain <arg>
```
Where <arg> specifies a domain name. If this option is not specified, then the Vitis compiler picks up the default domain from the
software platform (SPFM) file. For AI Engine designs, this should always be aiengine.
For example:

```
v++ -l --package.domain xrt
```
### --package.dtb

```
--package.dtb <arg>
```
Where <arg> specifies the absolute or relative path to device tree binary (DTB) used for loading Linux on the APU. If this options is not
specified, then Vitis compiler searches for the dtb in the platform.
For example:


#### Displayed in the footer

###### !!

```
v++ -l --package.dtb ./device_tree.image
```
### --package.enable_aie_debug

```
--package.enable_aie_debug
```
When enabled, the tool generates CDO commands to halt the AI Engine cores during the PDI load, forcing them into debug halt mode.
Once debugger connected, the user can debug the AI Engine cores step by step. By default this option is disabled, or FALSE.
For example:

```
v++ -l --package.enable_aie_debug
```
### --package.image_format

```
--package.image_format <arg>
```
Where <arg> specifies <ext4 | fat32> output image file format used on the SD card. For embedded platforms with a Linux
domain, the default image format is ext4. For all others, the image format is fat32.

```
ext4: Linux file system
fat32: Windows file system
```
**Important:** EXT4 format is not supported on Windows.
For example:

```
v++ -l --package.image_format fat32
```
### --package.kernel_image

```
--package.kernel_image <arg>
```
Where <arg> specifies the absolute or relative path to a Linux kernel image file. Overrides the existing image available in the platform.
The platform image file is available for download from xilinx.com. Refer to the Vitis Software Platform Installation in the _Vitis Software
Platform Release Notes_ (UG1742) for more information. If this option is not specified, then the Vitis compiler copies the Linux image
from the platform to the SD card folder.
For example:

```
v++ -l --package.kernel_image ./kernel_image
```
### --package.no_image

```
--package.no_image
```
Bypass SD card image creation. Valid for --package.boot_mode sd. By default this option is disabled, or FALSE.

### --package.out_dir

```
--package.out_dir <arg>
```
Where <arg> specifies the absolute or relative path to the output directory of the --package command. The default output directory is
the directory from which the Vitis compiler is launched.
For example:

```
v++ -l --package.out_dir ./out_dir
```

#### Displayed in the footer

##### ★

### --package.ps_debug_port

```
--package.ps_debug_port <arg>
```
Where <arg> specifies the TCP port where emulator listens for incoming connections from the debugger to debug PS cores.
For example:

```
v++ -l --package.debug_port 3200
```
### --package.ps_elf

```
--package.ps_elf <arg>
```
Where <arg> specifies <path_to_elf_file,core>.

```
path_to_elf_file: Specifies the ELF file for the PS core.
core: Indicates the PS core it should run on.
```
Used when a baremetal ELF file is running on a device processor core. This option specifies an ELF file and processor core pair to be
included in the boot image. The available processors for supported devices are listed below:

```
Versal processor core values include: a72-0, a72-1, a72-2, and a72-3.
Zynq UltraScale+ MPSoC processor core values include: a53-0, a53-1, a53-2, a53-3, r5-0, and r5-1.
Zynq 7000 processor core values include: a9-0 and a9-1.
```
**Tip:** Specify the option separately for each ELF/Core pair.
For example:

```
v++ -l --package.ps_elf a53_0.elf,a53-0 --package.ps_elf r5_0.elf,r5-0
```
### --package.rootfs

```
--package.rootfs <arg>
```
Where <arg> specifies the absolute or relative path to a processed Linux root file system file. The platform RootFS file is available for
download from Xilinx.com. Refer to the Vitis Software Platform Installation in the _Vitis Software Platform Release Notes_ (UG1742) for
more information. If this option is not specified, then the Vitis compiler picks up the default rootfs path from the software platform
(SPFM) file.
For example:

```
v++ -l --package.rootfs ./rootfs.ext4
```
### --package.sd_dir

```
--package.sd_dir <arg>
```
Where <arg> specifies a folder to package into the sd_card directory/image. The contents of the directory are copied to a sub-folder
of the sd_card folder.
For example:

```
v++ -l --package.sd_dir ./test_data
```
### --package.sd_file

```
--package.sd_file <arg>
```

#### Displayed in the footer

###### !!

Where <arg> specifies an ELF or other data file to package into the sd_card directory/image. This option can be used repeatedly to
specify multiple files to add to the sd_card. The .xclbin and libadf.a files are automatically copied to the out-dir or sd_card folder.
For example:

```
v++ -l --package.sd_file ./arm_trusted.elf
```
### --package.uboot

```
--package.uboot <arg>
```
Where <arg> specifies a path to U-Boot ELF file which overrides a platform U-Boot. If this option is not specified, then the Vitis
compiler searches for the uboot in the platform.
For example:

```
v++ -l --package.uboot ./uboot.elf
```
###### --profile Options

As discussed in Enabling Profiling in Your Application in the _Data Center Acceleration using Vitis_ (UG1700), there are a number of --
profile options that let you enable profiling of the application and kernel events during runtime execution. This option enables
capturing transaction details for data traffic between the kernel and host, kernel stalls, the execution times of kernels and compute units
(CUs), in addition to monitoring activity in Versal AI Engines.
**Important:** Using the --profile option in v++ also requires the addition of one of the profile or trace options in the xrt.ini file. Refer
to xrt.ini File for more information.
The --profile commands can be specified in a configuration file under the [profile] section head using the following format, for
example:

```
[profile]
data=all:all:all # Monitor data on all kernels and CUs
data=k1:all:all # Monitor data on all instances of kernel k1
data=k1:cu2:port3 # Specific CU master
data=k1:cu2:port3:counters # Specific CU master (counters only, no trace)
memory=all # Monitor transfers for all memories
memory=<sptag> # Monitor transfers for the specified memory
stall=all:all # Monitor stalls for all CUs of all kernels
stall=k1:cu2 # Stalls only for cu2
exec=all:all # Monitor execution times for all CUs
exec=k1:cu2 # Execution tims only for cu2
aie=all # Monitor all AIE streams
aie=DataIn1 # Monitor the specific input stream in the SDF graph
aie=M02_AXIS # Monitor specific stream interface
```
The various options of the command are described below:

### --profile.aie:<arg>

Enables profiling of AI Engine streams in adaptive data flow (ADF) applications, where <arg> is:

```
<ADF_graph_argument|pin name|all>
```
```
<ADF_graph_argument>: Specifies an argument name from the ADF graph application.
<pin_name>: Indicates a port on an AI Engine kernel.
<all>: Indicates monitoring all stream connections in the ADF application.
```
For example, to monitor the DataIn1 input stream use the following command:

```
v++ --link --profile.aie:DataIn1
```

#### Displayed in the footer

##### ★

###### !!

### --profile.aie_trace_offload:<arg>

Enables profiling of AI Engine streams in adaptive data flow (ADF) applications, where <arg> is HSDP, or high-speed debug port.

```
v++ --link --profile.aie_trace_offload:HSDP
```
The use of the HSDP is described in _Event Trace Offload using High Speed Debug Port_ in _AI Engine Tools and Flows User Guide_
(UG1076).

### --profile.data:<arg>

Enables monitoring of data ports through monitor IP that are added into the design. This option needs to be specified during linking.
Where <arg> is:

```
[<kernel_name>|all]:[<cu_name>|all]:[<interface_name>|all](:[counters|all])
```
```
[<kernel_name>|all] defines either a specific kernel to apply the command to. However, you can also specify the keyword
all to apply the monitoring to all existing kernels, compute units, and interfaces with a single option.
[<cu_name>|all] when <kernel_name> has been specified, you can also define a specific CU to apply the command to, or
indicate that it should be applied to all CUs for the kernel.
[<interface_name>|all] defines the specific interface on the kernel or CU to monitor for data activity, or monitor all
interfaces.
[<counters|all] is an optional argument, as it defaults to all when not specified. It allows restricting the information
gathering to only counters for larger designs, while all will include the collection of actual trace information.
```
For example, to assign the data profile to all CUs and interfaces of kernel k1 use the following command:

```
v++ --link --profile.data k1:all:all
```
### --profile.exec:<arg>

This option records the execution times of the kernel and provides minimum port data collection during the system run. This option
needs to be specified during linking.

**Tip:** The execution time of a kernel is collected by default when --profile.data or --profile.stall is specified. You can
specify --profile.exec for any CUs not covered by data or stall.
The syntax for exec profiling is:

```
[<kernel_name>|all]:[<cu_name>|all](:[counters|all])
```
For example, to profile to execution of cu2 for kernel k1 use the following command:

```
v++ --link --profile.exec:k1:cu2
```
### --profile.stall:<arg>

**Important:** This option must be specified during both v++ compilation and linking.
Adds stall monitoring logic to the device binary (.xclbin) which requires the addition of stall ports on the kernel interface. To facilitate
this, the stall option must be specified during both compilation and linking.
The syntax for stall profiling is:

```
[<kernel_name>|all]:[<cu_name>|all](:[counters|all])
```
For example, to monitor stalls of cu2 for kernel k1 use the following command:

```
v++ --compile -k k1 --profile.stall ...
v++ --link --profile.stall:k1:cu2 ...
```

#### Displayed in the footer

##### ★

###### !!

###### !!

### --profile.trace_memory:<arg>

**Tip:** This option applies to hardware build targets (-t=hw) only, and should not be used for software or hardware emulation flows.
When building the hardware target (-t=hw), use this option to specify the type and amount of memory to use for capturing trace data.
You can specify the argument as follows:

```
<FIFO>:<size>|<MEMORY>[<n>][:<SLR>]
```
This argument specifies memory type to use for capturing trace data. Use the --profile.trace_memory command to define the
type or memory to use, with the trace_buffer_size switch in the xrt.ini file to define the amount of memory to use as described in
xrt.ini File. The default memory type used is the first memory defined in the platform, and the default buffer size is 1 MB.
When trace_memory is not specified but device_trace is enabled in the xrt.ini File, the profile data is captured to the default
platform memory with 1 MB allocated for the trace buffer.

**FIFO:<size>**
Specified in KB. The maximum is 128K, although 64K is the maximum recommended.

**Memory**
Specifies the type and number of memory resource on the platform. Memory resources for the target platform can be identified
with the platforminfo command. Supported memory types include HBM, DDR, PLRAM, HP, ACP, MIG, and MC_NOC. For
example, DDR[1].

**[:<SLR>]**
Optionally indicates that CUs assigned to the specified <SLR> should use the DDR or HBM resources specified in the <MEMORY>
field. Note that this syntax can only be used with DDR or HBM memory banks.

You can specify the --profile.trace_memory command with the memory size and unit such as FIFO:8k, or specify the memory
bank such as DDR[0] or HBM[3]. In this case, the profile data for all CUs are captured in the specified memory.
Or you can specify the memory to use to capture profile data, and the SLR assignment for that memory. In this case, the SLR
assignment indicates that any CUs assigned to the specified SLR should have profile data captured in the specified memory. This is
shown in the following config file example:

```
[profile]
trace_memory=DDR[1]:SLR0
trace_memory=DDR[2]:SLR1
```
In the example above, profile data for CUs assigned to SLR0 are captured in DDR bank 1, and CUs assigned to SLR1 are captured in
DDR bank 2. CUs are assigned to SLRs using the --connectivity.slr command as described in Assigning Compute Units to
SLRs on Alveo Accelerator Cards in the _Data Center Acceleration using Vitis_ (UG1700).
**Important:** You cannot mix DDR and HBM memory banks in a single design, and when you specify the <SLR> syntax you must use
that syntax for all trace_memory commands in the design.

###### --vivado Options

The –-vivado.XXX options are used to configure the Vivado tools for synthesis and implementation of your device binary (.xclbin).
For instance, you can specify the number of jobs to spawn, LSF commands to use for implementation runs, or the specific
implementation strategies to use. You can also configure optimization, placement, timing, or specify which reports to output.
**Important:** Familiarity with the Vivado Design Suite is required to make the best use of these options. See the _Vivado Design Suite
User Guide: Implementation_ (UG904) for more information.

### --vivado.impl.jobs

```
--vivado.impl.jobs <arg>
```
Specifies the number of parallel jobs the Vivado Design Suite uses to implement the device binary. Increasing the number of jobs allows
the Vivado implementation step to spawn more parallel processes and complete faster jobs.
For example:

```
v++ --link --vivado.impl.jobs 4
```

#### Displayed in the footer

###### !!

##### ★

### --vivado.impl.lsf

```
--vivado.impl.lsf <arg>
```
Specifies the bsub command line as a string to pass to an LSF cluster. This option is required to use the IBM Platform Load Sharing
Facility (LSF) for Vivado implementation.
For example:

```
v++ --link --vivado.impl.lsf '{bsub -R \"select[type=X86_64]\" -N -q medium}'
```
### --vivado.impl.strategies

```
--vivado.impl.strategies <arg>
```
Specifies a comma-separated list of strategy names for Vivado implementation runs. Use ALL to run all available implementation
strategies. This lets you run a variety of implementation strategies at the same time during the build process and allows you to more
quickly resolve the timing and routing issues of the design.
**Important:** Running ALL implementation strategies might launch 30 or more runs in the Vivado tool. This can be a tremendous drain
on resources, and is not advised. You can prevent this by defining a set of strategies to run, and using a command queue to distribute
the process load in some managed way, such as through the --vivado.impl.jobs or the --vivado.impl.lsf commands.

### --vivado.param

```
--vivado.param <arg>
```
Specifies parameters for the Vivado Design Suite to be used during synthesis and implementation of the FPGA binary (xclbin).

```
Tip: You can use the report_param Tcl command in the Vivado tool to identify the available parameters.
```
### --vivado.prop

```
--vivado.prop <arg>
```
Specifies properties for the Vivado Design Suite to be used during synthesis and implementation of the FPGA binary (xclbin).

**Table: Prop Options**

```
Property Name Valid Values Description
```
```
vivado.prop <object_type>.<object_name>.
<prop_name>
```
```
Type: Various This allows you to specify any property used in the
Vivado hardware compilation flow.
<object_type> is
run|fileset|file|project.
The <object_name> and <prop_name> values
are described in Vivado Design Suite Properties
Reference Guide (UG912).
Examples:
```
```
vivado.prop run.impl_1.
{STEPS.PLACE_DESIGN.ARGS.MORE
OPTIONS}={-no_bufg_opt}
```
```
vivado.prop fileset.
current.top=foo
```
```
If <object_type> is set to file, current is
not supported.
```

#### Displayed in the footer

##### ★

###### !!

```
Property Name Valid Values Description
If <object type> is set to run, the special
value of __KERNEL__ can be used to specify run
optimization settings for ALL kernels, instead of
the need to specify them one by one.
```
For example, from the command line:

```
v++ --link --vivado.prop run.impl_1.STEPS.PHYS_OPT_DESIGN.IS_ENABLED=true
--vivado.prop run.impl_1.STEPS.PHYS_OPT_DESIGN.ARGS.DIRECTIVE=Explore
--vivado.prop run.impl_1.STEPS.PLACE_DESIGN.TCL.PRE=/.../xxx.tcl
```
The example above enables the optional PHYS_OPT_DESIGN step as part of the Vivado implementation process, sets the Explore
directive for that step, and specifies a Tcl script to run before the PLACE_DESIGN step.

**Tip:** Each step in the Vivado synthesis and implementation process can have a Tcl prescript to run before the step, and a Tcl
postscript to run after the step. This lets you customize the build process by inserting pre-processing or post-processing Tcl commands
around the different steps. These scripts can be specified as shown in the example above.
These options can also be specified in a configuration file under the [vivado] section head using the following format:

```
[vivado]
prop=run.impl_1.STEPS.PHYS_OPT_DESIGN.IS_ENABLED=true
prop=run.impl_1.STEPS.PHYS_OPT_DESIGN.ARGS.DIRECTIVE=Explore
prop=run.impl_1.STEPS.PLACE_DESIGN.TCL.PRE=/.../xxx.tcl
```
**Important:** Some Vivado properties have spaces in their name, such as MORE OPTIONS and Tcl syntax requires these properties to
be enclosed in braces, {}. However, the placement of the braces in the --vivado options is important. You must surround the complete
property name with braces, rather than a portion of it. For instance, the correct placement would be:

```
--vivado.prop run.impl_1.{STEPS.PLACE_DESIGN.ARGS.MORE OPTIONS}={-no_bufg_opt}
```
While the following would result in an error during the build process:

```
--vivado.prop "run.impl_1.{STEPS.PLACE_DESIGN.ARGS.MORE OPTIONS}={-no_bufg_opt}"
```
### --vivado.synth.jobs

```
--vivado.synth.jobs <arg>
```
Specifies the number of parallel jobs the Vivado Design Suite uses to synthesize the device binary. Increasing the number of jobs
allows the Vivado synthesis to spawn more parallel processes and complete faster jobs.
For example:

```
v++ --link --vivado.synth.jobs 4
```
### --vivado.synth.lsf

```
--vivado.synth.lsf <arg>
```
Specifies the bsub command line as a string to pass to an LSF cluster. This option is required to use the IBM Platform Load Sharing
Facility (LSF) for Vivado synthesis.
For example:

```
v++ --link --vivado.synth.lsf '{bsub -R \"select[type=X86_64]\" -N -q medium}'
```

#### Displayed in the footer

##### ★

##### ★

###### Vitis Compiler Configuration File

Configuration files are the recommended way of working with either the Vitis Unified IDE, or the common command-line supported by
v++. A configuration file provides an organized way of passing options to the tools by grouping similar commands together, creating
reusable configuration files to perform specific tasks like connectivity or profiling, and minimizing and simplifying the v++ command line.
Some of the features that can be controlled through config file entries include:

```
AI Engine commands to configure component creation using v++ -c --mode aie
HLS commands to configure interface definition, configure synthesis and simulation, and control packaging when using v++ -c -
-mode hls
```
```
Tip: Almost all HLS commands must be specified in a configuration file. The exceptions are --platform and --freqhz,
which are permitted on the command line or in a config file.
Connectivity directives for system linking specifying the number of kernels to instantiate, or assigning AI Engine streaming ports to
PL kernel ports when using v++ --link
Package directives to configure the creation of boot files, the genration of the .xclbin file from a .xsa, and the creation of an SD
card when using v++ --package
Directives for the Vivado Design Suite to manage hardware synthesis and implementation.
Comments can be added to the configuration file by starting the line with a "#":
```
```
# This is a comment line
```
The Vitis Unified IDE use configuration files to drive component build and simulation processes. The configuration file can be created by
the tool at the time of component creation, can be imported from an existing component, or can be custom created and added to the
component separately. For the command-line flow the configuration file is specified through the use of the v++ --config option as
discussed in the v++ General Options. An example of the --config option follows:

```
v++ --link --config ../src/system.cfg
```
Config file commands can be non-composing, which means they are only permitted once such as --platform. In this case the
commands are read from the config file in the order they are encountered. The first command read is used. Config file commands can
also be composing, which means that multiple values can be specified to apply to specific ports or CUs in the design. In this case the
commands are cumulative, with any later conflicting commands either ignored or resulting in an error.
Commands are read in the order they are encountered. If the same switch is repeated with conflicting information, the first switch read
is used or an error is returned. The order of precedence for switches is as follows, where item one takes highest precedence:

1. Command line switches.
2. Config files as specified on the command line from left-to-right.
3. Within a single config file, precedence is as encountered from top-to-bottom.

In general, any v++ command option can be specified in a configuration file. However, the configuration file supports defining sections
containing groups of related commands under command headers to help manage build options and strategies. The following table lists
the defined section headers.

**Tip:** Multiple processes can be defined in a single configuration file. However, it is recommended to keep separate configuration files
for separate processes, such as HLS component creation, system linking, and system packaging.

**Table: Section Tags of the Configuration File**

```
Section Name Description
```
```
Unlabeled Generally, an unlabelled section can be placed at the top of a configuration file to contain commands
that are not specific to one of the following section heads. Examples of these command can include -
-part or --platform, --freqhz, and --debug.
```
```
[advanced] --advanced Options
```
```
[aie] Used for AI Engine compilation with the v++ -c --mode aie command
```
```
[clock] --clock Options
```
```
[connectivity] --connectivity Options
```

#### Displayed in the footer

```
Section Name Description
```
```
[debug] --debug Options
```
```
[hls] Used for HLS component compilation using the v++ -c --mode hls command. These commands
are described in v++ Mode HLS
```
```
[linkhook] --linkhook Options
```
```
[package] --package Options
```
```
[profile] --profile Options
```
```
[vivado] --vivado Options:
```
###### Using the Message Rule File

The v++ command executes various AMD tools during kernel compilation and linking. These tools generate many messages that
provide build status to you. These messages might or might not be relevant to you depending on your focus and design phase. The
Message Rule file (.mrf) can be used to better manage these messages. It provides commands to promote important messages to the
terminal or suppress unimportant ones. This helps you better understand the kernel build result and explore methods to optimize the
kernel.
The Message Rule file is a text file consisting of comments and supported commands. Only one command is allowed on each line.

### Comment

Any line with “#” as the first non-white space character is a comment.

### Supported Commands

By default, v++ recursively scans the entire working directory and promotes all error messages to the v++ output. The promote and
suppress commands below provide more control on the v++ output.

```
promote: This command indicates that matching messages should be promoted to the v++ output.
suppress: This command indicates that matching messages should be suppressed or filtered from the v++ output. Errors cannot
be suppressed.
```
Enter only one command per line.

### Command Options

The Message Rule file can have multiple promote and suppress commands. Each command can have one and only one of the
options below. The options are case-sensitive.

```
-id [<message_id>]: All messages matching the specified message ID are promoted or suppressed. The message ID is in
format of nnn-mmm. As an example, the following is a warning message from HLS. The message ID in this case is 204-68.
```
```
WARNING: [V++ 204-68] Unable to enforce a carried dependence constraint (II = 1, distance = 1,
offset = 1)
between bus request on port 'gmem'
(/matrix_multiply_cl_kernel/mmult1.cl:57) and bus request on port 'gmem'-severity [severity_level]
```
```
For example, to suppress messages with message ID 204-68, specify the following: suppress -id 204-68.
-severity [<severity_level>]: The following are valid values for the severity level. All messages matching the specified
severity level will be promoted or suppressed.
info
warning
critical_warning
For example, to promote messages with severity of 'critical-warning', specify the following: promote -severity
critical_warning.
```

#### Displayed in the footer

###### ✎

##### ★

### Precedence of Message Rules

The suppress rules take precedence over promote rules. If the same message ID or severity level is passed to both promote and
suppress commands in the Message Rule file, the matching messages are suppressed and not displayed.

### Example of Message Rule File

The following is an example of a valid Message Rule file:

```
# promote all warning, critical warning
promote -severity warning
promote -severity critical_warning
# suppress the critical warning message with id 19-2342
suppress -id 19-2342
```
## emconfigutil Utility

When running software or hardware emulation in the command line flow, it is necessary to create an emulation configuration file,
emconfig.json, used by the runtime library during emulation. The emulation configuration file defines the device type and quantity of
devices to emulate for the specified platform. A single emconfig.json file can be used for both software and hardware emulation.
**Note:** When running on real hardware, the runtime and drivers query the installed hardware to determine the device type and
quantity installed, along with the device characteristics.
To use the emconfigutil utility to automate the creation of the emulation file, specify the target platform and additional options in the
emconfigutil command line:

```
emconfigutil --platform <platform_name> [options]
```
At a minimum, you must specify the target platform through the -f or -–platform option to generate the required emconfig.json file.
The specified platform must be the same as specified during the host and hardware builds.
The emconfigutil options are provided in the following table.

**Table: emconfigutil Options**

```
Option Valid Values Description
```
```
-f or --platform Target device Required. Defines the target device from the specified platform.
```
```
--nd Any positive integer Optional. Specifies number of devices. The default is 1.
```
```
--od Valid directory Optional. Specifies the output directory. When running emulation, the
emconfig.json file must be in the same directory as the host
executable. The default is to write the output in the current directory.
```
```
-s or --save-temps N/A Optional. Specifies that intermediate files are not deleted and remain
after the command is executed. The default is to remove temporary
files.
```
```
--xp Valid AMD parameters and
properties.
```
```
Optional. Specifies additional parameters and properties. For example:
```
```
--xp prop:solution.platform_repo_paths=<xsa_path>
```
```
This example sets the search path for the target platforms.
```
```
-h or --help N/A Prints command help.
```
The emconfigutil command generates the emconfig.json configuration file in the output directory or the current working directory.

**Tip:** When running emulation, the location of the emconfig.json file can be specified by the $EMCONFIG_PATH variable, or must be
found in the same directory as the host executable.
The following example creates a configuration file targeting two xilinx_u200_gen3x16_xdma_2_202110_1 devices.


#### Displayed in the footer

```
$emconfigutil --xilinx_u200_gen3x16_xdma_2_202110_1 --nd 2
```
## kernelinfo Utility

The kernelinfo utility extracts and displays information from Xilinx object (XO) files which can be used during host code
development. This information includes hardware function names, arguments, offsets, and port data.
The following command options are available:

**Table: kernelinfo Commands**

```
Option Description
```
```
-h [ --help ] Print help message.
```
```
-x [ --xo_path ] <arg> Absolute path to file including file name and .xo extension.
```
```
-l [ --log ] <arg> By default, information is displayed on the screen. Otherwise, you can use the --log
option to output the information as a file.
```
```
-j [ --json ] Output the file in JSON format.
```
```
[input_file] XO file. Specify the XO file positionally or use the --xo_path option.
```
```
[output_file] Output from AMD v++ Compiler. Specify the output file positionally, or use the --log
option.
```
To run the kernelinfo utility, enter the following in a Linux terminal:

```
kernelinfo <filename.o>
```
The output is divided into three sections:

```
Kernel Definitions
Arguments
Ports
```
The report generated by the following command is reviewed to help better understand the report content. The report is broken down
into specific sections for better understandability.

```
kernelinfo krnl_vadd.xo
```
Where krnl_vadd.xo is a compiled kernel.

###### Kernel Definition

Reports high-level kernel definition information. Importantly, for the host code development, the kernel name is given in the name field.
In this example, the kernel name is krnl_vadd.

```
=== Kernel Definition ===
name: krnl_vadd
language: c
vlnv: xilinx.com:hls:krnl_vadd:1.0
preferredWorkGroupSizeMultiple: 1
workGroupSize: 1
debug: true
containsDebugDir: 1
sourceFile: krnl_vadd/cpu_sources/krnl_vadd.cpp
```
###### Arguments

Reports kernel function arguments.
In the following example, there are four arguments: a, b, c, and n_elements.


#### Displayed in the footer

```
=== Arg ===
name: a
addressQualifier: 1
id: 0
port: M_AXI_GMEM
size: 0x8
offset: 0x10
hostOffset: 0x0
hostSize: 0x8
type: int*
```
```
=== Arg ===
name: b
addressQualifier: 1
id: 1
port: M_AXI_GMEM
size: 0x8
offset: 0x1C
hostOffset: 0x0
hostSize: 0x8
type: int*
```
```
=== Arg ===
name: c
addressQualifier: 1
id: 2
port: M_AXI_GMEM1
size: 0x8
offset: 0x28
hostOffset: 0x0
hostSize: 0x8
type: int*
```
```
=== Arg ===
name: n_elements
addressQualifier: 0
id: 3
port: S_AXI_CONTROL
size: 0x4
offset: 0x34
hostOffset: 0x0
hostSize: 0x4
type: int const
```
###### Ports

Reports the memory and control ports used by the kernel.

```
=== Port ===
name: M_AXI_GMEM
mode: master
range: 0xFFFFFFFF
dataWidth: 32
portType: addressable
base: 0x0
```
```
=== Port ===
name: M_AXI_GMEM1
mode: master
range: 0xFFFFFFFF
dataWidth: 32
portType: addressable
```

#### Displayed in the footer

##### ★

##### ★

###### !!

```
base: 0x0
```
```
=== Port ===
name: S_AXI_CONTROL
mode: slave
range: 0x1000
dataWidth: 32
portType: addressable
base: 0x0
```
## launch_emulator Utility

For embedded platforms that have an Arm® subsystem, the AMD Vitis™ tool uses QEMU to emulate the PS subsystem. The QEMU
processes must be run along with the RTL simulator process to emulate the entire system in hardware emulation. The
launch_emulator.py is a utility which launches QEMU and manages the synchronization of the PL simulator processes. It launches
QEMU and the simulation process with provided arguments. The Vitis IDE also calls this command when starting and stopping the
emulator.

**Tip:** For help inside QEMU, press Ctrl + a h while in the emulator shell. To terminate the QEMU command, press Ctrl + a x while in
the emulator shell.
For embedded platforms, the v++ Command command generates the launch_hw_emu.sh script to call the launch_emulator.py
command with the required arguments based on the platform and the target application.
You can pass additional arguments to the launch_emulator utility from the command line when using the launch_hw_emu.sh
wrapper script. Simply append the option to the command line when running the script. This allows you to customize the
launch_emulator utility as needed to support your specific platform or application.
The following table shows the list of available options.

**Table: Common Options for launch_emulator**

```
Option Accepted Value Description
```
```
-add-env ADD_ENV_CMD N/A Specify additional Environment Variables for the
emulation shell.
```
```
-aie-sim-options AIE_SIM_OPTIONS file Points to an AI Engine sim options file that has various AI
Engine debug flags that are required for debugging the AI
Engine SystemC module. Refer to the section on Reusing
AI Engine Simulator Options in the AI Engine Tools and
Flows User Guide (UG1076) for more information.
The options file should be specified with a relative path
with respect to
package.hw_emu/sim/behav_waveform/xsim/.
```
```
Tip: This is optional and only applies to AI Engine
designs.
You can enable profiling options that were used during
aiesimulator to be applied to hardware emulation of
the system-level design. To do this, add the following
command:
```
```
-aie-sim-options
../aiesimulator_output/aiesim_options.txt
```
```
-enable-debug N/A Debug mode opening two different XTERMs for QEMU
and PL.
Important: This is very useful for the batch mode users
to understand the flow and handshake between the
QEMU and PL process.
```
```
-graphic-qemu N/A Start the Quick Emulator(QEMU) in GUI mode
```

#### Displayed in the footer

##### ★

```
Option Accepted Value Description
```
```
-help N/A Prints help message.
```
```
-run-app <application_script_name> Ensure that the application script is packaged using --
package.sd_file option during package step. Only
when it is packaged in sd_card, the application script is
available to run after QEMU is up running and mounted.
```
```
Tip: When using the -run-app option, all QEMU
messages are initially written to a file called
qemu_output.log inside the package.hw_emu, and then
re-written to the console after some delay. You can
examine the contents of the qemu_output.log if you need
to verify whether any problems incurred.
```
```
-timeout <n> Terminates emulation after <n> seconds. The default
value when -run-app is used is 4000 seconds. This
means the application terminates after running for 4000
seconds without user intervention.
```
```
-user-post-sim-script Path to Tcl script required to be
done post simulation before quit
```
```
Creates Tcl for any post operations into a Tcl file and pass
the Tcl script to this switch.
```
```
-user-pre-sim-script Path to Tcl script For the first run, launch the script
launch_emulator.py in GUI mode and add the signals
that you want to observe.
Copies the commands from the Tcl console and save into
a Tcl script.
From the next run, pass the Tcl script in batch mode,
launch_emulator.py -user-pre-sim-script
<path_to_saved_tcl_script>.
Only supports the Vivado simulator (xsim).
```
```
-verbose N/A Enables additional debug messages
```
```
-wcfg-file-path N/A Specifies the wcfg file created by the XSIM to open during
GUI simulation. Requires complete absolute path of the
file.
```
```
-wdb-File Path to WDB file Specifies the wdb file to load. Requires complete absolute
path of the file.
```
```
-xtlm-aximm-log N/A This switch generates xTLM AXI4 transaction logs for
interface connection between two SystemC models (with
information like address/data/size, etc).
While running the emulation log is available at (directory
structure can vary based on v++ options and simulator
used):
package.hw_emu/sim/behav_waveform/xsim/xsc_report.log
```
```
-xtlm-axis-log N/A This switch generates xTLM AXI4-Stream transaction logs
for interface connection between two SystemC models.
While running emulation log is available at (directory
structure can vary based on v++ options and simulator
used):
package.hw_emu/sim/behav_waveform/xsim/xsc_report.log
```
**Table: Advanced Options for launch_emulator**

```
Option Accepted Value Description
```

#### Displayed in the footer

##### ★

##### ★

```
Option Accepted Value Description
```
-disable-host-
completion-check

```
N/A Skips the check for host/test completion. Generally used
in applications where python scripts check for the test
completion status PASS/FAIL.
By default, search for "TEST PASSED" string when -
run-app switch is used.
```
-enable-tcp-sockets N/A Enables TCP Sockets

-kill <pid> Kills a specified emulator process.

-kill-pid-file N/A Specifies the file to be used to kill the process. This file
stores the group PID of the process. Can be created
using -pid-file.

-no-reboot N/A Exits QEMU instead of rebooting. Used to exit gracefully
from QEMU by executing command reboot -f at the
embedded Linux prompt.

-no_build N/A Enables a check of the build command without running
the build process.

-no_run N/A Builds but does not run the emulation.

-ospi-image OSPI Image file Specifies an OSPI image file for booting.

-pl-sim-args Arguments to simulator These arguments are appended to the simulator
command line. They are an alternative to pm-sim-args-
file.

-pmc-args Arguments to PMC The PMC/PMU is emulated by qemu-system-
microblazeel. The most common command line
switches of the PMC are captured in pmc_args.txt.
Instead of writing to a file called pmc_args.txt, you can
directly provide all the arguments that need to be
appended to the PMC command line. This is an
alternative to -pmc-args-file.
PMC/PMU arguments for specific devices can be found in
Versal PS and PMC Arguments for QEMU and Zynq
UltraScale+ MPSoC PS and PMU Arguments for QEMU.

```
Tip: This option is not supported for AMD Zynq™
7000 devices.
```
-pmc-args-file PMC QEMU arguments file name Any option to be passed to PMU/PMC can be provided in
this file. The specific format is determined by the base file
on your chosen platform.
This is auto passed in the v++ package generated script.
PMC/PMU arguments for specific devices can be found in
Versal PS and PMC Arguments for QEMU and Zynq
UltraScale+ MPSoC PS and PMU Arguments for QEMU.

```
Tip: This option is not supported for Zynq 7000
devices.
```
-print-qemu-version N/A Prints the version of QEMU being used.

-qemu-args Arguments to QEMU The PS is emulated by qemu-system-aarch64. The
most common command line switches of the PS are
captured in qemu_args.txt.
Instead of writing into a file called qemu_args.txt, you can
directly provide all the arguments that need to be


#### Displayed in the footer

###### ✎

```
Option Accepted Value Description
appended to the QEMU command line. This is an
alternative to qemu-args-file.
PS arguments for specific devices can be found in Versal
PS and PMC Arguments for QEMU and following sections
for AMD Zynq™ UltraScale+™ MPSoC.
```
```
-qemu-dtb <path_to_DTB_file> v++ --package automatically creates a DTB file based
on the addressing in the design and passes it to the
launch_emulator command. This option lets you
specify the DTB file to override the defaults.
Note: Ensure the DTB is compatible with the
noc_memory_config.txt file used.
```
```
-qspi-high-image Specify QSPI high image file The image file which is passed as a QEMU argument in
the form of boot mode. This is auto passed in the V++
package generated script.
Required only when DUAL QSPI mode is used.
```
```
-qspi-image Specify qspi.bin The image file is passed as a QEMU argument in the
form of boot mode. This is auto passed in the V++
package generated script.
Required only when you opt for QSPI mode.
```
```
-qspi-low-image Specify QSPI low image file The image file is passed as a QEMU argument in the
form of boot mode. This is auto passed in the V++
package generated script.
Required only when DUAL QSPI mode is used.
```
```
-result-string N/A Result string searches for the status of test completion.
Default = "TEST PASSED."
```
```
-use-qemu-version-v4 N/A Use QEMU version 4.2
```
**Table: Auto-Populated by V++ in Emulation Script**

```
Option Accepted Value Description
```
```
-aie-sim-config N/A Points to the AI Engine sim config file that provides
various AI Engine files required for the SystemC Model of
AI Engine.
This is auto passed by the v++ package.
Required for AI Engine designs.
```
```
-boot-bh Path to BH file Specify the boot BH file path
```
```
-device-family 7Series | UltraScale | Versal Required to specify the device family for the platform.
This is auto passed by the v++ package generated script
launch_hw_emu.sh.
This needs to be passed manually for direct usage of the
launch_emulator command.
```
```
-enable-prep-target N/A Enable pre-process target.
```
```
-forward-port <target> <host> Forwards TCP port from target to host.
```
```
-gdb-port Port number QEMU waits for GDB connection on <port>.
```
```
-noc-memory-config
<path/to/noc_memory_config.txt>
```
```
N/A By default, v++ --package creates the NoC memory
configuration based on the design configuration, and you
can see this file parallel to simulation binaries. You can
override this file by replacing the file specified in the
```

#### Displayed in the footer

##### ★

###### ✎

##### ★

```
Option Accepted Value Description
simulation binary folder. Use the -user-pre-sim-
script option to copy your noc_memory_config.txt file to
the simulation binary area and to get the configuration
applied.
```
-pid-file File name Write process ID to <file> for later use with -kill. Used
by the Vitis software platform to kill when emulation is
successful.

-pl-sim-dir Simulation directory Start the Programmable Logic Simulator by launching the
scripts from this directory. This is auto passed in the v++
package generated script. The tool expects a file called
simulate.sh in the specified directory and will execute it to
launch the PL simulator (for example, XSIM).

-pl-sim-script Simulation script location Advanced users can have one direct script to launch
simulation (for example, Vivado users).
When this option is given, run the script, other options are
of no value.

-platform-name NAME The platform name

-pmc-args-file PMC QEMU arguments file name Any options to be passed to PMU/PMC can be given in
this file. The specific format is determined by the base file
on your chosen platform.
This is auto passed in the v++ package generated script.
PMC/PMU arguments for specific devices can be found in
Versal PS and PMC Arguments for QEMU and Zynq
UltraScale+ MPSoC PS and PMU Arguments for QEMU.

```
Tip: This option is not supported for Zynq 7000
devices.
```
-pmc-dtb <path_to_DTB_file> v++ --package automatically creates a device-tree
binary (DTB) file based on the addressing in the design
and passes it to the launch_emulator command. This
option lets you specify the DTB file to override the
defaults.
**Note:** Ensure the DTB is compatible with the
noc_memory_config.txt file used.
PMC/PMU arguments for specific devices can be found in
Versal PS and PMC Arguments for QEMU and Zynq
UltraScale+ MPSoC PS and PMU Arguments for QEMU.

```
Tip: This option is not supported for Zynq 7000
devices.
```
-protoinst-File Path to ProtoInst file Specify the protoinst file to load. Provide the complete
absolute path.

-qemu-args-file PS QEMU Arguments file name Any options to be passed to QEMU can be given in this
file. This is specific format where you fetch the base file
from the platform chosen. This is auto passed in the v++
package generated script.

-qemu-dtb <path_to_DTB_file> v++ --package automatically creates a DTB file based
on the addressing in the design and passes it to the
launch_emulator command. This option lets you
specify the DTB file to override the defaults.


#### Displayed in the footer

###### ✎

##### ★

```
Option Accepted Value Description
Note: Ensure the DTB is compatible with the
noc_memory_config.txt file used.
```
```
-sd-card-image Specify sd_card.img The image file is passed as a QEMU argument in the
form of boot mode. This is auto passed in the V++
package generated script.
Required only when SD mode is used.
```
```
-t | -target hw_emu v++ package generates the hardware emulation script
launch_hw_emu.sh.
```
```
-xtlm-log-state WAVEFORM | LOG | BOTH Option to specify what the XTLM Log should contain. It
can contain the waveform, the text log, or both. This
option is used only for hardware emulation.
```
###### Versal PS and PMC Arguments for QEMU

In the AMD Versal™ device, the PS(a72) is emulated by qemu-system-aarch64 and PMC is emulated by qemu-system-
microblazeel. Most common command line switches of PS are captured in qemu_args.txt and PMC command line switches are
captured in pmc_args.txt.

```
Tip: You can add comments to the pmc_args.txt, qemu_args.txt, and pmu_args.txt files using the '#' symbol at the start of the line.
```
**Table: Versal Options for qemu_args.txt**

```
Arg Name Value Description Source How to Extract the Info
```
```
-boot mode=<boot-number>
Ex. for sd1 -boot
mode=5
```
```
Specify boot mode on
your platform:
```
```
qspi24 = 1
qspi32 = 2
sd0 = 3
sd1 = 5
emmc0 = 6
ospi = 8
```
```
v++ -p DRC needed between
CIPS enabled boot
modes and v++ -p
selection
```
```
-display none By default QEMU
creates display for user
I/O. This option
disables the display
```
```
Static Specify none to disable
the display
```
```
-hw-dtb <ps-dtb-file> Device tree file which
describes the PS (a72).
The Vitis compiler --
package command
generates the dtb file
and appends it to
qemu-args.txt.
```
```
v++ -p
```
```
-M arm-generic-fdt This specifies the
QEMU machine to
create. arm-generic-
fdt machine option
tells QEMU to parse dtb
for machine generation,
passes by -hw-dtb
user.dtb.
```
```
Device specific Hard coded for Versal
```

#### Displayed in the footer

```
Arg Name Value Description Source How to Extract the Info
```
```
-serial -serial null -serial null -
serial mon:stdio
```
```
By default, QEMU
connects invoking
terminal to UART0 for
user I/O operations.
You can override this
behavior by specifying
this option. Versal
platforms have four
UARTs specified using
positional arguments:
the first two are for
debug and the last two
are UART0 and
UART1.
To connect UART0 to
the terminal, specify -
serial null -
serial null -
serial mon:stdio
which ignores debug
related UARTS and
connects to UART0 to
terminal.
Similarly to connect
only UART1 to terminal
specify -serial null
-serial null -
serial null -
serial mon:stdio
```
```
Based on UARTs
enabled on CIPS
configuration.
```
```
The Versal device has
four UARTs. The first
two are debug related
UARTs.
When enabling UART0:
```
```
CONFIG.PS_UART
0_PERIPHERAL_E
NABLE = 1
CONFIG.PS_UART
1_PERIPHERAL_E
NABLE =0 or 1
```
```
Then specify -serial
null -serial null
-serial mon:stdio
When enabling only
UART1:
```
```
CONFIG.PS_UART
0_PERIPHERAL_E
NABLE = 0
CONFIG.PS_UART
1_PERIPHERAL_E
NABLE = 1
```
```
Then specify: -serial
null -serial null
-serial null -
serial mon:stdio
```
```
-sync-quantum Time in milliseconds Specifies how
frequently QEMU will
sync with the RTL
simulator. Modifying
this can have an impact
on the speed of
simulation.
```
```
Static Hard coded for Versal
devices (user need to
change)
```
Versal CIPS has two Ethernet interfaces. Most of AMD Versal CIPS board has eth0 enabled. If no -net or -netdev is specified then
QEMU by default enables eth0 and maps to user mode backend.

**Table: Versal Options for pmc_args.txt**

```
Switch Name Value Description Source of the ConfigHow to Extract the Info
```
```
-M microblaze-fdt Specifies the QEMU
machine to create. QEMU
creates MicroBlaze™ with
nodes from dtb.
```
```
Static Hard coded for Versal
Devices
```
```
-display none By default, QEMU creates
display for user I/O. This
option instructs QEMU that
there is no need for display.
```
```
Static Hard coded for Versal
Devices
```
```
-device loader,file=
<BOOT_bh.bin>,addr=0xf201e000,force-
raw
```
```
Specifies Boot header file
with load address as
0xF201E000 (BOOT_bh.bin
is loaded at address
```
```
v++ -pack v++ pack extracts
BOOT_bh.bin and
generates this switch
```

#### Displayed in the footer

##### ★

```
Switch Name Value Description Source of the ConfigHow to Extract the Info
0xF201E000). This is fixed
argument in pmc_args.txt
which is processed by v++ -p
for final argument which has
absolute path of
BOOT_bh.bin file.
BOOT_bh.bin is generated
by v++ -p from final PDI.
BOOT_bh.bin is directly
loaded onto QEMU because
there is no BOOT ROM
access for QEMU to load
boot header from PDI.
```
```
-device loader,file=
<pmc_cdo.bin>,addr=0xF2000000,force-
raw
```
```
Specifies pmc_cdo.bin with
load address as
0xF2000000. This is fixed
argument in pmc_args.txt.
This is processed by v++ -p
for final argument which has
absolute path of
pmc_cdo.bin file.
```
```
v++ -pack v++ pack extracts
pmc_cdo.bin and
generates this switch
```
```
-device loader,file=
<plm.bin>,addr=0xF0200000,force-
raw
```
```
Specifies plm binary
firmware with load address
as 0xF0200000. This is a
fixed argument in
pmc_args.txt which is
processed by v++ -p for final
argument. This has an
absolute path of plm.bin file.
This plm is executed by
PPU1 when it is out of reset.
```
```
v++ -pack v++ pack extracts
plm.bin and generates
this switch
```
```
-device loader,addr=0xf0000000,data=0xba020004,data-
len=4 -device
loader,addr=0xf0000004,data=0xb800fffc,data-
len=4
```
```
Specifies PPU0 process to
be in boot loop. As there is
no BOOTROM access for
QEMU, PPU0 is put in
bootloop which generally
loads BOOT header from
PDI to memory.
```
```
Static Hard coded for Versal
Devices
```
```
-device loader,addr=0xF1110624,data=0x0,data-
len=4 -device
loader,addr=0xF1110620,data=0x1,data-
len=4
```
```
Makes PPU1 out of reset
and puts in executing mode.
```
```
Static Hard coded for Versal
Devices
```
```
-hw-dtb <ps-dtb-file> dtb file which describes aout
PS(a72). v++ pack generates
this dtb file and appends to
qemu-args.txt.
```
```
v++ pack v++ pack generates dtb
files based on DDR
config on device
```
###### Zynq UltraScale+ MPSoC PS and PMU Arguments for QEMU

The Zynq UltraScale+ MPSoC PS (a53) is emulated by qemu-system-aarch64 and PMU is emulated by qemu-system-
microblazeel. Most common command line switches of PS are captured in qemu_args.txt and PMC command line switches are
captured in pmu_args.txt.

```
Tip: You can add comments to the pmc_args.txt, qemu_args.txt, and pmu_args.txt files using the '#' symbol at the start of the line.
```

#### Displayed in the footer

##### ★

**Table: Zynq UltraScale+ MPSoC Options for qemu_args.txt**

```
Switch Name Value Description Source of the Config How to Extract the Info
```
```
-M arm-generic-fdt This specifies the
QEMU machine to
create. arm-generic-
fdt machine option
tells QEMU to parse dtb
for machine generation,
passes by -hw-dtb
user.dtb.
```
```
Static Hard-coded for Zynq
UltraScale+ MPSoC
devices
```
```
-serial mon:stdio -serial is a positional
argument. Redirect the
serial port to specified
char dev (i.e., stdio, tcp
port, file, etc.).
```
```
Based on UART
configuration on Zynq
UltraScale+ MPSoC
```
```
Zynq UltraScale+
MPSoC has two
UARTs.
When enabling UART0:
```
```
CONFIG.PSU__UA
RT0__PERIPHERA
L__ENABLE = 1
CONFIG.PSU__UA
RT1__PERIPHERA
L__ENABLE = 0
or 1
```
```
Then specify: -serial
mon:stdio
When enabling only
UART1:
```
```
CONFIG.PSU__UA
RT0__PERIPHERA
L__ENABLE = 0
CONFIG.PSU__UA
RT1__PERIPHERA
L__ENABLE = 1
```
```
Then specify: -serial
null -serial
mon:stdio
```
```
-global xlnx,zynqmp-boot.cpu-
num=0
```
```
Make the specified
CPU come out of reset.
```
```
Static Hard coded for Zynq
UltraScale+ MPSoC
devices
```
```
-net -net nic -net nic -net nic
-net nic -net user
```
```
-net is positional
argument. Initialize
network interfaces
gem3. Connect the
specified network
adapter to user mode
network.
```
```
Tip: -net none
will disable all Ethernet
interfaces.
```
```
Static Based on Ethernet
configurations:
If gem0(eth0) is
enabled:
```
```
CONFIG.PSU__EN
ET0__PERIPHERA
L__ENABLE =1
```
```
Then specify -net
nic -net user
If gem1 is enabled:
```
```
CONFIG.PSU__EN
ET1__PERIPHERA
```

#### Displayed in the footer

##### ★

```
Switch Name Value Description Source of the Config How to Extract the Info
L__ENABLE = 1
Then specify -net
nic -net nic -net
user
If gem2 is enabled:
```
```
CONFIG.PSU__EN
ET2__PERIPHERA
L__ENABLE = 1
```
```
Then specify -net
nic -net nic -net
nic -net user
If gem 3 is enabled:
```
```
CONFIG.PSU__EN
ET3__PERIPHERA
L__ENABLE = 1
```
```
Then specify -net
nic -net nic -net
nic -net nic -net
user
```
```
Tip: If no -net
(and/or -netdev) is
mentioned then by
default QEMU will
enable the first Ethernet
(gem0) and map it to
user mode backend.
```
```
-m 4G Enabling 4 GB DDR on
Zynq UltraScale+
MPSoC.
```
```
Static Emulating full DDR on
Zynq UltraScale+
MPSoC
```
```
-device loader,file=
<bl31.elf>,cpu-num=0
```
```
Load bl31.elf file on
A53 core 0.
```
```
Static v++ --package
should replace bl31.elf
with absolute path of
bl31.elf
```
```
-device loader,file=<u-boot.elf> Loading u-boot.elf. Static v++ --package
should replace bl31.elf
with absolute path of u-
boot.elf
```
```
-hw-dtb <ps-dtb-file> dtb file which describes
PS which is emulated
by QEMU can be
specified using -hw-
dtb.
```
```
Static Hard coded for Zynq
UltraScale+ MPSoC
devices:
<ps-dtb-
file>=/proj/xbuilds/HEAD_d
arm-cosim.dtb
```
**Table: Zynq UltraScale+ MPSoC Options for pmu_args.txt**

```
Switch Name Value Description Source of the ConfigHow to Extract the Info
```

#### Displayed in the footer

##### ★

##### ★

```
Switch Name Value Description Source of the ConfigHow to Extract the Info
```
```
-M microblaze-fdt This specifies the QEMU
machine to create.
microblaze-fdt tells QEMU to
parse dtb for machine
generation, passes by -hw-
dtb user.dtb.
```
```
Static Hard coded for Zynq
UltraScale+ MPSoC
devices
```
```
-device loader,file=<pmufw.elf> Load pmufw.elf file on PMU
RAM.
```
```
Static Hard coded for Zynq
UltraScale+ MPSoC
devices
```
```
-machine-path <path-to-xsim-dir> Point -machine-path to
folder to create shared RAM
and remote-port sockets.
```
```
Static launch_emulator
command will set this
machine path
```
```
-display none By default, QEMU creates
display for user I/O. This
option disables the display.
```
```
Static Hard coded for Zynq
UltraScale+ MPSoC
devices
```
```
-hw-dtb <pmu-dtb-file> dtb file which describes PMU
which is emulated by QEMU
can be specified using -hw-
dtb.
```
```
Static <pmu-dtb-
file>=/proj/xbuilds/HEAD_daily_late
pmu.dtb
```
**Tip:** Although the file is called pmu_args.txt here, the file is specified for launch_emulator using the -pmc-args-file
command.

###### Zynq 7000 PS Arguments for QEMU

Zynq 7000 PS(a9) is emulated by qemu-system-aarch64 QEMU binary. Most common command line switches of PS are captured in
qemu_args.txt.

```
Tip: You can add comments to the pmc_args.txt, qemu_args.txt, and pmu_args.txt files using the '#' symbol at the start of the line.
```
**Table: Zynq 7000 Options for qemu_args.txt**

```
Switch Name Value Description Source of the Config How to Extract the Info
```
```
-M arm-generic-fdt-7series Indicates the QEMU
machine to create. arm-
generic-fdt-7series tells
QEMU to parse dtb for
machine generation,
passes by -hw-dtb
user.dtb.
```
```
Static Hard coded for Zynq 7000
devices
```
```
-serial -serial /dev/null -serial
mon:stdio
```
```
Redirect the serial port to
specified char dev (i.e.,
stdio, tcp port, file, etc.)
```
```
Based on the UART
configuration of the Zynq IP.
```
```
Zynq 7000 has two UARTs.
When enabling UART0:
```
```
CONFIG.PCW_UART0_
PERIPHERAL_ENABLE
= 1
CONFIG.PCW_UART1_
PERIPHERAL_ENABLE
= 0 or 1
```
```
Then specify: -serial
mon:stdio
When enabling only
UART1:
```

#### Displayed in the footer

```
Switch Name Value Description Source of the Config How to Extract the Info
```
```
CONFIG.PCW_UART1_
PERIPHERAL_ENABLE
= 1
Then specify: -serial
null -serial
mon:stdio
```
```
-device loader,addr=0xf8000008,data=0xDF0D,data-
len=4 -device
loader,addr=0xf8000140,data=0x00500801,data-
len=4 -device
loader,addr=0xf800012c,data=0x1ed044d,data-
len=4 -device
loader,addr=0xf8000108,data=0x0001e008,data-
len=4 -device
loader,addr=0xF800025C,data=0x00000005,data-
len=4 -device
loader,addr=0xF8000240,data=0x00000000,data-
len=4
```
```
Register writes to SLCR
block, to set PLL and
CLK_CTRL regs (required
for Linux).
```
```
Static Hard coded for Zynq 7000
devices
```
```
-boot mode=5 Boot mode 5 is for SD boot. v++ -p
```
```
-kernel <u-boot.elf> Guest software to load
during boot up.
```
```
Static <u-boot.elf> is replaced
with the absolute path of u-
boot.elf from the target
platform
```
```
-machine linux=on Make QEMU itself a loader
of the Linux image.
```
```
Static Hard coded for Zynq 7000
devices
```
## manage_ipcache Utility

To provide better performance during synthesis of kernels in your application designs, the AMD Vitis™ compiler uses an IP cache to
store and reuse synthesis results. This lets the build process for the .xclbin file avoid having to repeat synthesis for kernels and CUs
that have not changed. The IP cache stores the synthesis results and applies them for unchanged kernels in the design.
By default, the IP cache is stored inside the Vitis IDE workspace for a project, or at the level of your builds when running v++ from the
command line. You can customize the location for the IP cache using --remote_ip_cache to specify a new location, or disable the
use of the IP cache using --no_ip_cache. See v++ General Options for information on these options.
The manage_ipcache utility is a standalone utility to help you manage the contents of your IP cache repository. It lets you report
statistics on the IP cache repository and remove entries based on a variety of criteria.

**Table: manage_ipcache Options**

```
Option Description
```
```
-c | --cache Required. Specifies the IP Cache directory to work on.
```
```
-d | --disk_space <size> Delete all but the most recently used entries that fit in the disk space specified in MB.
```
```
-h | --help Prints help for the manage_ipcache command.
```
```
-k | --keep_top <N> Delete all but the top N most recently used entries (N is an integer).
```
```
-o | --outfile <file> Report stats for the IP cache to the specified file.
```
```
-p | --purge Delete ALL cache entries.
```
```
-r | --report Report stats for the IP cache to stdout.
```
```
-u | --unused Delete cache entries that have never been used (no cache hits).
```

#### Displayed in the footer

##### ★

###### ✎

The following example reports on the entries of the specified IP cache:

```
manage_ipcache --cache ./ip_cache --report
```
The manage_ipcache command returns 0 if successful, or returns -1 if an error occurs.

## package_xo Command

### Syntax

```
package_xo -kernel_name <arg> [-force] [-kernel_xml <arg>]
[-output_kernel_xml <arg>] [-design_xml <arg>]
[-ip_directory <arg>] [-parent_ip_directory <arg>]
[-kernel_files <args>] [-kernel_xml_args <args>]
[-kernel_xml_pipes <args>] [-kernel_xml_connections <args>]
[-ctrl_protocol <arg>] -xo_path <arg> [-quiet] [-verbose]
```
### Description

The package_xo command is a Tcl command within the AMD Vivado™ Design Suite. Kernels written in RTL are compiled in the
Vivado tool using the package_xo command line utility which generates a Xilinx object (XO) file which can subsequently used by the
v++ command, during the linking stage.

**Table: Arguments**

```
Argument Description
```
```
-kernel_name <arg> (Required) Specify the name of the RTL kernel.
```
```
-force (Optional) Overwrite an existing XO file if one exists.
```
```
-kernel_xml <arg> (Optional) Specify the path to an existing kernel XML file. The Vivado tool will
create a kernel.xml file for the XO file if one is not specified.
```
```
-output_kernel_xml (Optional) Specify the path to write the kernel XML file. The Vivado tool will
create a kernel.xml file to include in the XO file, and also write it to the
specified output file.
```
```
Tip: You can use this option to generate a kernel.xml file which you can
edit and use as an input in the package_xo command.
```
```
-design_xml <arg> (Optional) Specify the path to an existing design XML file
```
```
-ip_directory <arg> (Optional) Specify the path to the packaged IP directory.
```
```
-parent_ip_directory (Optional) If the kernel IP directory specified contains multiple IPs, specify a
directory path to the parent IP where its component.xml is located directly
below.
```
```
-kernel_xml_args <args> (Optional) Generate the kernel.xml with the specified function arguments.
Each argument value should use the following format:
```
```
{name:addressQualifier:id:port:size:offset:type:memSize}
```
```
Note: memSize is optional.
```
```
-kernel_xml_pipes <args> (Optional) Generate the kernel.xml with the specified pipe(s). Each pipe value
use the following format:
```
```
{name:width:depth}
```

#### Displayed in the footer

##### ★

###### ✎

###### ✎

##### ★

##### ★

```
Argument Description
```
```
-kernel_xml_connections <args> (Optional) Generate the kernel.xml file with the specified connections. Each
connection value should use the following format:
```
```
{srcInst:srcPort:dstInst:dstPort}
```
```
-ctrl_protocol Kernel control protocol as described in PL Kernel Properties in the Data
Center Acceleration using Vitis (UG1700). Valid values: ap_ctrl_hs,
ap_ctrl_chain, ap_ctrl_none, user_managed.
```
```
Tip: The default ap_ctrl_hs is written to the kernel.xml file when -
ctrl_protocol is not specified.
```
```
-xo_path <arg> (Required) Specify the path and file name of the compiled object (XO) file.
```
```
-quiet (Optional) Execute the command quietly, returning no messages from the
command. The command also returns TCL_OK regardless of any errors
encountered during execution.
Note: Any errors encountered on the command-line, while launching the
command, will be returned. Only errors occurring inside the command will be
trapped.
```
```
-verbose (Optional) Temporarily override any message limits and return all messages
from this command.
Note: Message limits can be defined with the set_msg_config
command.
```
### Examples

The following example creates the specified XO file containing an RTL kernel of the specified name using the ap_ctrl_chain control
protocol, and creates the kernel.xml file because one has not been specified:

```
package_xo -xo_path Vadd_A_B.xo -kernel_name Vadd_A_B -ctrl_protocol ap_ctrl_chain -ip_directory ./ip
```
The following example creates the XO file using the specified kernel.xml file:

```
package_xo -xo_path Vadd_A_B.xo -kernel_name Vadd_A_B -kernel_xml kernel.xml -ip_directory ./ip
```
```
Tip: The control protocol will be defined in the specified kernel.xml file.
```
###### RTL Kernel XML File

**Tip:** The package_xo command will create a kernel.xml file from the component.xml of a packaged IP, so you do not need to
manually provide one, or generate one using the RTL Kernel wizard.
An XML kernel description file, called kernel.xml, must be created for each RTL kernel, so that it can be used in the AMD Vitis™
application acceleration development flow. The kernel.xml file specifies kernel attributes like the register map and ports needed by the
runtime and Vitis tool flows. The following code shows is an example of a kernel.xml file.

```
<?xml version="1.0" encoding="UTF-8"?>
<root versionMajor="1" versionMinor="6">
<kernel name="vitis_kernel_wizard_0" language="ip_c"
vlnv="mycompany.com:kernel:vitis_kernel_wizard_0:1.0"
attributes="" preferredWorkGroupSizeMultiple="0" workGroupSize="1" interrupt="true">
<ports>
<port name="s_axi_control" mode="slave" range="0x1000" dataWidth="32" portType="addressable"
base="0x0"/>
<port name="m00_axi" mode="master" range="0xFFFFFFFFFFFFFFFF" dataWidth="512"
portType="addressable"
base="0x0"/>
```

#### Displayed in the footer

###### ✎

```
</ports>
<args>
<arg name="axi00_ptr0" addressQualifier="1" id="0" port="m00_axi" size="0x8" offset="0x010"
type="int*"
hostOffset="0x0" hostSize="0x8"/>
</args>
</kernel>
</root>
```
**Note:** The kernel.xml file can be created automatically using the RTL Kernel Wizard to specify the interface specification of your RTL
kernel.
The following table describes the format of the kernel.xml in detail:

**Table: Kernel XML File Content**

```
Tag Attribute Description
```
```
<root>
```
```
versionMajor For the current release of Vitis software platform, set to 1.
```
```
versionMinor For the current release of Vitis software platform, set to 6.
```
```
<kernel>
```
```
name Kernel name
```
```
language Always set to ip_c for RTL kernels.
```
```
vlnv
```
```
Must match the vendor, library, name, and version attributes in the
component.xml of an IP. For example, if component.xml has the
following tags:
<spirit:vendor>xilinx.com</spirit:vendor>
<spirit:library>hls</spirit:library>
<spirit:name>test_sincos</spirit:name>
<spirit:version>1.0</spirit:version>
The vlnv attribute in kernel XML must be set
to:xilinx.com:hls:test_sincos:1.0
```
```
attributes Reserved. Set it to empty string: ""
```
```
preferredWorkGroupSizeMultipleReserved. Set it to 0.
```
```
workGroupSize Reserved. Set it to 1.
```
```
interrupt Set to "true" (interrupt="true") if the RTL kernel has an interrupt,
otherwise omit.
```
```
hwControlProtocol Specifies the control protocol for the RTL kernel.
```
```
ap_ctrl_hs: Default control protocol for RTL kernels.
ap_ctrl_chain: Control protocol for chained kernels that
support dataflow. Adds ap_continue to the control registers to
enable ap_done/ap_continue completion acknowledgment.
ap_ctrl_none: Control protocol (none) applied for data driven
kernels.
user_managed: Specifies a kernel that meets the interface
requirements for Vitis compiler to link the kernel with other kernels
and a target platform, but does not adhere to the requirements for
execution management by XRT. Refer to the following for more
information:
For data center acceleration: Creating User-Managed RTL
Kernels in the Data Center Acceleration using Vitis (UG1700)
For embedded design development: Creating User-Managed
Kernels in the Embedded Design Development Using Vitis
(UG1701)
```

#### Displayed in the footer

###### !!

##### ★

**Tag Attribute Description**

<port>

```
name
```
```
Specifies the port name.
Important: The AXI4-Lite interface must be named
S_AXI_CONTROL.
```
```
mode
```
```
At least one AXI4 master port and one AXI4-Lite slave control port are
required.
AXI4-Stream ports can be specified to stream data between kernels.
```
```
For AXI4 master port, set to "master."
For AXI4 slave port, set to "slave."
For AXI4-Stream master port, set to "write_only."
For AXI4-Stream slave port, set it "read_only."
```
```
range The range of the address space for the port.
```
```
dataWidth The width of the data that goes through the port, default is 32-bits.
```
```
portType Indicate whether or not the port is addressable or streaming.
```
```
For AXI4 master and slave ports, set it to "addressable."
For AXI4-Stream ports, set it to "stream."
```
```
base For AXI4 master and slave ports, set to 0x0. This tag is not applicable
to AXI4-Stream ports.
```
<arg>

```
name Specifies the kernel software argument name.
```
```
addressQualifier
```
```
Valid values:
```
```
0: Scalar kernel input argument
1: global memory
2: local memory
3: constant memory
4: pipe
```
```
id
```
```
Only applicable for AXI4 master and slave ports. The ID needs to be
sequential. It is used to determine the order of kernel arguments.
Not applicable for AXI4-Stream ports.
```
```
port Specifies the <port> name to which the arg is connected.
```
```
size Size of the argument in bytes. The default is 4 bytes.
```
```
offset Indicates the register memory address.
```
```
type
```
```
The C data type of the argument. For example, uint*, int*, or
float*.
```
```
hostOffset Reserved. Set to 0x0.
```
```
hostSize Size of the argument. The default is 4 bytes.
```
```
memSize
```
```
For AXI4-Stream ports, memSize sets the depth of the created FIFO.
```
```
Tip: Not applicable to AXI4 ports.
```
The following tags specify additional tags for AXI4-Stream ports. They do not apply to AXI4 ports.

<connection> The connection tag describes the actual connection in hardware, either from the kernel to the FIFO inserted for
the PIPE, or from the FIFO to the kernel.


#### Displayed in the footer

```
Tag Attribute Description
```
```
srcInst Specifies the source instance of the connection.
```
```
srcPort Specifies the port on the source instance for the connection.
```
```
dstInst Specifies the destination instance of the connection.
```
```
dstPort Specifies the port on the destination instance of the connection.
```
## platforminfo Utility

The platforminfo command line utility reports platform meta-data, including information on interface, clock, valid SLRs, allocated
resources and memory. The information is in a structured format. You can reference the information when, for example, allocating
kernels to SLRs or memory resources.
The following command options are available to use with platforminfo:

**Table: platforminfo Commands**

```
Option Description
```
```
-f [ --force ] Overwrite an existing output file.
```
```
-h [ --help ] Print help message and exit.
```
```
-k [ --keys ] Get keys for a given platform. Returns a list of JSON paths.
```
```
-l [ --list ] List platforms. Searches the user repo paths $PLATFORM_REPO_PATHS and then
the install locations to find .xpfm files.
```
```
-e [ --extended ] List platforms with extended information. Use with '--list'.
```
```
-d [ --hw ] <arg> Specify the platform definition (*.xsa) on which to operate. The value must be a full
path, including file name and .xsa extension.
```
```
-s [ --sw ] <arg> Specify the software platform definition (*.spfm) on which to operate. The value must
be a full path, including file name and .spfm extension.
```
```
-p [ --platform ] <arg> AMD platform definition (*.xpfm) on which to operate. The value for --platform can
be a full path including file name and .xpfm extension, as shown in example 1 below. If
supplying a file name and .xpfm extension without a path, this utility will search only the
current working directory. You can also specify just the base name for the platform.
When the value is a base name, this utility will search the
$PLATFORM_REPO_PATHS, and the install locations, to find a corresponding .xpfm
file, as shown in example 2 below.
```
```
Example 1: --platform
/opt/xilinx/platforms/xilinx_vck190_base_202520_1/xilinx_vck190_
base_202520_1.xpfm
```
```
Example 2: --platform xilinx_vck190_base_202520_1
```
```
-o [ --output ] <arg> Specify an output file. Default: STDOUT.
```
```
-j [ --json ] <arg> Use JSON format for output. When used with no value, prints the entire platform as
JSON. Accepts an optional adjacent value that specifies a JSON path. The JSON path,
when valid, fetches a JSON subtree, list, or value.
```
```
Example 1:
platforminfo --json="hardwarePlatform" --platform <platform base
name>
```
```
Example 2: Specify the index when referring to an item in a
```

#### Displayed in the footer

###### ✎

##### ★

```
Option Description
list.
platforminfo --json="hardwarePlatform.devices[0].name" --
platform <platform base name>
```
```
Example 3: When using the short option form (-j), the value must
follow immediately.
platforminfo -j"hardwarePlatform.systemClocks[]" -p <platform
base name>
```
-v [ --verbose ] The platforminfo utility can generate a default, human-readable report for a given
platform. The default report contains the most important characteristics of the platform.
Use the verbose switch to display the full platform meta-data in human-readable form.
**Note:** Except when using the --help or --list options, a platform must be specified. You can specify the platform using the --
platform option, or using either --hw, --sw. You can also simply insert the platform name or full path into the command line
positionally.
To understand the generated report, condensed output logs, based on the following command are reviewed. The report is broken down
into specific sections for better understandability.

```
platforminfo -p $PLATFORM_REPO_PATHS/xilinx_u200_gen3x16_xdma_2_202110_1.xpfm
```
```
Tip: See Platform info for vek385_base for an example of embedded processor platforms.
```
###### Basic Platform Information

Platform information and high-level description are reported.

```
Platform: gen3x16_xdma_2
File: $PLATFORM_REPO_PATHS/xilinx_u200_gen3x16_xdma_2_202110_1.xpfm
Description:
This platform targets the Alveo U200 Data Center Accelerator Card. This high-performance acceleration
platform features up to four channels of DDR4-2400 SDRAM which are instantiated as required by the
user kernels for high fabric resource availability, and Xilinx DMA Subsystem for PCI Express with PCIe
Gen3 x16 connectivity.
```
```
Hardware: 1
Has Software Platform(s): 1
Has Hardware Emulation: 1
```
###### Hardware Platform Information

General information on the hardware platform is reported. For the Hardware Emulation field, a "1" indicates this platform is suitable for
these configurations.

```
Vendor: xilinx
Board: U200 (gen3x16_xdma_2)
Name: gen3x16_xdma_2
Version: 202110.1
Generated Version: 2021.1
Is Extensible: 1
Supports Hardware Target: 1
Supports Hardware Emulation Target: 1
Is a Hardware Emulation Platform: 1
FPGA Family: virtexuplus
FPGA Device: xcu200
Board Vendor: xilinx.com
Board Name: xilinx.com:au200:1.3
Board Part: xcu200-fsgd2104-2-e
```

#### Displayed in the footer

###### Interface Information

The following shows the reported PCIe interface information.

```
Interface Name: PCIe
Interface Type: gen3x16
PCIe Vendor Id:
PCIe Device Id:
PCIe Subsystem Id:
```
###### Clock Information

Reports the maximum kernel clock frequencies available. The Clock Index is the reference used in the --kernel_frequency v++
directive when overriding the default value.

```
Default Clock Index: 0
Clock Index: 0
Frequency: 300.000000
Status: scalable
Clock Index: 1
Frequency: 500.000000
Status: scalable
Clock Index: 2
Frequency: 50.000000
Status: fixed
Clock Index: 3
Frequency: 250.000000
Status: fixed
Clock Index: 4
Frequency: 100.000000
Status: fixed
```
###### Valid SLRs

Reports the valid SLRs in the platform.

```
SLR0, SLR1, SLR2
```
###### Resource Availability

The total available resources and resources available per SLR are reported. This information can be used to assess applicability of the
platform for the design and help guide allocation of compute unit to available SLRs.

```
=====
Total
=====
LUTs: 979040
FFs: 1958080
BRAMs: 1860
DSPs: 5880
URAMs: 800
```
```
=======
Per SLR
=======
SLR0:
LUTs: 388160
FFs: 776320
BRAMs: 720
URAMs: 320
DSPs: 2280
SLR1:
LUTs: 205440
```

#### Displayed in the footer

```
FFs: 410880
BRAMs: 420
URAMs: 160
DSPs: 1320
SLR2:
LUTs: 385440
FFs: 770880
BRAMs: 720
URAMs: 320
DSPs: 2280
```
###### Memory Information

Reports the available DDR and PLRAM memory connections per SLR as shown in the example output below.

```
Type: ddr4
Bus SP Tag: DDR
Segment Index: 0
Consumption: automatic
SP Tag: bank0
SLR: SLR0
Max Masters: 15
Segment Index: 1
Consumption: default
SP Tag: bank1
SLR: SLR1
Max Masters: 15
Segment Index: 2
Consumption: automatic
SP Tag: bank2
SLR: SLR1
Max Masters: 15
Segment Index: 3
Consumption: automatic
SP Tag: bank3
SLR: SLR2
Max Masters: 15
Bus SP Tag: PLRAM
Segment Index: 0
Consumption: explicit
SLR: SLR0
Max Masters: 15
Segment Index: 1
Consumption: explicit
SLR: SLR1
Max Masters: 15
Segment Index: 2
Consumption: explicit
SLR: SLR2
Max Masters: 15
Bus SP Tag: HOST
Segment Index: 0
Consumption: explicit
SLR: SLR2
Max Masters: 15
```
The Bus SP Tag heading can be DDR or PLRAM and gives associated information below.
The Segment Index field is used in association with the SP Tag to generate the associated memory resource index as shown below.

```
Bus SP Tag[Segment Index]
```
For example, if Segment Index is 0, then the associated DDR resource index would be DDR[0].


#### Displayed in the footer

This memory index is used when specifying memory resources in the v++ command as shown below:

```
v++ ... --connectivity.sp vadd.m_axi_gmem:DDR[3]
```
There can be more than one Segment Index associated with an SLR. For instance, in the output above, SLR1 has both Segment
Index 1 and 2.
The Consumption field indicates how a memory resource is used when building the design:

**default**
If the --connectivity.sp directive is not specified, it uses this memory resource by default during v++ build. For example in
the report below, DDR with Segment Index 1 is used by default.

**automatic**
When the maximum number of memory interfaces are used and under Consumption: default is fully applied, then the
interfaces under automatic is used. The maximum number of interfaces per memory resource are given in the Max Masters
field.

**explicit**
For PLRAM, consumption is set to explicit which indicates this memory resource is only used when explicitly indicated through
the v++ command line.

###### Software Platform Information

Although software platform information is reported, it is only useful for users that have an OS running on the device, and not applicable
to users that use a host machine.

```
Number of Runtimes: 2
Default System Configuration: config0_0
System Configurations:
System Config Name: config0_0
System Config Description: config0_0 Linux OS on x86_0
System Config Default Processor Group: x86_0
System Config Default Boot Image:
System Config Is QEMU Supported: 0
System Config Processor Groups:
Processor Group Name: x86_0
Processor Group CPU Type: x86
Processor Group m_os Name: Linux OS
System Config Boot Images:
Supported Runtimes:
Runtime: C/C++
Runtime: XRT
```
###### Platform info for vek385_base

Use the following command to return the platforminfo for the vek385_base:

```
platforminfo -p $PLATFORM_REPO_PATHS/vek385_base
```
The results returned are as follows:

```
==========================
Basic Platform Information
==========================
Platform: vek385_base
File: <path_to_platform>/vek385_base.xpfm
Description: The VEK385 Vitis base platform is designed for the VEK385 evaluation kit,
enabling developers to create high-performance applications using AI and DSP engines that deliver over
10× the scalar compute performance of current server-class CPUs. This platform features a powerful
heterogeneous architecture with 8 Arm® Cortex®-A78 application processors, 10 Arm® Cortex®-R5 real-
time processors, and 20GB of high-speed LPDDR5X memory. It offers a robust foundation for building
advanced solutions in AI, signal processing, and compute-intensive domains using AMD Versal™ adaptive
SoCs.
```

#### Displayed in the footer

Hardware: 1
Has Software Platform(s): 1
Supports Hardware Emulation Target: 1
Has Hardware Emulation: 1

=====================================
Hardware Platform (Shell) Information
=====================================
Vendor: xilinx.com
Board: vek385_1_base
Name: vek385_1_base
Version: 1.0
Generated Version: 2025.2
Is Extensible: 1
Supports Hardware Target: 1
Is a Hardware Emulation Platform: 0
FPGA Family: versal
FPGA Device: xc2ve3858
Board Vendor: xilinx.com
Board Name: xilinx.com:vek385_1:1.0
Board Part: xc2ve3858-ssva2112-2MP-e-S

==============
AIE Partitions
==============
Start Col: 0
# Columns: 36

=====================
Available Resources
=====================
NoC PL NSU: 24
NoC PL NMU: 30

=================
Clock Information
=================
Default Clock Index: 1
Clock Index: 0
Frequency: 624.993750
Status: fixed_non_ref
Clock Index: 1
Frequency: 312.496875
Status: fixed_non_ref
Clock Index: 2
Frequency: 156.248437
Status: fixed_non_ref
Clock Index: 3
Frequency: 78.124218
Status: fixed_non_ref
Clock Index: 4
Frequency: 99.999000
Status: fixed

========================
AIE Hardware Information
========================
Arch: AIE2PS
NPI Base Address: 0xf6d50000
AXI Base Address: 0x20000000000
Shim Row Start: 0 # Rows: 1
Core Row Start: 3 # Rows: 4
Mem Row Start: 1 # Rows: 2


#### Displayed in the footer

==========
Valid SLRs
==========
SLR0

=====================
Resource Availability
=====================
=====
Total
=====
LUTs: 542394
FFs: 1085371
BRAMs: 1342
DSPs: 2064

==================
Memory Information
==================
Bus SP Tag: AIE
Bus SP Tag: LPDDR
Bus SP Tag: LPDDR01
Bus SP Tag: LPDDR23
Bus SP Tag: LPDDR4

=============================
Software Platform Information
=============================
Number of Runtimes: 2
Default System Configuration: vek385_base
System Configurations:
System Config Name: vek385_base
System Config Description: The VEK385 Vitis base platform is designed for the VEK385
evaluation kit, enabling developers to create high-performance applications using AI and DSP engines
that deliver over 10× the scalar compute performance of current server-class CPUs. This platform
features a powerful heterogeneous architecture with 8 Arm® Cortex®-A78 application processors, 10 Arm®
Cortex®-R5 real-time processors, and 20GB of high-speed LPDDR5X memory. It offers a robust foundation
for building advanced solutions in AI, signal processing, and compute-intensive domains using AMD
Versal™ adaptive SoCs.
System Config Default Processor Group: xrt
System Config Default Boot Image: standard
System Config Is QEMU Supported: 1
System Config Processor Groups:
Processor Group Name: aiengine
Processor Group CPU Type: ai_engine
Processor Group m_os Name: aiengine
Processor Group Name: xrt
Processor Group CPU Type: cortex-a78
Processor Group m_os Name: xrt
System Config Boot Images:
Boot Image Name: standard
Boot Image Type:
Boot Image BIF: boot/
Boot Image Data: xrt/image
Boot Image Boot Mode:
Boot Image RootFileSystem:
Boot Image Mount Path:
Boot Image Read Me:
Boot Image QEMU Args: qemu/pmc_args.txt:qemu/qemu_args.txt
Boot Image QEMU Boot:
Boot Image QEMU Dev Tree:
Supported Runtimes:


#### Displayed in the footer

##### ★

```
Runtime: C/C++
Runtime: XRT
```
## vitis-run Command

The vitis-run command is provided to enable C-simulation, C/RTL Co-simulation, and Vivado implementation of an HLS component
created with the v++ -c --mode hls command.
The options of the vitis-run command include:

**--mode hls**
Specifies the use of vitis-run for HLS components. This is the only mode currently supported.

**--csim**
Run C-simulation on an HLS component.

**--cosim**
Run C/RTL Co-simulation on an HLS component.

**--package**
Run the package process to generate the IP or XO files from the RTL design of the HLS component. This generates the required
export files to use the RTL design in the Vivado Design Suite, or Vitis development flow.

**--impl**
Run Vivado implementation out-of-context (OOC) run on the HLS component. This is used to provide resource usage and timing
estimates from the synthesized or implemented design.

**--tcl**
Run Vitis HLS using the Tcl scripting language. Tcl commands are described in the _Vitis High-Level Synthesis User Guide_
(UG1399) under the heading of Vitis HLS Command Reference.

**--work_dir**
Specify the working directory. For -cosim and -impl, the specified working directory must contain a previously compiled HLS
component.

**--config arg**
Specify a config file for use with the tool. Refer to v++ Mode HLS for specific commands to use in the config file.

**-h [ --help ]**
Display command help for the tool.

```
Tip: The list of options above is not complete list. You can use the --help command to display the complete list of vitis-run
commands.
```
You can use the vitis-run command to run C-simulation without an existing design, or run C/RTL Co-simulation or Vivado
implementation on an existing HLS component. Some examples follow.

### Run C-Simulation

You can run C-simulation on an existing work directory containing a previously compiled HLS component, or specify a new work
directory to run C-simulation on the source files directly. The config file for an existing HLS component only needs to specify commands
for C-Simulation as described in C-Simulation Configuration. The previously built HLS component provides the foundation for
simulation, defining the source files, test bench files, and part or platform for the design. Running C-simulation on a new work directory
requires a complete config file, specifying the required source files and part, in addition to any C-simulation options.
An example command line:

```
vitis-run --mode hls --csim --config ./hls_csim.cfg --work_dir newTest
```
The example config file:

```
part=xcvu11p-flga2577-1-e
```
```
[hls]
clock=8
flow_target=vitis
```

#### Displayed in the footer

```
syn.file=../../src/dct.cpp
syn.top=dct
tb.file=../../src/out.golden.dat
tb.file=../../src/in.dat
tb.file=../../src/dct_test.cpp
tb.file=../../src/dct_coeff_table.txt
syn.output.format=xo
clock_uncertainty=15%
csim.O=true
csim.code_analyzer=0
csim.clean=true
csim.profile=true
```
### Run C/RTL Co-Simulation

You can run Co-simulation on an existing work directory containing a previously compiled HLS component, or specify a new work
directory to run C-simulation on the source files directly. The config file for an existing HLS component only needs to specify commands
for C-Simulation as described in Co-Simulation Configuration. The previously built HLS component provides the foundation for
simulation, defining the source files, test bench files, and part or platform for the design. Running C-simulation on a new work directory
requires a complete config file, specifying the required source files and part, in addition to any C-simulation options.
An example command line:

```
vitis-run --mode hls --cosim --config ./cosim.cfg --work_dir myHLS
```
The example config file:

```
part=xcvu11p-flga2577-1-e
```
```
[HLS]
clock=8
flow_target=vitis
syn.file=/group/xcoswmktg/randyh/rigel-tests/03-Vitis_HLS/reference-files/src/dct.cpp
syn.top=dct
tb.file=/group/xcoswmktg/randyh/rigel-tests/03-Vitis_HLS/reference-files/src/out.golden.dat
tb.file=/group/xcoswmktg/randyh/rigel-tests/03-Vitis_HLS/reference-files/src/in.dat
tb.file=/group/xcoswmktg/randyh/rigel-tests/03-Vitis_HLS/reference-files/src/dct_test.cpp
tb.file=/group/xcoswmktg/randyh/rigel-tests/03-Vitis_HLS/reference-files/src/dct_coeff_table.txt
syn.output.format=xo
clock_uncertainty=15%
cosim.trace_level=port
#cosim.wave_debug=true
cosim.random_stall=true
cosim.enable_dataflow_profiling=true
```
### Run Vivado Implementation

You can run the Vivado Design Suite to synthesize and run place and route on the RTL generated by the HLS synthesis process. The
synthesis and implementation processes are managed by commands specified in the configuration file as described in Implementation
Configuration.
An example command line:

```
vitis-run --mode hls --impl --config ./impl.cfg --work_dir myHLS
```
The example config file:

```
part=xcvu11p-flga2577-1-e
```
```
[HLS]
clock=8
flow_target=vitis
```

#### Displayed in the footer

##### ★

```
syn.file=/group/xcoswmktg/randyh/rigel-tests/03-Vitis_HLS/reference-files/src/dct.cpp
syn.top=dct
tb.file=/group/xcoswmktg/randyh/rigel-tests/03-Vitis_HLS/reference-files/src/out.golden.dat
tb.file=/group/xcoswmktg/randyh/rigel-tests/03-Vitis_HLS/reference-files/src/in.dat
tb.file=/group/xcoswmktg/randyh/rigel-tests/03-Vitis_HLS/reference-files/src/dct_test.cpp
tb.file=/group/xcoswmktg/randyh/rigel-tests/03-Vitis_HLS/reference-files/src/dct_coeff_table.txt
syn.output.format=xo
clock_uncertainty=15%
cosim.trace_level=port
#cosim.wave_debug=true
cosim.random_stall=true
cosim.enable_dataflow_profiling=true
```
### Run Tcl Script

You can also use vitis-run to run an existing Tcl script, such as the script.tcl from an existing project, to build the project and then
write a config file from the Tcl script. The example below shows the vitis-run command to perform these actions.
An example command line:

```
vitis-run --mode hls --tcl dct-build.tcl
```
An example Tcl script:

```
open_project dctProj
set_top dct
add_files ../03-Vitis_HLS/reference-files/src/dct.cpp
add_files -tb ../03-Vitis_HLS/reference-files/src/dct_coeff_table.txt
add_files -tb ../03-Vitis_HLS/reference-files/src/dct_test.cpp
add_files -tb ../03-Vitis_HLS/reference-files/src/in.dat
add_files -tb ../03-Vitis_HLS/reference-files/src/out.golden.dat
open_solution "solution1" -flow_target vivado
set_part {xcvu11p-flga2577-1-e}
create_clock -period 10 -name default
csynth_design
write_ini ./dct-build.cfg
exit
```
**Tip:** The addition of the write_ini command at the end of the script creates a config file from the Tcl script, thus providing you
with a config file to use with v++ -c --mode hls.

## xrt-smi Utility

The xrt-smi utility is a standalone command line utility that is included with the Xilinx Runtime (XRT) installation package. Details of the
xrt-smi command can be found at https://xilinx.github.io/XRT/master/html/xrt-smi.html.
xrt-smi includes multiple commands to validate and identify the installed accelerator card(s) along with additional card details
including on card memory, host interface, target platform name, and system information. This information can be used for both card
administration and application debugging.
Accelerator cards are partitioned into a user function and a management function to provide different levels of card access. The user
function lets you load and run applications, while the management function is for system administrators to manage the card. The xrt-
smi utility interacts with the user function. The xbmgmt utility, which requires root privilege, is for interacting with the management
function. The reason for splitting the function access between the two utilities is to provide some security for the management features
of the tool.
You can use the help command to list the available xrt-smi commands and options:

```
xrt-smi --help
```
Set up the xrt-smi command as part of the XRT installation using the following scripts:


#### Displayed in the footer

```
For csh shell:
```
```
$ source /opt/xilinx/xrt/setup.csh
```
```
For bash shell:
```
```
$ source /opt/xilinx/xrt/setup.sh
```
## xbmgmt Utility

AMD Board Management (xbmgmt) utility is a standalone command line tool that is included with the Xilinx Runtime (XRT) installation
package. Details of the xbmgmt command can be found at https://xilinx.github.io/XRT/master/html/xbmgmt.html.
Accelerator cards are partitioned into a user function and a management function to provide different levels of card access. The user
function lets you load and run applications, while the management function is for system administrators to manage the card. The xrt-
smi utility interacts with the user function.
The xbmgmt utility is used for card installation and administration, and requires sudo privileges when running it. The xbmgmt supported
tasks include flashing the card firmware, and scanning the current device configuration.
You can use the help command to list the available xbmgmt commands and options, and access help for individual commands by
using the following:

```
xbmgmt --help <command>
```
For detailed help of each sub-command, use the following:

```
xbmgmt help <subcommand>
```
Set up the xbmgmt command as part of the XRT installation using the following scripts:

```
For csh shell:
```
```
$ source /opt/xilinx/xrt/setup.csh
```
```
For bash shell:
```
```
$ source /opt/xilinx/xrt/setup.sh
```
## xclbinutil Utility

The xclbinutil utility can create, modify, and report xclbin content information.
The available command options are shown in the following table.

**Table: xclbinutil Commands**

```
Option Description
```
```
-h [ --help ] Print help messages.
```
```
-i [ --input ]<arg> Input file name. Reads xclbin into memory.
```
```
-o [ --output ]<arg> Output file name. Writes in memory xclbin image to a file.
```
```
--target <arg> Target flow for this image. Valid values: hw, and hw_emu.
```
```
----private-key <arg> Private key used in signing the xclbin image.
```
```
--ceritifcate <arg> Certificate used in signing and validating the xclbin image.
```
```
--digest-algorithm <arg> Digest algorithm. Default: sha512
```
```
--validate-signature Validates the signature for the given xclbin archive.
```
```
-v [ --verbose ] Display verbose/debug information
```

#### Displayed in the footer

```
Option Description
```
```
-q [ --quiet ] Minimize reporting information.
```
```
--migrate-forward Migrate the xclbin archive forward to the new binary format.
```
```
--add-section <arg> Section name to add to the xclbin image. Format: <section>:<format>:<file>
```
```
--add-replace-section <arg> Replace an existing section or add the section of the xclbin image if it does not exist.
Format: <section>:<format>:<file>
```
```
--add-merge-section <arg> Add the section if it does not exist, or merge content with an existing section. Format:
<section>:<format>:<file>
```
```
--remove-section<arg> Section name to remove from the xclbin image.
```
```
--dump-section<arg> Section to dump. Format: <section>:<format>:<file>
```
```
--replace-section<arg> Section to replace.
```
```
--key-value<arg> Key value pairs. Format: [USER|SYS]:<key>:<value>
```
```
--remove-key<arg> Removes the given user key from the xclbin archive.
```
```
--add-signature<arg> Adds a user defined signature to the given xclbin image.
```
```
--remove-signature Removes the signature from the xclbin image.
```
```
--get-signature Returns the user defined signature (if set) of the xclbin image.
```
```
--info Report accelerator binary content. Including: generation and packaging data, kernel
signatures, connectivity, clocks, sections, etc
```
```
--list-sections List all possible section names (standalone option).
```
```
--version Version of this executable.
```
```
--force Forces a file overwrite.
```
The following are various use examples of the tool.

**Reporting xclbin information**

```
xclbinutil --info --input binary_container_1.xclbin
```
**Extracting the bitstream image**

```
xclbinutil --dump-section BITSTREAM:RAW:bitstream.bit --input binary_container_1.xclbin
```
**Extracting the build metadata**

```
xclbinutil --dump-section BUILD_METADATA:HTML:buildMetadata.json --input binary_container_1.xclbin
```
**Removing a section**

```
xclbinutil --remove-section BITSTREAM --input binary_container_1.xclbin --output
binary_container_modified.xclbin
```
For most users, details about the contents and how the xclbin was created is desired. This information can be obtained through the -
-info option and reports information on the xclbin, hardware platform, clocks, memory configuration, kernel, and how the xclbin
was generated.
The output of the xclbinutil command using the --info option is shown below divided into sections.

```
xclbinutil -i binary_container_1.xclbin --info
```

#### Displayed in the footer

###### xclbin Information

```
Generated by: v++ (2020.1) on Mon Apr 13 20:19:40 MDT 2020
Version: 2.6.436
Kernels: CopyKernel
Signature:
Content: Bitstream
UUID (xclbin): d081de98-3fd3-4e9b-bab3-108b42c73101
UUID (IINTF): 862c7020a250293e32036f19956669e5
Sections: DEBUG_IP_LAYOUT, BITSTREAM, MEM_TOPOLOGY, IP_LAYOUT,
CONNECTIVITY, CLOCK_FREQ_TOPOLOGY, BUILD_METADATA,
EMBEDDED_METADATA, SYSTEM_METADATA, PARTITION_METADATA
```
###### Hardware Platform Information

```
Vendor: xilinx
Board: u200
Name: xdma
Version: 201830.1
Generated Version: Vivado 2018.3 (SW Build: 2388429)
Created: Wed Nov 14 20:06:10 2018
FPGA Device: xcu200
Board Vendor: xilinx.com
Board Name: xilinx.com:au200:1.0
Board Part: xilinx.com:au200:part0:1.0
Platform VBNV: xilinx_u200_xdma_201830_1
Static UUID: 00194bb3-707b-49c4-911e-a66899000b6b
Feature ROM TimeStamp: 1542252769
```
###### Clocks

Reports the maximum kernel clock frequencies available. Both the clock names and clock indexes are provided. The clock indexes are
identical to those reported in platforminfo Utility.

```
Name: DATA_CLK
Index: 0
Type: DATA
Frequency: 300 MHz
```
```
Name: KERNEL_CLK
Index: 1
Type: KERNEL
Frequency: 500 MHz
```
###### Memory Configuration

```
Name: bank0
Index: 0
Type: MEM_DDR4
Base Address: 0x0
Address Size: 0x400000000
Bank Used: No
```
```
Name: bank1
Index: 1
Type: MEM_DDR4
Base Address: 0x400000000
Address Size: 0x400000000
Bank Used: Yes
```
```
Name: bank2
```

#### Displayed in the footer

```
Index: 2
Type: MEM_DDR4
Base Address: 0x800000000
Address Size: 0x400000000
Bank Used: No
```
```
Name: bank3
Index: 3
Type: MEM_DDR4
Base Address: 0xc00000000
Address Size: 0x400000000
Bank Used: No
```
```
Name: PLRAM[0]
Index: 4
Type: MEM_DDR4
Base Address: 0x1000000000
Address Size: 0x20000
Bank Used: No
```
```
Name: PLRAM[1]
Index: 5
Type: MEM_DRAM
Base Address: 0x1000020000
Address Size: 0x20000
Bank Used: No
```
```
Name: PLRAM[2]
Index: 6
Type: MEM_DRAM
Base Address: 0x1000040000
Address Size: 0x20000
Bank Used: No
```
###### Kernel Information

For each kernel in the xclbin, the function definition, ports, and instance information is reported.
The following is an example of the reported function definition.

```
Definition
----------
Signature: krnl_vadd (int* a, int* b, int* c,
int const n_elements)
```
The following is an example of the reported ports.

```
Ports
-----
Port: M_AXI_GMEM
Mode: master
Range (bytes): 0xFFFFFFFF
Data Width: 32 bits
Port Type: addressable
```
```
Port: M_AXI_GMEM1
Mode: master
Range (bytes): 0xFFFFFFFF
Data Width: 32 bits
Port Type: addressable
```
```
Port: S_AXI_CONTROL
Mode: slave
```

#### Displayed in the footer

```
Range (bytes): 0x1000
Data Width: 32 bits
Port Type: addressable
```
The following is an example of the reported instance(s) of the kernel.

```
Instance: krnl_vadd_1
Base Address: 0x0
```
```
Argument: a
Register Offset: 0x10
Port: M_AXI_GMEM
Memory: bank1 (MEM_DDR4)
```
```
Argument: b
Register Offset: 0x1C
Port: M_AXI_GMEM
Memory: bank1 (MEM_DDR4)
```
```
Argument: c
Register Offset: 0x28
Port: M_AXI_GMEM1
Memory: bank1 (MEM_DDR4)
```
```
Argument: n_elements
Register Offset: 0x34
Port: S_AXI_CONTROL
Memory: <not applicable>
```
###### Tool Generation Information

The utility also reports the v++ command line used to generate the xclbin. The Command Line section gives the actual v++
command line used, while the Options section displays each option used in the command line, but in a more readable format with one
option per line.

```
$ xclbinutil --input build_dir.hw_emu.xilinx_u250_gen3x16_xdma_4_1_202210_1/vadd.xclbin --info
XRT Build Version: 2.19.185 (2025.2)
Build Date: 2025-04-27 06:39:04
Hash ID: 6d79c5f0e8c3f0ca8c08d35f2cfe6908ca0752a3
------------------------------------------------------------------------------
Warning: The option '--output' has not been specified. All operations will
be done in memory with the exception of the '--dump-section' command.
------------------------------------------------------------------------------
Reading xclbin file into memory. File:
build_dir.hw_emu.xilinx_u250_gen3x16_xdma_4_1_202210_1/vadd.xclbin
```
```
==============================================================================
XRT Build Version: 2.19.185 (2025.2)
Build Date: 2025-04-27 06:39:04
Hash ID: 6d79c5f0e8c3f0ca8c08d35f2cfe6908ca0752a3
==============================================================================
xclbin Information
------------------
Generated by: v++ (2025.2) on 2025-04-27-00:01:42
Version: 2.19.185
Kernels: vadd
Signature:
Content: HW Emulation Binary
UUID (xclbin): 65022735-bba0-8345-3e99-a17768a9913e
Sections: BITSTREAM, MEM_TOPOLOGY, IP_LAYOUT, CONNECTIVITY,
CLOCK_FREQ_TOPOLOGY, BUILD_METADATA,
```

#### Displayed in the footer

EMBEDDED_METADATA, SYSTEM_METADATA,
GROUP_CONNECTIVITY, GROUP_TOPOLOGY
==============================================================================
Hardware Platform (Shell) Information
-------------------------------------
Vendor: xilinx
Board: u250
Name: gen3x16_xdma_4_1
Version: 202210.1
Generated Version: Vivado 2022.1 (SW Build: 3510589)
Created:
Thu Mar 31 07:42:58 2022 FPGA Device: xcu250
Board Vendor: xilinx.com
Board Name: xilinx.com:au250:1.4
Board Part: xilinx.com:au250:part0:1.4
Platform VBNV: xilinx_u250_gen3x16_xdma_4_1_202210_1
Static UUID: 00000000-0000-0000-0000-000000000000
Feature ROM TimeStamp: 0

Scalable Clocks
---------------
Name: DATA_CLK
Index: 0
Type: DATA
Frequency: 300 MHz

Name: KERNEL_CLK
Index: 1
Type: KERNEL
Frequency: 300 MHz

System Clocks
------
Name: ii_level1_wire_ulp_m_aclk_ctrl_00
Type: FIXED
Default Freq: 50 MHz

Name: ii_level1_wire_ulp_m_aclk_pcie_user_00
Type: FIXED
Default Freq: 250 MHz

Name: ii_level1_wire_ulp_m_aclk_freerun_ref_00
Type: FIXED
Default Freq: 100 MHz

Name: ss_ucs_aclk_kernel_00
Type: SCALABLE
Default Freq: 300 MHz
Requested Freq: 0 MHz
Achieved Freq: 0 MHz

Name: ss_ucs_aclk_kernel_01
Type: SCALABLE
Default Freq: 500 MHz
Requested Freq: 0 MHz
Achieved Freq: 0 MHz

Memory Configuration
--------------------
Name: bank0
Index: 0
Type: MEM_DDR4
Base Address: 0x4000000000


#### Displayed in the footer

Address Size: 0x400000000
Bank Used: No

Name: bank1
Index: 1
Type: MEM_DDR4
Base Address: 0x5000000000
Address Size: 0x400000000
Bank Used: Yes

Name: bank2
Index: 2
Type: MEM_DRAM
Base Address: 0x6000000000
Address Size: 0x400000000
Bank Used: No

Name: bank3
Index: 3
Type: MEM_DRAM
Base Address: 0x7000000000
Address Size: 0x400000000
Bank Used: No

Name: PLRAM[0]
Index: 4
Type: MEM_DRAM
Base Address: 0x3000000000
Address Size: 0x20000
Bank Used: No

Name: PLRAM[1]
Index: 5
Type: MEM_DRAM
Base Address: 0x3000200000
Address Size: 0x20000
Bank Used: No

Name: PLRAM[2]
Index: 6
Type: MEM_DRAM
Base Address: 0x3000400000
Address Size: 0x20000
Bank Used: No

Name: PLRAM[3]
Index: 7
Type: MEM_DRAM
Base Address: 0x3000600000
Address Size: 0x20000
Bank Used: No

Name: HOST[0]
Index: 8
Type: MEM_DRAM
Base Address: 0x2000000000
Address Size: 0x400000000
Bank Used: No
==============================================================================
Kernel: vadd

Definition
----------


#### Displayed in the footer

Signature: vadd (void* in1, void* in2, void* out, unsigned int size)

Ports
-----
Port: M_AXI_GMEM0
Mode: master
Range (bytes): 0xFFFFFFFF
Data Width: 32 bits
Port Type: addressable

Port: M_AXI_GMEM1
Mode: master
Range (bytes): 0xFFFFFFFF
Data Width: 32 bits
Port Type: addressable

Port: S_AXI_CONTROL
Mode: slave
Range (bytes): 0x3C
Data Width: 32 bits
Port Type: addressable

--------------------------
Instance: vadd_1
Base Address: 0x1d010000

Argument: in1
Register Offset: 0x10
Port: M_AXI_GMEM0
Memory: bank1 (MEM_DDR4)

Argument: in2
Register Offset: 0x1C
Port: M_AXI_GMEM1
Memory: bank1 (MEM_DDR4)

Argument: out
Register Offset: 0x28
Port: M_AXI_GMEM0
Memory: bank1 (MEM_DDR4)

Argument: size
Register Offset: 0x34
Port: S_AXI_CONTROL
Memory: <not applicable>
==============================================================================
Generated By
------------
Command: v++
Version: 2025.2 - 2025-04-27-00:01:42 (SW BUILD: 6117406)
Command Line: v++ --debug --input_files _x.hw_emu.xilinx_u250_gen3x16_xdma_4_1_202210_1/vadd.xo --
link --optimize 0 --output ./build_dir.hw_emu.xilinx_u250_gen3x16_xdma_4_1_202210_1/vadd.link.xclbin -
-platform xilinx_u250_gen3x16_xdma_4_1_202210_1 --report_level 0 --save-temps --target hw_emu --
temp_dir ./_x.hw_emu.xilinx_u250_gen3x16_xdma_4_1_202210_1
Options: --debug
--input_files _x.hw_emu.xilinx_u250_gen3x16_xdma_4_1_202210_1/vadd.xo
--link
--optimize 0
--output ./build_dir.hw_emu.xilinx_u250_gen3x16_xdma_4_1_202210_1/vadd.link.xclbin
--platform xilinx_u250_gen3x16_xdma_4_1_202210_1
--report_level 0
--save-temps
--target hw_emu


#### Displayed in the footer

##### ★

```
--temp_dir ./_x.hw_emu.xilinx_u250_gen3x16_xdma_4_1_202210_1
==============================================================================
User Added Key Value Pairs
--------------------------
<empty>
==============================================================================
Leaving xclbinutil.
```
## xrt.ini File

The Xilinx Runtime (XRT) library uses various control parameters to specify debugging, profiling, and message logging when running
the host application and kernel execution. These control parameters are specified in a runtime initialization file, xrt.ini and used to
configure features of XRT at start-up.
If you are a command line user, the xrt.ini file needs to be created manually and saved to the same directory as the host executable.
The runtime library checks if xrt.ini exists in the same directory as the host executable and automatically reads the file to configure the
runtime. You can also specify the location of an xrt.ini file at runtime by setting the XRT_INI_PATH environment variable to point to the
file, for example:

```
export XRT_INI_PATH=/path/to/xrt.ini
```
**Tip:** The AMD Vitis™ IDE creates an xrt.ini file automatically based on your run configuration and saves it in the run configuration
folder.

### Runtime Initialization File Format

The xrt.ini file is a simple text file with groups of keys and their values. Any line beginning with a semicolon (;) or a hash (#) is a
comment. The group names, keys, and key values are all case sensitive.
The following is an example xrt.ini file that enables the timeline trace feature, and directs the runtime log messages to the Console view.

```
#Start of Debug group
[Debug]
native_xrt_trace = true
device_trace = fine
```
```
#Start of Runtime group
[Runtime]
runtime_log = console
```
There are three groups of initialization keys:

```
Runtime
Debug
AIE_profile_settings
AIE_trace_settings
Emulation
```
The following tables list all supported keys for each group, the supported values for each key, and a short description of the purpose of
the key.

### Runtime Group

The Runtime group of switches lets you configure elements of the runtime operation as described below.

**Table: Runtime Group Keys and Values**

```
Key Valid Values Description
```
```
api_checks [true|false] Enables or disables OpenCL API checks.
```

#### Displayed in the footer

```
Key Valid Values Description
true: Enable. This is the default value.
false: Disable.
```
```
cpu_affinity {N,N,...} Pins all runtime threads to specified CPUs.
Example:
```
```
cpu_affinity = {4,5,6}
```
```
exclusive_cu_context [true|false] This allows the host application to direct
OpenCL to acquire exclusive CU access, so
that low-level AXI read/write (xclRegRead and
xclRegWrite) can be used for regular kernels.
```
```
force_program_xclbin [true|false] Forces a reconfigurable module (RM) .xclbin
file to be reloaded into the device, even if it is
already loaded. This is used to force the
reloading of a DFX region in the platform with
the .xclbin when requested.
```
```
runtime_log [null | console | syslog |
<filename>]
```
```
Specifies where the runtime logs are printed
```
```
null: Do not print any logs. This is the
default value.
console: Print logs to stdout
syslog: Print logs to Linux syslog.
<filename>: Print logs to the specified
file. For example,
runtime_log=my_run.log.
```
```
verbosity [0 | 1 | 2 | 3 | 4 | 5 | 6 | 7
]
```
```
Verbosity of the log messages. The default
value is 4.
```
### Debug Group

The Debug group of switches define key options for the enabling profiling of the application during runtime, or tracing data transfers and
execution. These switches apply to both AI Engine and PL kernels in the Vitis acceleration flow, and let you configure aspects of the
runtime to control the frequency of data capture, the events to capture, and the amount of memory to reserve or use for recording trace
and profile data.

**Table: Debug Options**

```
Key Valid Values Description
```
```
aie_profile [true|false] Enables the runtime configuration and polling
of AI Engine hardware performance counters.
Available on hardware and hardware
emulation runs.
```
```
true: Enable.
false: Disable. This is the default value.
```

#### Displayed in the footer

###### ✎

##### ★

```
Key Valid Values Description
```
aie_trace [true|false] Enables the runtime configuration and
collection of AI Engine event trace. Available
on hardware runs only.

```
true: Enable.
false: Disable. This is the default value.
```
aie_status [true|false] Enables the polling of AI Engine status
information. Available on hardware and
hardware emulation runs.

aie_status_interval_us integer (default=1000us) Controls the interval at which AI Engine status
information is captured. Specified in
microseconds.

continuous_trace [true|false] Enables the continuous dumping of files for
trace and the continuous reading of device
data into the host.

```
true: Enable.
false: Disable. This is the default value.
```
```
Note: This switch only has an effect if
device_trace is enabled.
```
device_counters [true|false] Enables device counter offload only, without
enabling trace functionality.

device_trace [off|fine|coarse|accel] Enables the collection of data from monitors
inserted on the PL to add to summary and
trace.

```
accel: Traces compute unit starts/stops.
coarse: Lumps all reads/writes together
under each execution of a compute unit.
fine: Tracks everything as it happens.
off: Turns off reading and reporting of
device-level trace during runtime. This is
the default value.
```
host_trace [true|false] Enables trace of host code based on the first
protocol encountered.

```
Tip: If your host application uses XRT
native API you should manually specify
native_xrt_trace to capture all events.
```
native_xrt_trace [true|false] Enables generation of the Native C/C++ API
trace. This also generates the tables for "Host
Data Transfer from/to Global memory" in the
Profile Summary.

```
true: Enable.
false: Disable. This is the default value.
```
pl_deadlock_detection [true|false] Enables deadlock detection for PL kernels.

power_profile [true|false] Enables the polling of power data during the
execution of the application.


#### Displayed in the footer

###### ✎ ✎ ✎ ✎ ✎ ✎

```
Key Valid Values Description
true: Enable.
false: Disable. This is the default value.
```
```
Note: This feature is not supported on
embedded platforms or AWS.
```
power_profile_interval_ms <int>(default=20) Controls the interval of reading the power
counters in milliseconds. The default interval
is 20 ms.
**Note:** This switch only has an effect if
power_profile = true.

profile_api [true|false] Enables access to HAL API directly from the
host application to read counters on device
profiling monitors during execution.

```
true: Enable.
false: Disable. This is the default value.
```
stall_trace [off|all|dataflow|memory|pipe] Specifies the type of device-side stalls to
capture and report in the timeline trace. The
default is off.

```
off: Turn off stall trace information.
all: Record all stall trace information.
dataflow: Intra-kernel streams (for
example, writing to full FIFO between
dataflow blocks).
memory: External memory stalls (for
example, AXI4 read from the DDR
memory).
```
```
Note: This switch only has an effect if
device_trace is enabled.
```
trace_buffer_offload_interval_ms<int> Controls the reading of device data from the
device to the host in milliseconds (ms). The
default is 10 ms.
**Note:** This switch only has an effect if
device_trace is enabled.

trace_buffer_size <string> If the .xclbin was created with memory offload
of trace specified, as described in --profile
Options,this switch determines the size of the
buffer to allocate in memory to capture trace
data. The default is 1M.
**Note:** This switch only has an effect if
device_trace is enabled.

trace_file_dump_interval_s <int> Controls the time between dumping of trace
files in seconds (s). The default is 5s.
**Note:** This switch only has an effect if
device_trace is enabled.


#### Displayed in the footer

###### !!

###### !!

```
Key Valid Values Description
```
```
vitis_ai_profile [true|false] Profile summary and other files come from
Vitis AI application layer.
```
```
true: Enable.
false: Disable. This is the default value.
```
```
xocl_debug [true|false] Generates the xocl.log file when enabled.
When any trace options are also enabled, the
debug log is added to the xrt.run_summary to
view in Vitis Analyzer.
```
```
xrt_trace [true|false] Enables generation of low-level HW shim
function trace during HW runs. This will be
disabled when used with
native_xrt_trace.
```
```
true: Enable.
false: Disable. This is the default value.
```
### AIE_profile_settings Group

The options specified in this group are applied only if aie_profile=true under the [Debug] group.

**Table: AI Engine Profile Options**

```
Key Valid Values Description
```
```
graph_based_aie_metrics <graph name|all>:<kernel name|all>:
<off|heat_map|stalls|execution|floating_point|write_bandwidths|read_bandwidths|aie_trace>
```
```
Specify the metric sets reported by the AI
Engine module of AI Engine tiles on a graph-
by-graph basis.
Important: Currently, only all is supported
for kernel specification.
Controls the configuration of the statistics read
from the AIE core performance counters for
the entire AI Engine graph application.
heat_map: profile active/stall cycles and
vector instruction usage
stalls: profile the different types of stalls (i.e.,
memory, stream, lock, and cascade)
execution: profile the AI Engine instructions
floating_point: profile floating point exceptions
write_bandwidths: profile the write bandwidth
of streams and cascades
read_bandwidths: profile the read bandwidths
of streams and cascades
aie_trace: profile amount and stalls of event
trace from core and memory modules
```
```
graph_based_aie_memory_metrics<graph name|all>:<kernel name|all>:
<off|conflicts|dma_locks|dma_stalls_s2mm|dma_stalls_mm2s|write_bandwidths|read_bandwidths>
```
```
Specify the metric sets reported by the
memory module of AI Engine tiles on a graph-
by-graph basis.
Important: Currently, only all is supported
for kernel specification.
Controls the configuration of statistics read
from the AI Engine memory performance
```

#### Displayed in the footer

###### ✎

###### ✎

```
Key Valid Values Description
counters for the entire AI Engine graph
application.
conflicts: profile the DMA memory conflicts
dma_locks: profile DMA locks and stalls on
lock acquire
dma_stalls_s2mm: profile stalls on DMA
S2MM channels
dma_stalls_mm2s: profile stalls on DMA
MM2S channels
write_bandwidths: profile bandwidths of DMA
S2MM channels
read_bandwidths: profile bandwidths of DMA
MM2S channels
```
```
tile_based_aie_metrics <{<column>,<row>}|all>:
<off|heat_map|stalls|execution|floating_point|write_bandwidths|read_bandwidths|aie_trace>
;
{<mincolumn,<minrow>}:{<maxcolumn>,
<maxrow>}:
<off|heat_map|stalls|execution|floating_point|write_bandwidths|read_bandwidths|aie_trace>
```
```
Specify the metric sets reported by the AI
Engine module of AI Engine tiles on a tile-by-
tile basis. This can be used in conjunction with
graph-by-graph selection and will take priority
on the specified tiles.
Refer to descriptions from
graph_based_aie_metrics
```
```
tile_based_aie_memory_metrics<{<column>,<row>}|all>:
<off|conflicts|dma_locks|dma_stalls_s2mm|dma_stalls_mm2s|write_bandwidths|read_bandwidths>
;
{<mincolumn,<minrow>}:{<maxcolumn>,
<maxrow>}:
<off|conflicts|dma_locks|dma_stalls_s2mm|dma_stalls_mm2s|write_bandwidths|read_bandwidths>
```
```
Specify the metric sets reported by the
memory module of AI Engine tiles on a tile-by-
tile basis. This can be used in conjunction with
graph-by-graph selection and will take priority
on the specified tiles.
Refer to descriptions from
graph_based_aie_memory_metrics
```
```
tile_based_interface_tile_metrics<column|all>:
<off|input_bandwidths|output_bandwidths|packets>
[:<channel>]
;
<mincolumn>:<maxcolumn>:
<off|input_bandwidths|output_bandwidths|packets>
[:<channel>]
```
```
Specify the metric sets reported by the AI
Engine interface tiles on a tile-by-tile basis.
Note: Interface tiles are separate from the
AI Engine tiles and have different metric sets.
```
```
interval_us <int> Controls the interval of reading the AI Engine
counter values in microseconds (μs). The
default interval is 1000 μs.
Note: This switch only has an effect if
aie_profile = true.
```
### AIE_trace_settings Group

The options specified in this group are applied only if aie_trace=true under the [Debug] group.

**Table: AI Engine Trace Options**

```
Key Valid Values Description
```
```
buffer_size <string> (default=8M) Controls the total size of the buffers allocated
for AI Engine event trace. This size is
partitioned evenly into the number of different
trace streams coming out of the AI Engine.
The default is 8M.
```

#### Displayed in the footer

###### ✎

###### !!

###### !!

###### ✎

```
Key Valid Values Description
Note: This switch only has an effect if
aie_trace = true.
```
```
buffer_offload_interval_us integer (default=10ms) Interval, in milliseconds, between reading of
PLIO mode AI Engine trace from device to
Host memory.
```
```
periodic_offload true/false (default=true) Enables continuous offload of PLIO mode AI
Engine trace. Generated AI Engine trace
output files (one per stream) gets appended
with new trace data.
```
```
file_dump_interval_s integer (default=5s) Interval, in seconds, between writing
(appending) of raw AI Engine trace data to
output files.
```
```
graph_based_aie_tile_metrics string("")
<graph name|all>:<kernel name|all>:
<off|functions|functions_partial_stalls|functions_all_stalls>
```
```
Specify the metric sets reported by the AI
Engine module of AI Engine tiles on a graph-
by-graph basis.
Important: Currently, only all is supported
for kernel specification.
```
```
tile_based_aie_tile_metrics string("")
<{<column>,<row>}|all>:
<off|functions|functions_partial_stalls|functions_all_stalls>
[:
<memory_stalls|stream_stalls|cascasde_stalls|lock_stalls>]
{<mincolumn,<minrow>}:{<maxcolumn>,
<maxrow>}:
<off|functions|functions_partial_stalls|functions_all_stalls>
```
```
Specify the metric sets reported by the AI
Engine module of AI Engine tiles on a tile-by-
tile basis.
Important: Currently, only all is supported
for kernel specification.
```
```
reuse_buffer true/false (false)
```
### Emulation Group

The Emulation group of switches apply to the emulation environments and the AMD Vivado™ simulator.

**Table: Emulation Group Keys and Values**

```
Key Valid Values Description
```
```
aliveness_message_interval Any integer Specifies the interval in seconds that
aliveness messages need to be printed. The
default is 300.
```
```
debug_mode [off|batch|gui] Specifies how the waveform is saved and
displayed during emulation.
```
```
off: Do not launch simulator waveform
GUI, and do not save wdb file. This is the
default value.
batch: Do not launch simulator
waveform GUI, but save wdb file
gui: Launch simulator waveform GUI,
and save wdb file
```
```
Note: The kernel needs to be compiled
with debug enabled (v++ -g) for the
```

#### Displayed in the footer

```
Key Valid Values Description
waveform to be saved and displayed in the
simulator GUI.
```
print_infos_in_console [true|false] Controls the printing of emulation info
messages to user's console. Emulation info
messages are always logged into a file called
emulation_debug.log

```
true: Print in user's console. This is the
default value.
false: Do not print in user console.
```
print_warnings_in_console [true|false] Controls the printing emulation warning
messages to user's console. Emulation
warning messages are always logged into a
file called emulation_debug.log.

```
true: Print in user's console. This is the
default value.
false: Do not print in user console.
```
print_errors_in_console [true|false] Controls printing emulation error messages in
user's console. Emulation error messages are
always logged into the emulation_debug.log
file.

```
true: Print in user's console. This is the
default value.
false: Do not print in user's console.
```
user_pre_sim_script Path to Tcl file For the first run, run simulation in GUI mode.
Add signals that you want to add. Copy the
commands from the Tcl console and save into
a Tcl script.
For the next run, pass the Tcl script in batch
mode.

user_post_sim_script Path to Tcl file Any post operations can be specified in the Tcl
and pass to the switch. All the command
provided in the Tcl gets executed after
simulation is completed.

xtlm_aximm_log [true|false] Enables the XTLM AXI4 Memory Map
transaction logging at runtime and you could
see all the transactions in the xsc_report.log
file.

xtlm_axis_log [true|false] Enables the XTLM AXI4-Stream transaction
logging at runtime and you could see all the
transactions in the xsc_report.log file.

timeout_scale na/ms/sec/min Timeout support for clPollStream API in
emulation. Provides a scale for the timeout
specified in clPollStream API. The timeout
specified in the code is specified in ms, and
might not work for emulation. Therefore use
the timeout_scale to map ms to another
scale if needed for emulation.


#### Displayed in the footer

###### !!

**Key Valid Values Description**

```
Important: Timeout is not enabled in
emulation by default. Use this option to enable
clPollStream timeout.
```

