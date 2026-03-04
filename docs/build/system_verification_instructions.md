##### Confidential - Copyright © Fluid Topics

## Embedded Design Development Using Vitis User

## Guide (UG1701)


##### Displayed in the footer

**Application Verification Using Vitis Emulation Flow**
Using Embedded Platforms
Building the System in HW Emulation
Packaging the System in HW Emulation
Running the System on Embedded Processor Platform
Simulator Support in Hardware Emulation
Profile and Debug in Hardware Emulation
Using I/O Traffic Generators
Speed and Accuracy of Hardware Emulation
**Profiling and Tracing the Application**
Profiling the Application
Tracing The Application
**Debugging System Projects**
Hardware Profile and Debug Methodology
Stage 1: Design Execution and System Metrics
Stage 2: System Profiling
Stage 3: PL Kernel Analysis
Stage 4: AI Engine Event Trace and Analysis
Stage 5: Host Application Debug


##### Displayed in the footer

```
✎
```
# Application Verification Using Vitis Emulation

# Flow

Development of an application and hardware kernels targeting an FPGA requires a phased development approach. Because FPGA,
AMD Versal™ adaptive SoC, and AMD Zynq™ UltraScale+™ MPSoC are programmable devices, building the device binary for
hardware takes time. To enable faster iterations without going through the full hardware compilation flow, the AMD Vitis™ tool provides
hardware emulation target to perform C-RTL co-simulation of the software application and PL kernels. Compiling for hardware
emulation target is significantly faster than compiling for the actual hardware. Additionally, hardware emulation target provides full
visibility into the application, making it easier to perform debugging. Once your design passes in hardware emulation, you can compile
and run the application on the hardware platform in the late stages of development.
The Vitis tool provides the following emulation target:

**Hardware emulation (hw_emu)**
The host program runs in the QEMU, but the kernel code is compiled into an RTL behavioral model which is run in the AMD
Vivado™ simulator or other supported third-party simulators. This build and run loop takes longer but provides a cycle-accurate
view of kernel logic.

Compiling and linking for emulation targets is seamlessly integrated into the Vitis command line and IDE flows. You can compile your
host and kernel source code for hardware emulation target, without making any change to the source code. For your host code, you do
not need to compile differently for emulation as the same host executable or PS application ELF binary can be used in emulation.
Hardware emulation target support most of the features including XRT APIs, buffer transfer, platform memory SP tags and kernel-to-
kernel connections. The following sections describes the features and requirements of the hardware emulation flow.
In a typical development flow, verification using HW Emulation is done prior to hardware run. The following are some prerequisites that
can be followed during the application verification:

1. Functional emulation of the system for verifying the functional correctness of the complete system. This includes running C
    simulation on HLS components, or running the x86 simulator for verifying AI Engine components.
2. AI Engine simulator for verifying that the AI Engine kernel and graph meets performance needs of the application.
3. Hardware emulation of the system for verifying timing, and accuracy of the full design prior to Hardware run.
    **Note:** This can also include running C/RTL co-simulation on HLS components, or running the AIE Simulator for verifying AI
    Engine components.
4. Testing and debugging on hardware.

## Using Embedded Platforms

Emulation is supported for embedded platforms. The device is modeled as separate x86 process emulating the hardware. The user
host code and the device model process communicate using RPC calls. For embedded platforms, where the CPU code is running on
the embedded Arm processor, emulation flows use QEMU (Quick Emulator) to mimic the Arm-based PS-subsystem. In QEMU, you can
boot embedded Linux and run Arm binaries on the emulation targets.
For running emulation of an embedded application, you will compile the application for an Arm processor using Arm-GCC and launch
the QEMU emulation environment on an x86 processor to model the execution environment of the Arm processor. This requires the use
of the launch_emulator.py command, or launch_emulator.sh shell scripts generated during the build process. The details of
this flow are explained in Running the System on Embedded Processor Platform.

QEMU

QEMU stands for Quick Emulator. It is a generic and open source machine emulator. AMD provides a customized QEMU model that
mimics the Arm based processing system present on AMD Versal™ adaptive SoC, AMD Zynq™ UltraScale+™ MPSoC, and Zynq 7000
SoC. The QEMU model provides the ability to execute CPU instructions at almost real time without the need for real hardware. For
more information, refer to the QEMU User Documentation.
For hardware emulation, the AMD Vitis™ emulation targets use QEMU. It is co-simulated with an RTL and SystemC-based model for
the rest of the design to provide a complete execution model of the entire platform. You can boot an embedded Linux kernel on the
model and run XRT-based applications. Because the QEMU can execute Arm instructions, you can take the Arm binaries and run them
in emulation flows as-is without the need to recompile. QEMU also allows you to debug your application using GDB and TCF-based
target connections from Xilinx System Debugger (XSDB).


##### Displayed in the footer

###### ★

The Vitis emulation flow also uses QEMU to emulate the MicroBlaze™ processor to model the platform management modules (PLM
and PMU) of the devices. On Versal devices, the PLM firmware is used to load the PDI to program sections of the PS and AI Engine
model.
To ensure that the QEMU configuration matches the platform, there are additional files that must be provided as part of sw directory of
Vitis platforms. Two common files, qemu_args.txt and pmc_args.txt, contain the command line arguments to be used when launching
QEMU. When you create a custom platform, these two files are automatically added to your platform with default contents. You can
review the files and edit as needed to model your custom platform. Refer to an AMD embedded platform for an example.
Because QEMU is a generic model, it uses a Linux device tree style DTB formatted file to enable and configure various hardware
modules. A default QEMU hardware DTB file is shipped with the Vitis tools in the <vitis_installation>/data/emulation/dtbs folder.
However, if your platform requires a different QEMU DTB, you can package it as part of your platform.

```
Tip: The QEMU DTB represents the hardware configuration for QEMU, and is different from the DTB used by the Linux kernel.
```
### Building the System in HW Emulation

To simulate the entire system, including AI Engine graph and PL logic along with XRT-based host application to control the AI Engine
and PL, for a specific board and platform, you must use the Vitis hardware emulation flow. This flow includes the SystemC model of the
AI Engine, transaction-level SystemC models for the NoC, DDR memory, PL Kernels (RTL), and the PS (running on QEMU).
Building the system involves building the device binary for HW Emulation target including AI Engine graph and the PL kernel and
building the XRT based PS application. For details on building the system in HW Emulation, see Building and Running the System.
The Vitis tool provides two distinct end-to-end flows for embedded systems: hardware emulation (hw_emu) and hardware (hw). These
flows are intentionally separate because they target different execution models, generate different artifacts, and serve different
validation goals.

#### What the Linker and Packager Builds

See the following for the tasks hw_emu performs.

```
v++ --link -t hw_emu builds a co-simulation model that integrates the following.
Processing System (PS) running functionally in QEMU.
AI Engine (AIE) modeled in SystemC (cycle-approximate).
Programmable Logic (PL) kernels compiled to RTL and simulated in an RTL simulator (for example, XSIM).
Transaction-level models for NoC, memory, and other data paths where applicable.
The linker still produces an .xclbin that XRT consumes. Packaging (v++ -p -t hw_emu) generates emulation artifacts and
scripts (for example, launch_hw_emu.sh). Packaging also generates emulation-oriented boot components (rootfs, device tree)
required to drive QEMU and the simulators.
```
hw:

```
v++ --link -t hw generates a deployable .xclbin that corresponds to a fully implemented PL bitstream, AIE binaries, and
associated metadata to run on the device.
Packaging produces SD/flash images for the board. The images contain real bitstream/PDI, AIE binaries, and application/software
stack for execution on silicon.
```
#### Execution Model and Fidelity

hw_emu:

```
Runs in a mixed simulation environment. The PS is functionally accurate (QEMU). AIE and interconnect and memory are cycle-
approximate SystemC models. The user PL kernels execute as RTL in the simulator (RTL-accurate within the simulated region).
This option is not intended to be cycle-accurate at the full-system level. Latency, bandwidth, and contention behavior can differ
from hardware.
```
hw:

```
Runs on real hardware with actual clocks, I/O, DDR/LPDDR behavior, NoC routing, board peripherals, and software stack.
Required to validate timing closure, I/O margins, power/thermal behavior, and full-chip interactions.
```

##### Displayed in the footer

#### Debug, visibility, and analysis

hw_emu:

```
Emphasis on visibility and bring-up. These include RTL waveforms, SystemC/TLM traces, AIE graph inspection, and rich XRT
profiling/tracing.
Supports system-level checks such as deadlock detection workflows and controlled traffic generation.
Memory monitoring: supports AXI Interface Monitors (AIMs) on memory ports in counters-only configurations.
```
hw:

```
Debug uses on-chip capabilities (for example, ILA/ChipScope, XVC) and software debuggers. Visibility is more limited than in
emulation.
Used to confirm real-world performance, bandwidth under contention, and platform behavior that only appears on silicon.
```
#### Turnaround and optimization

hw_emu:

```
Faster build–run cycles because it avoids full synthesis/place/route and board programming.
Ideal for functional integration, driver/XRT flow debug, and early performance trend analysis.
hw:
Longer compile and deployment cycles due to full Vivado implementation and device programming.
Essential for QoR assessment (timing/resource), final performance/power, and board-level validation.
```
#### Segmented configuration and dynamic reload

hw_emu:

```
Segmented configuration (multi-PDI overlays and dynamic PL reload) is not supported. Attempts to use segmented configuration
in hw_emu will result in an error.
```
hw:

```
Segmented configuration is supported (platform- and version-dependent), enabling dynamic PL reload and multi-PDI use cases. It
provides an entry point to DFX-like scenarios without a full Vivado DFX flow and supports isolation/subsystem use cases where
PS-to-PL paths remain fixed.
```
#### When to use which flow

Choose hw_emu to:

```
Functionally validate PS–AIE–PL integration with high debug visibility.
Develop and debug host/driver/AIE/PL interactions and XRT flows.
Investigate deadlocks and tune buffering/latencies using approximate system models.
Gather early profiling data and performance trends (not final performance).
```
Choose hw to:

```
Validate timing, resource usage, bandwidth, and performance on the actual device.
Exercise board-level I/O and full software stacks under realistic conditions.
Use segmented configuration/dynamic PL reload where applicable.
Prepare the design for deployment and sign-off.
```
### Packaging the System in HW Emulation

Packaging the entire system after compilation and linking is required for embedded processor platforms. Packaging the system
generates the SD card and necessary flash images that helps boot the system prior to running HW Emulation. The packaging for HW
Emulation requires target to be set v++ -p -t hw_emu. For designs that use base platforms prepared for Embedded Development
Framework (EDF), but does not have EDF as default flow (like VRK160 boards), you need to set --advanced.param


##### Displayed in the footer

**!!**

✎

package.enableEdfHwEmu=1 when packaging. This switch uses QEMU file sets and packaging outputs suitable for EDF. For
VEK385, EDF flow is default and you do not need to set this additional parameter.
For more details on package options, see Packaging for Vitis Flow.

### Running the System on Embedded Processor Platform

Running Emulation target is achieved in the QEMU environment on embedded processors. Hardware emulation target has specific
drivers which are loaded at runtime by XRT. Thus, the same CPU binary can be run as-is without recompiling, by changing the target
mode during runtime. Based on the value of the XCL_EMULATION_MODE environment variable, XRT loads the target specific driver and
makes the application interface with an emulation model of the hardware. The allowed value of XCL_EMULATION_MODEis hw_emu. If
XCL_EMULATION_MODE is not set, then XRT will load the hardware driver.
**Important:** It is required to set XCL_EMULATION_MODE when running emulation.
Use the xrt.ini file to configure various options applicable to emulation. There is an [Emulation] specific section in xrt.ini, as
described in xrt.ini File.
Emulation for a system with the AI Engine can be useful in:

```
Checking initial system behavior with a limited known data set
Functional integration and debugging of PS, PL, and ADF graph using GDB
Testing the system with external traffic generator using Python, or C++
Running system with C-based models for RTL kernels
Applying AI Engine simulation options through the aiesim_options.txt found in the aiesimulator_output directory
```
When launching hardware emulation, specify options for the AI Engine component according to the simulator being used. These
options can be specified from the launch_hw_emu.sh script using the -aie-sim-options.

#### Reusing AI Engine Simulator Options

The AI Engine simulator generates an options file that lists the options used for simulating the AI Engine graph application. The options
file is automatically generated when the AI Engine simulator is run. This helps reuse the AI Engine simulator options from the initial
graph-level simulation later in the system-level hardware emulation. You can also manually edit the options file to specify other options
as required. The following table lists the options that can be specified in the aiesim_options.txt file. This file is located in the
aiesimulator_output directory and is created if the --dump-vcd option is used with the aiesimulator command. An example
command line is as follows.

```
./launch_hw_emu.sh \
-aie-sim-options ${FULL_PATH}/aiesimulator_output/aiesim_options.txt
```
where ${FULL_PATH} must be the full path to the file or directory.

**Table: AI Engine Options for Hardware Emulation**

```
Command Arguments Description
```
```
AIE_DUMP_VCD <filename> When AIE_DUMP_VCD is specified, the simulation generates VCD data and writes it
to the specified <filename>.vcd. For more information about the options used to
generate select signals associated with specific modules, see Generating VCD with
Select Signals in the AI Engine Tools and Flows User Guide (UG1076)
```
```
AIE_DEBUG_AXIMM True | False When AIE_DEBUG_AXIMM is enabled (True), simulation generates memory-
mapped AXI4 transaction data and writes it to a aiesim_debug_axi_mm_dump.txt
file.
```
```
AIE_PKG_DIR /path_to_work_dir/WorkThis is a mandatory option that sets the path to the Work directory generated by the
AI Engine compiler. If you do not specify this option, the generated
sim/behav_waveform/xsim/default.aierun_summary file will not have the correct
Work directory setting, which will impact the display of the summary file in the Vitis
analyzer.
```
```
Note: Any command that has a path to a file must use an absolute path.
```

##### Displayed in the footer

✎

###### ★

###### ★

###### ★

When creating a simulation option file manually it needs to follow the format of COMMAND=ARGUMENT, with each command being on a
separate line. The following example shows best practice.

```
AIE_DUMP_VCD=foo
```
To view the AMD Vivado™ simulator waveform GUI, use the launch_hw_emu script with -g option, and also use the -g option during
the v++ --link stage as well.

```
./launch_hw_emu.sh -g
```
When the emulation environment is fully booted and the Linux prompt is up, make sure to set the following environment variables in the
QEMU environment to ensure that the host application works. These must also be set when running on hardware.

```
export XILINX_XRT=/usr
export LD_LIBRARY_PATH=/mnt/sd*1:
export XCL_EMULATION_MODE=hw_emu
```
Below are end-to-end steps to launch HW Emulation:
**Recommended:** The file size limit on your machine should either be set to unlimited or a higher value (over 16 GB) because
embedded HW Emulation can create files with larger file size for memory.

```
Tip: Set up the command shell or window as described in Setting Up the Vitis Environment prior to running the builds.
```
1. Set the desired runtime settings in the xrt.ini file.
    As described in xrt.ini File , the file specifies various parameters to control debugging, profiling, and message logging in XRT when
    running the host application and kernel execution. As described in Enabling Profiling in Your Application this enables the runtime to
    capture debugging and profile data as your application is running.
    The xrt.ini file, as well as any additional files required for running the application, must be included in the output files as explained
    in Packaging for Vitis Flow

```
Tip: Be sure to use the v++ -g option when compiling your kernel code for emulation mode.
```
2. Launch the QEMU emulation environment by running the launch_hw_emu.sh script.
    The script is created in the emulation directory during the packaging process, and uses the launch_emulator.py command to
    setup and launch QEMU. When launching the emulation script you can also specify options for the launch_emulator.py
    command. Such as the -forward-port option to forward the QEMU port to an open port on the local system. This is needed
    when trying to copy files from QEMU as discussed in Step 5 below. Refer to launch_emulator Utility in the _Vitis Reference Guide_
    (UG1702) for details of the command.
    For AI Engine, specify the AI Engine Options for HW Emulation as described in above paragraph.
    Another example would be to specify launch_hw_emu.sh -enable-debug to configure additional XTERMs to be opened for
    QEMU and PL processes to observe live transcripts of command execution to aid in debugging the application. This is not enabled
    by default, but can be useful when needed for debug.
    Additionally, you can add more advanced options to log waveform data without having to launch emulation with the Vivado logic
    simulator GUI. An example command line is as follows.

```
./launch_hw_emu.sh \
-user-pre-sim-script pre-sim.tcl
```
```
The pre-sim.tcl contains Tcl commands to add waveforms or log design waveforms. For an example, for Tcl commands see Vivado
Design Suite User Guide: Logic Simulation (UG900).
```
```
Tip: Details on debugging techniques in Hardware Emulation can be found in Profile and Debug in Hardware Emulation.
```
3. Once the emulation environment is fully booted and the Linux prompt is up, mount and configure the QEMU shell with the required
    settings.
    The AMD embedded base platforms have rootfs on a separate EXT4 partition on the SD card. After booting Linux, this partition
    needs to be mounted. If you are running emulation manually, you need to run the following commands from the QEMU shell:

```
mount /dev/mmcblk0p1 /mnt
cd /mnt
export LD_LIBRARY_PATH=/mnt:/tmp:$LD_LIBRARY_PATH
export XCL_EMULATION_MODE=hw_emu
```

##### Displayed in the footer

###### ★

###### ★

```
✎
```
```
✎
```
✎

```
export XILINX_XRT=/usr
export XILINX_VITIS=/mnt
```
```
Tip: You can set the XCL_EMULATION_MODE environment variable to hw_emu for hardware emulation. This configures the
host application to run in emulation mode.
```
4. Run the application from within the QEMU shell.
    With the runtime initialization (xrt.ini), the XCL_EMULATION_MODE environment set, run the host executable with the command line
    as required by the host application. For example:

```
./host.elf kernel.xclbin
```
```
Tip: This command line assumes that the host program is written to take the name of the xclbin file as an argument, as most
AMD Vitis™ examples and tutorials do. However, your application can have the name of the xclbin file hard-coded into the host
program, or can require a different approach to running the application.
```
5. After the application run has completed is generated, the xrt.run_summary can be found in the /mnt folder inside the QEMU
    environment. However, to view the file you must copy them from the QEMU Linux system back to your local system.
       **Note:** The files can be copied either from the Guest or Host machine using the scp command. Key terminology to notice here
    are _Host_ and _Guest_. The _Host_ machine is the machine hosting QEMU, and the _Guest_ machine is the one with PetaLinux running
    on QEMU. Host-ip-address is the IP address of the machine from which QEMU is launched.
       a. Copying files from the Host machine:
          i. First, copy the required files from the root area to the petalinux (default) user home area using the following
             command from the Guest machine: cp -rf /mnt/<files> /home/petalinux/
ii. Switch from the default login user petalinux using the exit command
iii. Change the permission of the copied files using sudo chmod 755 <files>
The files can be copied using the scp command from the Host machine as follows: scp -P <port-num> <guest-
machine-user-name>@<host-ip-address>:<source-file> <dest-path>. For example: scp -P 1440
root@192.168.1.xxx:/mnt/xrt.run_summary.
<port-num>: To copy Guest machine files to the Host, the Port number through which Host and Guest are connected is
required. 1440 is the QEMU port to connect to the Guest port. The -forward-port 1440 22 is passed to the
launch_emulator to map the Guest port to the Host port. Here, 22 is the Guest TCP port and 1440 is the Host port. Both
ports are mapped. To access the Guest TCP services (on port 22), use the Host machine's mapped port (for example 1440).
Accessing the Host's port 1440 means accessing Guest port 22.
<guest-machine-user-name>The guest machine user name can be found by using the whoami command from the
PetaLinux terminal.
<host-ip-address>: The host IP address can be found with a Linux command such as nslookup <machine name> or
hostname-i
<source-file>: The path and name of the file you want to copy from the QEMU environment.
<dest-path>: The destination path to copy the file to on the local system.
       b. Copying files from the Guest machine:
          You can copy from the guest machine using the scp command: scp <guest-file> <userid-of-
          hostmachine>@<host-ip-address>:<host's-dir-path> For example: scp /mnt/xrt.run_summary
          userabcd@192.168.1.xxx:~
6. When your application has completed emulation and you have copied the required files, press the Ctrl + a + x keys to terminate
    the QEMU shell and return to the Linux shell.
       **Note:** If you have trouble terminating the QEMU environment, you can kill the processes it launches to run the environment.
    The tool reports the process IDs (pids) at the start of the transcript, or you can specify the -pid-file option to capture the pids
    when launching emulation.

#### Viewing the Run Summary

After hardware emulation is launched and run with the above simulation options, the run summary is available as follows:
For AI Engine run summary generated inside <package_dir>/sim/behav_waveform/<simulator>/default.aierun_summary, the VCD file is
generated at the same location, for example: <package_dir>/sim/behav_waveform/<simulator>/<vcd filename>.
**Note:** Viewing summary from the third-party simulator is also available.

AI Engine Throughput Estimates

Hardware Emulation reports the average throughput of each PLIO and GMIO port at the end of a simulation.


##### Displayed in the footer

As described in the previous section, emulation is launched using the following command:
./launch_hw_emu.sh -aie-sim-options ${FULL_PATH_TO}/<user_aiesim_options.txt>
Here, <user_aiesim_options.txt> must include the AIE_PKG_DIR option that sets the path to the Work directory generated by
the AI Engine compiler.
The throughput report is generated inside simulate.log found at
<design_directory>/sim/behav_waveform/xsim/simulate.log.

```
Info: /OSCI/SystemC: Simulation stopped by user.
```
```
Info: SystemC end_of_simulation Dump:
103765200 ps
.vitis_design_wrapper_sim_wrapper.vitis_design_wrapper_i.vitis_design_i.ai_engine_0.inst.aie_logical.a
ie_xtlm.math_engine: end of simulation invoked!
------------------------------------------------------------
| Intf Type | Port Name | Type | Throughput(MBps) |
------------------------------------------------------------
| gmio | gmioIn0 | IN | 3404.26 |
| | gmioOut0 | OUT | 3453.82 |
| | gmioIn1 | IN | 3890.58 |
| | gmioOut1 | OUT | 3957.06 |
| | gmioIn2 | IN | 3968.99 |
| | gmioOut2 | OUT | 3903.18 |
| | gmioIn3 | IN | 3244.61 |
| | gmioOut3 | OUT | 3278.27 |
------------------------------------------------------------
```
### Simulator Support in Hardware Emulation

The AMD Vitis™ tool uses the AMD Vivado™ logic simulator (xsim) as the default simulator for all platforms, AMD Versal™ and AMD
Zynq™ UltraScale+™ MPSoC embedded platforms. However, for Versal embedded platforms, like xilinx_vck190_base or custom
platforms similar to it, the Vitis tool also supports the use of third-party simulators for hardware emulation: Mentor Graphics Questa
Advanced Simulator, Xcelium, and VCS. The specific versions of the supported simulators are the same as the versions supported by
Vivado Design Suite.

#### Enabling Third-Party Simulators

Third-party simulators are supported in the AMD Vitis™ Hardware Emulation flow. There are specific settings around third-party
simulators that need to be provided in the AMD Vitis™ configuration file. The AMD Vitis™ Unified IDE also supports hardware emulation
flow using third-party simulators. Third-party simulators such as Questa Advanced Simulator (Mentor Graphics), Xcelium (Cadence),
VCS (Synopsys), and Riviera Simulator (Aldec) are supported when executing hardware emulation of your design. For more details,
see the _Vivado Design Suite User Guide: Logic Simulation_ (UG900) for third-party simulator setup.
You can enable these simulators by updating the AMD Vitis™ configuration file (config.ini or system.cfg). When the settings are added
to the AMD Vitis™ configuration file, build the design using the v++ link and package flow as described in the script launch_hw_emu.sh.
This will launch hardware emulation using the third party simulator specified.
The following are the configuration options to enable third-party simulator setup during v++ -link command line flow. To see enabling
third party simulators in the AMD Vitis™ Unified IDE, refer to Enabling Third Party Simulators in the _Vitis Reference Guide_ (UG1702).

**Table: Vitis Link Settings**

```
Simulator Vitis configuration file settings (config.ini or system.cfg)
```
```
Questa
Advanced
Simulator
```
```
[advanced]
param=hw_emu.simulator=QUESTA
[vivado]
prop=project.__CURRENT__.simulator.questa_install_dir=<SIMULATOR
DIRECTORY>/questa/2025.2/bin
prop=project.__CURRENT__.compxlib.questa_compiled_library_dir=<SIMULATOR LIBRARY
DIRECTORY>/questa/2025.2/lin64/lib/
prop=fileset.sim_1.questa.compile.sccom.cores={16}
```

##### Displayed in the footer

```
Simulator Vitis configuration file settings (config.ini or system.cfg)
prop=fileset.sim_1.questa.elaborate.vopt.more_options={-stats=all}
prop=fileset.sim_1.questa.simulate.vsim.more_options={-stats=all}
```
```
Xcelium
[advanced]
param=hw_emu.simulator=XCELIUM
[vivado]
prop=project.__CURRENT__.simulator.xcelium_install_dir=<SIMULATOR DIRECTORY>/bin/
prop=project.__CURRENT__.compxlib.xcelium_compiled_library_dir=<SIMULATOR LIBRARY
DIRECTORY>/xcelium/25.03.002/lin64/lib/
prop=fileset.sim_1.xcelium.elaborate.xmelab.more_options={-timescale 1ns/1ps -
STATUS]
```
```
VCS
[advanced]
param=hw_emu.simulator=VCS
[vivado]
prop=project.__CURRENT__.simulator.vcs_install_dir=<SIMULATOR DIRECTORY>/vcs/X-
2025.06/bin/
prop=project.__CURRENT__.compxlib.vcs_compiled_library_dir=<SIMULATOR LIBRARY
DIRECTORY>/vcs/X-2025.06/lin64/lib/
prop=project.__CURRENT__.simulator.vcs_gcc_install_dir=<SIMULATOR
DIRECTORY>/synopsys/vg_gnu/X-2025.06/linux64/gcc-9.2.0_64/bin
param=project.alignLibraryPathEnvForVCS=true
prop=fileset.sim_1.vcs.compile.vlogan.more_options={-v2005}
```
```
Riviera
[advanced]
param=hw_emu.simulator=RIVIERA
[vivado]
prop=project.__CURRENT__.simulator.riviera_install_dir=<SIMULATOR
DIRECTORY>/riviera/2024.10-lin64/bin/
prop=project.__CURRENT__.compxlib.riviera_compiled_library_dir=<SIMULATOR LIBRARY
DIRECTORY>/riviera/2024.10/lin64/lib/
prop=project.__CURRENT__.simulator.riviera_gcc_install_dir=<SIMULATOR
DIRECTORY>/riviera/2024.10-lin64/gcc_Linux64/bin/
prop=fileset.sim_1.riviera.simulate.asim.more_options={+access +r}
```
After generating the configuration file you can use it in the v++ command line, for example:

```
v++ -link --config system.cfg
```
You can use the -user-pre-sim-script and -user-post-sim-script options from the launch_emulator.py command to
specify Tcl scripts to run before the start of simulation, or after simulation completes. As an example, in these scripts, you can use the
$cwd command to get the run directory of the simulator and copy any files needed prior to simulation, or copy any output files
generated at the end of simulation.
To enable hardware emulation, you must set up the environment for simulation in the Vivado Design Suite. A key step for setup is pre-
compiling the RTL and SystemC models for use with the simulator. To do this, you must run the compile_sim_lib command in the
Vivado tool. For more information on pre-compilation of simulation models, refer to the _Vivado Design Suite User Guide: Logic
Simulation_ (UG900).
When creating your Versal platform ready for simulation, the Vivado tool generates a simulation wrapper which must be instantiated in
your simulation test bench. So, if the top most design module is <top>, then when calling launch_simulation in the Vivado tool, it
will generate a <top>_sim_wrapper module, and also generates xlnoc.bd. These files are generated as simulation-only sources and
will be overwritten whenever launch_simulation is called in the Vivado tool. Platform developers need to instantiate this module in
the test bench and not their own <top> module.


##### Displayed in the footer

✎

###### ★

Using the Simulator Waveform Viewer

Hardware emulation uses RTL and SystemC models for execution. A regular application and HLS-based kernel developer does not
need to be aware of the hardware level details. The Vitis analyzer provides sufficient details of the hardware execution model. For
advanced users who are familiar with HW signal and protocols, you can launch hardware emulation with the simulator waveform
running, as described in Waveform View and Live Waveform Viewer.
By default, when running v++ --link -t hw_emu, the tool compiles the simulation models in optimized mode. However, when you
also specify the -g switch, you enable hardware emulation models to be compiled in debug mode. During the application runtime, use
the -g switch with the launch_hw_emu.sh command to run the simulator interactively in GUI mode with waveforms displayed. By
default, the hardware emulation flow adds common signals of interest to the waveform window. However, you can pause the simulator
to add signals of interest and resume simulation.

Waveform View and Live Waveform Viewer

The Vitis core development kit can generate a Waveform view when running hardware emulation. It displays in-depth details at the
system-level, CU level, and at the function level. The details include data transfers between the kernel and global memory and data
flow through inter-kernel pipes. These details provide many insights into performance bottlenecks from the system-level down to
individual function calls to help optimize your application.
The Live Waveform Viewer is similar to the Waveform view, however, it provides even lower-level details with some degree of
interactivity. The Live Waveform Viewer can also be opened using the Vivado logic simulator, xsim.
**Note:** The Waveform view allows you to examine the device transactions from within the Vitis analyzer (see Working with the
Analysis View (Vitis Analyzer) in the _Vitis Reference Guide_ (UG1702)). In contrast, the Live Waveform Viewer opens the Vivado
simulation waveform viewer to examine the hardware transactions in addition to any user selected signals.
Waveform data is not collected by default because it requires the runtime to generate simulation waveforms during hardware emulation,
which consumes more time and disk space. Refer to Generating and Opening the Waveform Reports for instructions on enabling these
features.

**Figure: Waveform View**

You can also open the waveform database (.wdb) file with the Vivado logic simulator through the Linux command line:

```
xsim -gui <filename.wdb> &
```
```
Tip: The .wdb file is written to the directory where the compiled host code is executed.
```
Generating and Opening the Waveform Reports

Follow these instructions to enable waveform data collection from the command line during hardware emulation and open the viewer.

1. Enable debug code generation during compilation and linking using the -g option.

```
v++ -c -g -t hw_emu ...
```
2. Create an xrt.ini file in the same directory as the host executable with the following contents (see xrt.ini File for more information).


##### Displayed in the footer

###### ★

```
[Emulation]
debug_mode=batch
```
```
The debug_mode=batch enables the capture of waveform data (.wdb) by running simulation in batch mode. You can also enable
the Live Waveform Viewer to launch simulation in interactive mode using the following setting in the xrt.ini.
```
```
[Emulation]
debug_mode=gui
```
```
Tip: If Live Waveform Viewer is enabled, the simulation waveform opens during the hardware emulation run.
```
3. Run the hardware emulation build of the application as described in Application Verification Using Vitis Emulation Flow. The
    hardware transaction data is collected in the waveform database file, <hardware_platform>-<device_id>-<xclbin_name>.wdb.
    Refer to Output Directories of the v++ Command in the _Vitis Reference Guide_ (UG1702) for more information on locating these
    reports.
4. Open the Waveform view in the Vitis analyzer by opening the Run Summary, and opening the Waveform report.

```
vitis_analyzer xrt.run_summary
```
5. Waveforms for TLM transactions can also be dumped for third-party simulators (support limited to Mentor Graphics Questa
    Advanced Simulator and Cadence Xcelium). Wave data dump is enabled when v++ link is done with -g option (as mentioned in
    step 1). The format of the wave database dumped is simulator specific (for example, .wlf for Questa Advanced Simulator and .shm
    for Xcelium).

Interpreting Data in the Waveform Views

The following image shows the Waveform view:

**Figure: Waveform View**

The Waveform and Live Waveform views are organized hierarchically for easy navigation.


##### Displayed in the footer

```
The Waveform view is based on the actual waveforms generated during hardware emulation (Kernel Trace). This allows the
viewer to descend all the way down to the individual signals responsible for the abstracted data. However, because the Waveform
view is generated from the post-processed data, no additional signals can be added to the report, and some of the runtime
analysis cannot be visualized, such as DATAFLOW transactions.
The Live Waveform viewer is displaying the Vivado logic simulator (xsim) run, so you can add extra signals and internals of the
register transfer (RTL) design to the live view. Refer to the Vivado Design Suite User Guide: Logic Simulation (UG900) for
information on working with the Waveform viewer.
```
The hierarchy of the Waveform and Live Waveform views include the following:

**HLS Process Summary**
Hierarchical view of processes and their sub-processes corresponding to the user functions in CU
Entry for each kernel instance which has processes modeling user function in it (entries can be unfolded to show the
processes in it)
Handshake transactions on all the processes corresponding to user-functions
Both dataflow and non-dataflow/non-piplelined processes
The transactions on the processes including stalls, are shown using the corresponding protocol analyzer instance
Allows you to get an overview about the usage of the individual processes over the execution time (similar to C/C++ profile
capabilities)


##### Displayed in the footer

**Device "name"**
Target device name.
**Binary Container "name"**
Binary container name.
**Memory Data Transfers**
For each DDR Bank, this shows the trace of all the read and write request transactions arriving at the bank from the
host.
**Kernel "name" 1:1:**
For each kernel and for each compute unit of that kernel, this section breaks down the activities originating from the
compute unit.
**Compute Unit: "name"**
Compute unit name.
**CU Stalls (%)**
Stall signals are provided by Vitis HLS to inform you when a portion of the circuit is stalling because of external
memory accesses, or internal streams (that is, dataflow). The stall bus shown in detailed kernel trace compiles all
of the lowest level stall signals and reports the percentage that are stalling at any point in time. This provides a
factor of how much of the kernel is stalling at any point in the simulation.
For example, if there are 100 lowest level stall signals and 10 are active on a given clock cycle, then the CU Stall
percentage is 10%. If one goes inactive, then it is 9%.
**Data Transfers**
This shows the read/write data transfer accesses originating from each Master AXI port of the compute unit to the
DDR.
**User Functions**
This information is available for the HLS kernels and shows the user functions.
**Function: "name"**
Function name.
**Dataflow/Pipeline Activity**
This shows the number of parallel executions of the function if the function is implemented as a dataflow
process.
**Active Iterations**
This shows the currently active iterations of the dataflow. The number of rows is dynamically
incremented to accommodate the visualization of any concurrent execution.
**StallNoContinue**
This is a stall signal that tells if there were any output stalls experienced by the dataflow processes
(function is done, but it has not received a continue from the adjacent dataflow process).
**RTL Signals**
These are the underlying RTL control signals that were used to interpret the above transaction
view of the dataflow process.
**Function Stalls**
Shows the different types of stalls experienced by the process.
**External Memory**
Stalls experienced while accessing the DDR memory.
**Internal-Kernel Pipe**
If the compute units communicated between each other through pipes, then this shows the related
stalls.
**Intra-Kernel Dataflow**
FIFO activity internal to the kernel.
**Function I/O**
Actual interface signals.


##### Displayed in the footer

✎

**HLS FIFO**
This shows waveform for size of HLS FIFOs created inside non-RTL kernels
The waveform is in Analog style
It shows one entry for each kernel instance which has FIFO in it
The analog waveform is produced by tracing on an internal HDL signal of the kernel which gives the current number of
elements in the FIFO during simulation.
**CU name**
Name of the CU containing FIFO
**FIFO instance name**
Name of the FIFO instanc
**mOutPtr["size:"0]**
HDL signal which gives the number of elements currently in the FIFO during simulation

Interpreting TLM Waveform Data for Third-Party Simulators

1. Under the respective design hierarchy in the waveform windows, for each TLM—TLM socket connection, the following information
    is visible as waveforms.
       a. For memory mapped AXI4 interfaces, the bus transaction is visible as six channels:
          <socket_name>: This channel contains statistics such as whether transaction is read/write and a unique mark to
          differentiate information in sub channels. This also indicates the number of transactions completed.
          <socket_name>_AW: This channel contains the transaction information of Write Address.
          <socket_name>_W: This channel contains the transaction information of Write Data.
          <socket_name>_B: This channel contains the transaction information of the corresponding Write Response.
          <socket_name>_AR: This channel contains the transaction information of Read Address.
          <socket_name>_R: This channel contains the transaction information of Read Data.
Detailed attributes for each channel like burst size, burst type, response, etc. are visible as attributes in each channel.
       b. For AXI4-Stream, the bus transaction is visible in one channel only named after the socket_name. This contains the
          information like TID, TDEST, TDATA, etc. as attributes.

**Note:** TLM waveform viewing is only supported for Questa Advanced Simulator and Xcelium. The information on the usage of
waveform, adding socket to waveform view, and detailed view of attributes can be referred to its respective third-party simulator user
guide.

AXI Transactions Display in XSIM Waveform

Many models in hardware emulation use SystemC transaction-level modeling (TLM). In these cases, interactions between the models
cannot be viewed as RTL waveforms. However, Vivado simulator (xsim) provides a transaction level viewer. For standard platforms,
these interface objects can be added to the waveform view, similar to how RTL signals are added. As an example, to add an AXI
interface to the waveform, use the following Tcl command in xsim:

```
add_wave <HDL_objects>
```
Using the add_wave command, you can specify full or relative paths to HDL objects. For additional details on how to interpret the TLM
waveform see Interpreting TLM Waveform Data for Third-Party Simulators, or refer to _Vivado Design Suite User Guide: Logic
Simulation_ (UG900).

Generating Test Vectors for Vitis HLS during Hardware Emulation

It is possible to instruct the Vitis tool to generate test vectors for simulation during hardware emulation, without re-running v++
compilation and linking. The test vectors will enable Vitis HLS to run C/RTL Co-simulation without a dedicated C++ test bench for:

```
Deadlock analysis
FIFO depth optimization
Other performance optimizations
```
Use the following steps:

1. Create an hlsPre.tcl file and insert this command:

```
config_export -cosim_trace_generation
```

##### Displayed in the footer

2. Run v++ --compile and keep the HLS project directories, under the <compile_dir>
3. Run v++ --link --target hw_emu
4. Run the application for hardware emulation
5. Locate the hls_cosim inside the HW_EMU run directory
    a. This directory contains one directory for each kernel, with one directory for each kernel instance below it:

```
<build_dir>/.run/<run_number>/hw_em/device0/binary_0/behav_waveform/xsim/hls_cosim/<kernel_nam
e>
```
6. Copy the appropriate kernel directory to the HLS project directory, i.e.:

```
cp -r <build_dir>/.run/<run_number>/.../xsim/hls_cosim/<kernel_name>
<compile_dir>/<kernel_name>/<kernel_name>
```
7. Open the Vitis HLS tool and run C/RTL Co-simulation in batch mode or GUI mode:

```
cosim_design -hwemu_trace_dir <kernel_name>/<instance_name> ...
```
The traces generated from HW_EMU are valid only as long as:

```
The functionality of the kernel does not change
The top interface of the kernel does not change
The number of top interface reads and writes (s_axilite registers, m_axi interfaces, axis interfaces) does not change.
```
### Profile and Debug in Hardware Emulation

While running emulation, you can specify a number of profile options as described in Enabling Profiling in Your Application to capture
design data during runtime. Any reports generated during the run are collected into the xrt.run_summary file. This collection of
reports can be viewed by opening the run_summary in Vitis analyzer, and includes a Summary report, System and Platform Diagrams
to illustrate the hardware design, Run Guidance offering any suggestions for improving the performance of the system, and a Profile
Summary and Timeline Trace when enabled in the xrt.ini file during runtime. Refer to Working with the Analysis View (Vitis Analyzer) in
the _Vitis Reference Guide_ (UG1702) for additional information.
The following table shows some specific techniques for debugging different scenarios in hardware emulation.

**Table: Techniques for Debugging in Hardware Emulation**

```
Debug Focus Description Steps
```
```
x86 host (XRT) Enable detailed XRT logs by updating
xrt.ini.
```
```
Add the following to xrt.ini file. Run the
test case with the updated xrt.ini. Review
the generated xrt_hal.log.
```
```
[runtime]hal_log=xrt_hal.log
```
```
AXI traffic on SystemC models like AI
Engine, NOC, CIPS
```
```
The host transactions are routed through
sim_qdma SystemC module. These
transactions can be dumped into log file.
If PL is also SystemC, then even PL
transactions can be viewed there.
If PL is RTL, then boundary of PL can be
viewed in waveform.
```
```
In xrt.ini add,
```
```
[Emulation]xtlm_aximm_log=tr
ue xtlm_axis_log=true
```
```
View file xsc_report.log.
```
```
Viewing DDR memory content The DDR model saves its contents in a
binary file. In the folder where simulation
is run
(package.hw_emu/sim/behav_waveform/xsim
dir), look for files named like below. Each
such binary file corresponds to a region of
DDR memory at a particular offset.
```
```
The contents of the memory can be seen
by using hexdump command. an example
command is shown below.
```
```
hexdump qemu-memory-
_mem_0x60000000000@0x
0000ULL -s 0x80000000 -n
```

##### Displayed in the footer

```
Debug Focus Description Steps
qemu-memory-_ddr@0x00000000 or
qemu-memory-
_mem_0xc080000000@0xc080000000ULL
These files represent DDR/LPDDR
memory contents in binary form.
There is a backdoor connection between
QEMU to NOC_DDR model. Thus, users
will not see any transactions in the
waveform nor will they see any logs. The
shared memory is directly updated. To
view memory contents, users can directly
dump the memory contents.
```
```
4096 -v -e '1/4 "%02X" "\n"'
> dump_600.log
```
PS (QEMU) During firmware and software execution,
the Arm APU can run into issues. You see
details of various transactions and state
on PS using these mechanisms.

1. Setenv variable
    ENABLE_RP_LOGS=true in the
    shell where QEMU is being run.
    Look for a log file named
    sim/behav_waveform/xsim/qemu_rp.log.
    This contains transactions from PS
    to rest of the peripherals (other than
    DDR).
2. Review
    package.hw_emu/qemu_output.log
    to see the output. This contains PS
    output to STDOUT (UART). If there
    is any error during PLM or other SW
    execution, those will be captured
    here. Look at the details of the error
    (CDO address, U-Boot stage etc) to
    narrow down which CDO is causing
    the error.
3. Run launch_hw_emu.sh -
    enable_debug mode which will
    direct QEMU logs in its own xterm
    window.

AI Engine PDI is downloaded from PS to AI Engine
through fast mode, thus transactions
cannot be seen in the waveform. Users
can enable logging all transactions at AI
Engine AXI interfaces. There is a
separate file per AXI interface

1. Enable setenv
    ENABLE_AIE_DBG_TRACE. View
    transaction log in the simulation
    folder at aie_log/S00_AXI.txt file.
2. Enable AI Engine VCD Dump and
    view contents in Vitis Analyzer by
    adding switch -aie-sim-options
    to launch_hw_emu.sh cmd line.
    See UG1076 for details of AI
    Engine-sim-options file. These are
    common between aiesim and
    hw_emu.
3. Create a file (aie_sim_config.txt) to
    enable AI Engine simulation options.
       a. In this file, add
          "AIE_DEBUG_AXIMM=True"
       b. Pass this file on launch
          emulator cmd line:


##### Displayed in the footer

**!!**

```
Debug Focus Description Steps
```
```
./package.hw_emu/lau
nch_hw_emu.sh -aie-
sim-options
<Absolute path to
the options .txt
file>
```
```
Dumping Waveform in DC flow (Batch
Mode)
```
```
Make sure the design is linked with the -
g option and you have run the design at
least once in Xsim GUI mode to save the
required .wcfg file and save the list of
relevant signals.
Dump the waveform and open it in xsim
along with saved .wcfg file
```
```
Update following options in xrt.ini option
```
```
[Emulation]
user_pre_sim_script=<use pre
sim script absolute path>
```
```
Pre-simulation script is needed to enable
signal dumping:
```
```
log_wave -r *
```
### Using I/O Traffic Generators

Some user applications such as video streaming and Ethernet-based applications make use of I/O ports on the platform to stream data
into and out of the platform. For these applications, performing software and hardware emulation of the design, or running AI Engine
simulation, requires a mechanism to mimic the hardware behavior of the I/O port, and to simulate data traffic running through the ports.
I/O traffic generators let you model traffic through the I/O ports during software and hardware emulation in the AMD Vitis™ application
development flow, during the AI Engine simulation flows (x86sim, AI Enginesim), or during logic simulation in the AMD Vivado™ Design
Suite.
**Important:** Hardware emulation supports both AXI4-Stream and AXI4 memory map interface I/O emulation.
Traffic generators can be written in Python, MATLAB, C/C++, or RTL (Verilog/SV) modules. They are launched in an external process
that communicates with Vitis Emulation process or AI Engine simulation process using Inter Process Communication (IPC). The IPC
connections are established using IPC AXI4-Stream master/slave modules as described in Running Traffic Generators in Python/C++.
Traffic generators are designed to pass data into your system, or receive data from your system. The APIs are provided to handle data
transfer for standard datatypes, or for more complex datatypes. The API you use to create the traffic generator is determined by the
datatype your system requires. For instance, standard datatypes are covered by simple API such as send_data, or receive_data as
described in Python API for AI Engine Graphs or Python API for PL Kernels. More complex datatypes require API like those described in
General Purpose Python API.
The following are additional details on ways to integrate traffic generators in Python/C/C++/Verilog with subsequent PL kernels or the AI
Engine kernels based on your application.

Adding Traffic Generators to Your Design

AXI Traffic Generator kernels provide a method to inject traffic onto the I/O of your system design, AI Engine graph, or PL kernels during
simulation. AMD provides a library that enables interfacing AXI4-Stream to mimic a streaming data flow for software and hardware
emulation, and AXI4 memory mapped interface to mimic memory mapped data transfers for hardware emulation.
The AXI Traffic generators are provided as XO files which can be linked into your System project using the Vitis compiler (v++). These
XO files are called sim_ipc_axis_master_XY.xo and sim_ipc_axis_slave_ZW.xo where XY and ZW correspond to the number of bits in
the PLIO interface. For example, sim_ipc_axis_master_128.xo provides an AXI4-Stream master data bus that is 128 bits wide. A wider
interface allows the PL to achieve the same throughput at a lower clock frequency and allows the AI Engine array to maximize its
memory bandwidth. However, the PLIO interface tiles are each 64 bits wide and they are a limited resource. Using one 64-bit PLIO
interface at twice the clock speed provides an equivalent bandwidth to a 128-bit PLIO while using only one PLIO tile. This requires the
PL to run at twice the clock speed and the optimal choice will vary from application to application.
Two steps are required to use the traffic generators in your system design:

1. Specify the connections between the traffic generator (sim_ipc) modules and their corresponding AXI4-Stream ports on the AI
    Engine array. This is typically done in the system.cfg file using the --connectivity.nk and --connectivity.sc commands,
    as described in Linking the System. The following is an example:


##### Displayed in the footer

```
!!
```
###### ★

```
[connectivity]
nk=sim_ipc_axis_master:1:inst_sim_ipc_axis_master
nk=sim_ipc_axis_slave:1:inst_sim_ipc_axis_slave
stream_connect=sim_ipc_axis_master.M00_AXIS:ai_engine_0.DataIn
stream_connect=ai_engine_0.DataOut:sim_ipc_axis_slave.S00_AXIS
```
```
The syntax for connecting the sim_ipc_axis XO files is as follows.
```
```
nk=sim_ipc_axis_master:<Number Of Masters>:<inst_name_1>.<inst_name_2>.<...>
nk=sim_ipc_axis_slave:<Number Of Slaves>:<inst_name_1>.<inst_name_2>.<...>
```
```
Where:
The sim_ipc_axis_master/slave specifies the XO kernel in your design
The <Number Of Masters> or <Number Of Slaves> field lets you specify up to 8 different traffic generator kernels in
your design
The <inst_name> should be meaningful in your application
```
2. Next, add the XO files to the Vitis link command as shown below.
    **Important:** The traffic generator XO can only be used in hardware emulation with hw_emu target.

```
v++ -l --platform <platform.xpfm> sim_ipc_axis_master_128.xo sim_ipc_axis_slave_128.xo libadf.a -
target hw_emu --config system.cfg
```
For additional information on how to use XO files with the Vitis compiler, see Building the Device Binary in the _Data Center Acceleration
using Vitis_ (UG1700).

Using Traffic Generators for AI Engine Designs

#### Overview

This section describes how to provide input and capture the output from the AI Engine array in all simulation and emulation modes
using AXI traffic generators. In the AI Engine simulator, the input data stimulus is provided using the PLIO object which specifies a text
file containing the data:

```
input_plio plin = input_plio::create("DataIn", adf::plio_32_bits, "data/input.txt");
```
Although this is a fast process to get your first simulation in place, the main limitation of this approach is that if you want to change the
input file name for another simulation, you need to recompile the entire application. To avoid file name specification and rely on the
independent External Traffic Generator to generate data traffic on the PLIO, see below:

```
input_plio plin = input_plio::create("DataIn", adf::plio_32_bits);
```
For hardware emulation, an equivalent feature exists that emulates the behavior of this PLIO and AXI4-Stream interface. Both Python
and C++ APIs are provided to create these External Traffic Generators that will be connected seamlessly on any of these simulation or
emulation modes.
The primary external data interfaces for the AI Engine array are AXI4-Stream interfaces. These are known as PLIOs and allow the AI
Engine to receive data, operate on the data, and send data back on a separate AXI4-Stream interface. The input interface to the AI
Engine is an AXI4-Stream consumer, and the output is an AXI4-Stream producer. To interact with these top level interfaces during
hardware emulation complementary AXI4-Stream modules are provided. These complementary modules are referred to as the AXI
traffic generators.

**Tip:** The width of a PLIO interface is an important system level design decision. The wider the interface the more data can be sent
per PL clock cycle.
When developing an AI Engine application, you can test it standalone either in simulation (x86simulator, aiesimulator), or as part
of a system project in emulation ( hw_emu). In either case, you need to send the input data from a predefined reference file and capture
the output data in a separate file. Furthermore, if your AI Engine graph is intertwined with kernels that are located in the Programmable
Logic (HLS C++ or RTL) then you also have to deal with these data flow interruptions. For example, a full system design might look like
the following figure:

**Figure: AI Engine + Programmable Logic Application**


##### Displayed in the footer

In a first step you replace all the connections which are not in the AI Engine array with text files to provide input data or capture output
data:

**Figure: Initial Simulation Framework**

For more flexibility in data generation and verification you can exchange the text files with external traffic generators which enable
dynamic simulated communication between the PL and the AI Engine array through AXI4-Stream TLM connected to Unix sockets. The
power of these external traffic generator is that they can be used in all simulation/emulation framework without modification:

```
x86 Simulation
AI Engine simulation
HW Emulation
```
The overall simulation framework is illustrated in the following figure:

**Figure: External Traffic Generator-Based AI Engine Simulation Flow**


##### Displayed in the footer

Each AI Engine block can be validated using an external test bench written in Python, MATLAB® , or C++.
For system projects, incorporating AI Engine graph applications and PL kernels, the data movers are replaced with external Traffic
Generators source and sink, and the PL processing kernel is a streaming kernel connected to the AI Engine kernels. See the following
figure for details.

**Figure: Full System Emulation**

The traffic generators are used to feed and flush the data into the full system with PL logic as well. You do not need to model the PS
code writing data to DDR memory and model the data moving to the AI Engine kernels. This apporach replaces the data movers with
external traffic generators dynamically producing data. The following sections describe the step-by-step changes that are needed to
interface external traffic generators with an AI Engine system design for the emulation flow.

#### AI Engine Graph Modifications

Nothing has to be changed within the graph concerning the kernel connections. The definition of the traffic generators as the source of
data from the PLIO port is the only change required as shown below. The example below is based on the code in _Design Flow Using
RTL Programmable Logic_ in _AI Engine Kernel and Graph Programming Guide_ (UG1079).

```
plin = input_plio::create("DataIn1",adf::plio_32_bits);
clip_in = output_plio::create("clip_in",adf::plio_32_bits);
clip_out = input_plio::create("clip_out",adf::plio_32_bits);
plout = output_plio::create("DataOut1",adf::plio_32_bits);
```
The first parameter of the input/output plio declaration is important as this is the name that will be used on the traffic generator side to
connect to the right socket.
x86 simulation and AI Engine simulation can be launched working with the traffic generators. Launching simulation requires running the
aiesimulator or the x86simulator in parallel with the external traffic generator.

#### PL Kernels Change

When developing AI Engine applications for hardware emulation, you must model data transfers between AI Engine and programmable
logic. However during initial development phase, the PL kernels are often unfinished and not ready to be used in Vitis link. The solution


##### Displayed in the footer

is to insert hooks in the programmable logic interface to connect to external traffic generators. AMD provides a complete set of pre-
compiled .xo files that can be used for this purpose:

```
$(XILINX_VITIS)/data/emulation/XO/sim_ipc_axis_slave_32.xo,
$(XILINX_VITIS)/data/emulation/XO/sim_ipc_axis_master_32.xo
$(XILINX_VITIS)/data/emulation/XO/sim_ipc_axis_slave_64.xo,
$(XILINX_VITIS)/data/emulation/XO/sim_ipc_axis_master_64.xo
$(XILINX_VITIS)/data/emulation/XO/sim_ipc_axis_slave_128.xo,
$(XILINX_VITIS)/data/emulation/XO/sim_ipc_axis_master_128.xo
```
The .xo files must be copied to the right location in your project and specified in the configuration file during the Vitis link stage.

#### Preparing Connectivity to Link the Traffic Generators

During the Vitis link stage (v++ -l), the previously defined .xo files will be used to connect the related kernel instances to the AI
Engine graph. The hw_link.cfg configuration file is created in such a way that the kernel instance names matches the names you
defined in the graph for the input_plio and the output_plio. For example, the code below matches the PLIO assignments in the
example above:

```
[connectivity]
```
```
nk=sim_ipc_axis_master_32:1:in_interpolator
nk=sim_ipc_axis_slave_32:1:out_classifier
nk=polar_clip:1:polar_clip
```
```
sc=in_interpolator.M00_AXIS:ai_engine_0.in_interpolator
sc=ai_engine_0.out_interpolator:polar_clip.in_sample
sc=polar_clip.out_sample:ai_engine_0.in_classifier
sc=ai_engine_0.out_classifier:out_classifier.S00_AXIS
```
The format of the --connectivity.nk command is the kernel name such as sim_ipc_axis_master_32, the number of kernel
instances to create, and the names of each kernel instance (in_interpolator). Refer to --connectivity Options for more information
on the command.
The --connectivity.sc command defines the streaming connections between PL kernels, or between PL kernels and the AI
Engine graph. In the example above the output port of the traffic generator in_interpolator.M00_AXIS is connected to the input
port ai_engine_0.in_interpolator.
With this naming approach, the same external traffic generator can be used for multiple simulation or emulation runs. In the case of
hardware emulation (hw_emu), you can write the external traffic generator in C++, Python, MATLAB, or HDL if familiar with RTL coding.

#### Host Code

The host code creation is relatively simple. As there are no programmable logic kernels, you can avoid all the stages where you look for
and run the PL kernels as well as the parts where you allocate memory for all the buffer objects. The stages are:

```
Open the device
Load the xclbin file
Register XRT to connect to the design
Run the AI Engine graph
```
After compiling the host code, you can package the entire project. Running the emulation consists of running the external traffic
generator in parallel with the standard emulation launch.

Using Traffic Generators in AI Engine Graphs

#### Overview

This section describes the steps to interface external traffic generators with AI Engine components. Simulation using external traffic
generators can be run by launching the simulator/emulator and the traffic generator in parallel. The traffic generators can be written
either in Python, MATLAB, or in C++, using multi-threading capabilities of these languages. Then the AI Engine must be written to work


##### Displayed in the footer

✎

###### ★

with the traffic generator during simulation or emulation. The steps below describe interfacing the traffic generator into your AI Engine
graph for standalone AI Engine simulation or the full system emulation flow (hw_emu).
In order to interface external traffic generators with AI Engine components, you need to use certain APIs in your external script or
source code written in Python, MATLAB, or C++.
**Note:** External Traffic Generators provides only the PL stream traffic injection and recording capabilities for AI Engine Simulation
without any expectation of Cycle Accuracy or Cycle Locked simulation.

#### Using Python API

You must write a Python script using dedicated APIs to interface the external traffic generator process with the AI Engine simulation
process running the AI Engine graph.

```
# Mandatory
import os, sys
```
```
import multiprocessing as mp
import threading
import struct
```
```
from aie_input_plio import aie_input_plio
from aie_output_plio import aie_output_plio
```
```
# Optional for ease of use
import numpy as np
import logging
```
The Python traffic generator uses API described in Writing Python Traffic Generators.
Use the following steps to integrate the external traffic generator data into the graph:

1. Modify the graph code to declare the PLIO, but do not specify the PLIO based data text file in the PLIO constructors. This is
    needed for the external traffic generator to work properly. Take a note of the names (first argument) of the PLIO constructors.
    These will be used to hook up the external traffic generators and the same name should be used for creating the ports in the
    external traffic generators:

```
plin = input_plio::create("DataIn1",adf::plio_32_bits);
plout = output_plio::create("DataOut1",adf::plio_32_bits);
```
2. Instantiate the corresponding AXI master and slave connections in the Python script. This will establish connections to the PLIO
    ports of the graph:

```
pl_in = aie_input_plio("DataIn1", 'int16')
pl_out = aie_output_plio("DataOut1", 'int16')
```
```
Tip: You need to specify the PLIO datatype during object creation.
```
3. Send data to the AI Engine.
    The data to be sent to the PLIO port in the AI Engine graph must be set up in the Python script. Using the send_data() API, you
    can send the data in the form of a list.

```
plin.send_data send_data(<data_list>, tlast)
```
4. Receive data from the AI Engine.
    Use the receive_data_with_size() API to receive data from the AI Engine. This API returns the received values in
    recv_data from the following example. This is a blocking API and will wait until expected bytes of data are received. For more
    information, refer to Writing Python Traffic Generators.

```
recv_data = plout.receive_data_with_size(<expected_bytes>)
```

##### Displayed in the footer

#### Using C++ Traffic Generator APIs

When using the C++ language to implement an external traffic generator, various headers are necessary to use some libraries in the
external traffic generator source code. The headers useful for handling these libraries are:

```
# For the traffic generator
#include "xtlm_ipc.h"
#include <thread>
```
Also, the C++ traffic generator uses APIs available as part of xtlm_ipc sources. For a list of the APIs and their corresponding usage,
see Writing Traffic Generators in C++.
The Makefile dependencies are:

```
# Libraries directories
PROTO_PATH=$(XILINX_VIVADO)/data/simmodels/xsim/<vivado_version>/lnx64/6.2.0/ext/protobuf/
IPC_XTLM= $(XILINX_VIVADO)/data/emulation/ip_utils/xtlm_ipc/xtlm_ipc_v1_0/cpp/src/
IPC_XTLM_INC= $(XILINX_VIVADO)/data/emulation/ip_utils/xtlm_ipc/xtlm_ipc_v1_0/cpp/inc/
LOCAL_IPC= $(IPC_XTLM)../
```
```
LD_LIBRARY_PATH:=$(XILINX_VIVADO)/data/simmodels/xsim/<vivado_version>/lnx64/6.2.0/ext/protobuf/:$(XIL
INX_VIVADO)/lib/lnx64.o/Default:$(XILINX_VIVADO)/lib/lnx64.o/:$(LD_LIBRARY_PATH)
```
```
# Kernel directories
PLKERNELS_DIR := ../../pl_kernels
PLKERNELS := $(PLKERNELS_DIR)/polar_clip.cpp
PLHEADERS := $(PLKERNELS_DIR)/polar_clip.hpp $(PLKERNELS_DIR)/s2mm.hpp $(PLKERNELS_DIR)/mm2s.hpp
```
```
# XTLM source files
IPC_SRC := $(LOCAL_IPC)/src/axis/*.cpp $(LOCAL_IPC)/src/common/*.cpp $(LOCAL_IPC)/src/common/*.cc
```
```
# Compiler/linker flags
INC_FLAGS := -I$(LOCAL_IPC)/inc -I$(LOCAL_IPC)/inc/axis/ -I$(LOCAL_IPC)/inc/common/ -
I$(PROTO_PATH)/include/ -I$(PLKERNELS_DIR) -I$(XILINX_HLS)/include
LIB_FLAGS := -L$(PROTO_PATH)/ -lprotobuf -L$(XILINX_VIVADO)/lib/lnx64.o/ -lrdizlib -
L$(GCC)/../../lib64/ -lstdc++ -lpthread
```
```
# Compilation
compile: main.cpp $(PLHEADERS) $(PLKERNELS)
$(GCC) -g main.cpp $(PLKERNELS) $(IPC_SRC) $(INC_FLAGS) $(LIB_FLAGS) -o chain
```
Below are the steps to integrate external traffic generator using C++ APIs in the AI Engine component:

1. Declare the external PLIOs in the graph code as below:

```
plin = input_plio::create("DataIn1",adf::plio_32_bits);
plout = output_plio::create("DataOut1",adf::plio_32_bits);
```
2. Instantiate AXI master and sender.

```
xtlm_ipc::axis_master plin("DataIn1");
xtlm_ipc::axis_slave plout("DataOut1");
```
3. Prepare the data. This is the user logic.
4. Send the data.
    A simple API is available if you prefer not to have fine granular control and send the data.

```
std::vector<char> data; // The sender API expects data to be in the form of vector of char
```
```
The C++ XTLM library provides the utility conversion APIs to easily convert user readible data type to byte array:
```

##### Displayed in the footer

```
std::vector<char> type
std::vector<char> byte_array = plin.uInt8ToByteArray(user_list)
```
```
Here uint8 is the user data type and contains user list/array for uint8 type, such as std::vector<uint8> user_list;. For
more details on conversion API, see Writing Traffic Generators in C++.
```
```
// Write a user logic to fill in the data
// Send the data using send_data() API call
plin.send_data(byte_array, tlast)
```
5. Receive the data.
    A simple API is available if you do not need fine grain control. It returns the data in the form of byte array (std::vector<char>)
    and waits until expected data_size is received.

```
std::vector<char> data; // create empty vector of char
plout.receive_data_with_size(data, data_size);
```
```
The C++ library provides another set of utility conversion API to convert the received byte array into user readable data format. For
example:
```
```
std::vector<int8> recv_data;
recv_data = plout .byteArrayTouInt8(data)
```
```
For more details on the API and their usage in the external traffic generator CPP code, see Writing Traffic Generators in C++.
```
The interest of the C++ traffic generator is that you can use and test your HLS kernels as soon as they are created, without having to
synthesize them in a .xo file. This allows you to add more and more realism and flexibility to your simulations without having to
recreate a .xclbin file.

#### Using SystemVerilog Traffic Generator APIs

You can also drive traffic generators from external System Verilog/Verilog traffic generators and test benches to the aiesimulator or
x86simulator.
Prior to integrating a traffic generator module, declare the external PLIOs in the graph.

```
pl_in0 = adf::input_plio::create("in_classifier",adf::plio_32_bits);
out0 = adf::output_plio::create("out_interpolator",adf::plio_32_bits);
```
To establish the connection between the SystemVerilog/Verilog traffic generator and the AI Engine graph's external PLIOs, the
xtlm_ipc SystemC modules are required. The external AI Engine wrapper is generated based on the external PLIO declarations in
the ADF graph. Follow the steps below to generate this wrapper module for the AI Engine.

1. Use the following command to perform the ADF graph compilation to generate a scsim_config.json file that resides in the
    work/config/scsim_config.json directory. This config file contains information on the PLIOs declared in the graph.

```
v++ -c --mode aie --target hw \
--platform vek385_base \
--work_dir ./newAIE/hw/Work \
--config --config ./config.cfg aie/graph.cpp
```
2. This config file is passed as an argument to the python script available inside
    ${XILINX_VITIS}/data/emulation/scripts/gen_aie_wrapper.py to generate the Verilog-based AI Engine wrapper module. For more
    details on how to generate the AI Engine wrapper module, see External RTL Traffic Generator and AI Engine Simulation.
3. After generating the AI Engine wrapper module, you need to instantiate it in an external test bench to make the connection
    between the System Verilog/Verilog traffic generator and the AI Engine graph PLIOs. For details on integrating the wrapper
    module into the external RTL test bench, see Instantiating AI Engine Wrapper in the Test Bench.

After the connection is established, you can launch the HDL simulation in parallel with AI Enginesimulation or x86 simulator. For details
on how to launch the HDL simulation, see Running the RTL Traffic Generator with AI Engine Simulation.


##### Displayed in the footer

AXI4-Stream I/O Model for Streaming Traffic

The following section is specific to AXI4-Stream. The streaming I/O model can be used to emulate streaming traffic on the platform, and
also support delay modeling. You can add streaming I/O to your application when targeted for hardware emulation, or add them to your
custom platform design in the context of hardware emulation as described below:

```
Streaming I/O kernels can be added to the device binary (xclbin) file like any other compiled kernel object (XO) file, using the v++
--link command. Using these pre-compiled XO files reduces v++ compile time. The Vitis installation provides kernels for AXI4-
Stream interfaces of various data widths. The standard bit widths supported are 8, 16, 32, 64, 128, 256, 512. These can be found
in the software installation at $XILINX_VITIS/data/emulation/XO.
Add these to your designs using the following example command:
```
```
v++ -t hw_emu --link $XILINX_VITIS/data/emulation/XO/sim_ipc_axis_master_32.xo
$XILINX_VITIS/data/emulation/XO/sim_ipc_axis_slave_32.xo ...
```
```
In the example above, the sim_ipc_axis_master_32.xo and sim_ipc_axis_slave_32.xo provide 32-bit master and slave kernels that
can be linked with the target platform and other kernels in your design to create the .xclbin file for the emulation build.
In case of hardware emulation, IPC modules can also be added to a platform block design using the Vivado IP integrator for AMD
Versal™ and AMD Zynq™ UltraScale+™ MPSoC custom platforms. The tool provides sim_ipc_axis_master_v1_0 and
sim_ipc_axis_slave_v1_0 IP to add to your platform design. These can be found in the software installation at
$XILINX_VIVADO/data/emulation/hw_em/ip_repo.
The following is an example Tcl script used to add IPC IP to your platform design, which will enable you to inject data traffic into
your simulation from an external process written in Python or C++:
```
```
#Update IP Repository path if required
set_property ip_repo_paths $XILINX_VIVADO/data/emulation/hw_em/ip_repo [current_project]
## Add AXIS Master
create_bd_cell -type ip -vlnv xilinx.com:ip:sim_ipc_axis_master:1.0 sim_ipc_axis_master_0
#Change Model Property if required
set_property -dict [list CONFIG.C_M00_AXIS_TDATA_WIDTH {64}] [get_bd_cells sim_ipc_axis_master_0]
```
```
##Add AXIS Slave
create_bd_cell -type ip -vlnv xilinx.com:ip:sim_ipc_axis_slave:1.0 sim_ipc_axis_slave_0
#Change Model Property if required
set_property -dict [list CONFIG.C_S00_AXIS_TDATA_WIDTH {64}] [get_bd_cells sim_ipc_axis_slave_0]
```
Writing Python Traffic Generators

#### Introduction

You can include an external traffic generator process while simulating your application to dynamically generate data traffic on the I/O
traffic generators, or to capture output data from the emulation process. The AMD provided Python library can be used to create the
traffic generator code as described in the following sections.
The Python library having the API is divided into API for sending and receiving user data on instantiated traffic generator objects. There
are also advanced APIs for packet level granularity if you need more control over transactions. For seamless interaction of traffic
generators with simulation process, the API gives you control over sending or receiving the data without worrying too much about
managing packet level details like TLAST, TKEEP, TSRB, etc. This is auto managed by the sim_IPC infrastructure when using the API.
Also, an application can communicate to multiple I/O interface. It is not necessary to have each instance of I/O utilities to be in a
separate process/thread. If your application requires it, you might consider the non-blocking API to send the data.
The Python API are categorized as follows:

```
Python API for AI Engine Graphs
Python API for PL Kernels
General Purpose Python API
```
Python API for AI Engine Graphs

Here is the list of AI Engine specific APIs and their usage in integrating the traffic generators. With the Python API you can create the
traffic generator code to generate data to pass into or collect data from your AI Engine graph. Use the following API to instantiate
objects to send and receive data. You can provide any datatype vector/list to send_data or receive_data.


##### Displayed in the footer

**Instantiating Classes to Send or Receive Data**

```
aie_input_plio( name, datatype)
aie_output_plio(name, datatype)
```

##### Displayed in the footer

```
✎
```
```
✎
```
```
✎
```
Parameters:
name: A string value that should match a PLIO name from the graph.
datatype: A string with the following supported values [ "int8", "uint8","int16", "uint16","int32",
"uint32","int64", "uint64","float", "bfloat16"]

**Table: Supported AI Engine Data Types**

```
#num AIE Kernel Datatype API Enum String Usage User Value List Representation (
```
```
1 int8 int8 input_port =
aie_input_plio('input_port_name',"int8")
```
```
data = [12 23 2 -2 23]
```
```
2 int16 int16 input_port =
aie_input_plio('input_port_name',"int16")
```
```
data = [1212 121 232
-2323]
```
```
3 int32 int32 input_port =
aie_input_plio('input_port_name',"int32")
```
```
data = [ 121 23234
2323232 23 -878787]
```
```
4 int64 int64 input_port =
aie_input_plio('input_port_name',"int64")
```
```
data = [232 2342232
-23482947 238273]
```
```
5 uint8 uint8 input_port =
aie_input_plio('input_port_name',"uint8")
```
```
data = [23 23 12]
```
```
6 uint16 uint16 input_port =
aie_input_plio('input_port_name',"uint16")
```
```
data = [236 23728
2378 8237]
```
```
7 uint32 uint32 input_port =
aie_input_plio('input_port_name',"uint32")
```
```
data = [267 2376
2362736 232767362]
```
```
8 uint64 uint64 input_port =
aie_input_plio('input_port_name',"uint64")
```
```
data = [347 2348 327
98932 872389]
```
```
9 float float input_port =
aie_input_plio('input_port_name',"float")
```
```
data = [3.14 2.3234
3472.23]
```
```
10 bfloat16 bfloat16 input_port =
aie_input_plio('input_port_name',"bfloat16")
```
```
data = [3.14 2.3234
3472.23]
Note: Float and
Bfloat16
representations in
floating point are
similar. But when
transferred to AIE,
float will be transmitted
in 32 bits. bfloat16 will
be transmitted in 16
bits.
```
```
11 cfloat float input_port =
aie_input_plio('input_port_name',
"float")
```
```
data = [3.14 2.2323
23.3 23.33]
Note: From left,
even positions will be
real and odd positions
will be imaginary.
```
```
12 cint16 int16 input_port =
aie_input_plio('input_port_name',"int16")
```
```
data = [32 232 232
234]
Note: From left,
even positions will be
real and odd positions
will be imaginary.
```

##### Displayed in the footer

```
✎
```
```
✎
```
```
✎
```
```
✎
```
```
#num AIE Kernel Datatype API Enum String Usage User Value List Representation (
```
```
13 cint32 int32 input_port =
aie_input_plio('input_port_name',"int32")
```
```
data = [23 -23 23 -232]
Note: From left,
even positions will be
real and odd positions
will be imaginary.
```
```
Note: For some types like cint32 you can use int32 as it is expected that you will provide pairs of real and imaginary values
in the same list. This means the size of the user list for complex types is always even.
For example, before creating external ports, you need to do AI Engine graph modifications as described at Using Traffic
Generators in AI Engine Graphs. Once graph modifications are done, you can create the sender and receiver objects for the AI
Engine in your python script.
```
```
input = aie_input_plio("in_data", 'int16')
output = aie_output_plio("out_data", 'int16')
```
```
Here, in_data and out_data define the PLIO matching in the graph code; input and output are the objects created.
The second parameter int16 is the datatype of the interfaced AI Engine kernel with traffic generator.
```
**send_data()**

```
send_data(data, tlast)
creates a non-blocking call to send data
```
```
Parameters:
data: The list of specified datatype
tlast: Boolean value, can be true (1) or false (0)
Note: The datatype must be specified during object instantiation.
The following is an example of creating the traffic generator object and sending data through it:
```
```
input = aie_input_plio("in_data", "uint32")
input.send_data(<data_list>, "false")
```
```
This API call sends the data to the AI Engine kernel via "in_data" PLIO connected to the traffic generator.
```
**receive_data()**

```
receive_data()
creates a blocking call to receive data
```
```
RETURNS a list of specified dataype
Note: The datatype must be specified during object instantiation
For example:
```
```
recv_data = output.receive_data()
```
```
recv_data is a list containing the returned value of receive_data()
```
**receive_data_with_size()**

```
receive_data_with_size(data_size)
creates a blocking call to receive a specified amount of data
```
```
Parameters:
data_size: integer value indicating the amount of data in bytes to receive
```

##### Displayed in the footer

```
✎
```
```
✎
```
```
RETURNS a list of specified datatype
Note: Data size is specified in bytes only.
For example:
```
```
recv_data = output.receive_data_with_size(1024)
```
```
This is a blocking API which blocks until the specified bytes (1024 bytes) of data is received.
```
**receive_data_on_tlast()**

```
receive_data_on_tlast()
creates a blocking call returning data after receiving tlast packet
```
```
RETURNS a list of specified datatype
Note: The datatype must be specified during object instantiation.
For example:
```
```
recv_data = output.receive_data_on_tlast()
```
```
recv_data contains the returned data from receive_data_on_tlast(). This is a blocking API which will wait until it gets the
TLAST signal.
```
Python API for PL Kernels

Here is the list of PL kernel specific APIs and their usage in integrating the traffic generators. With the Python API you can create the
traffic generator code to generate data to pass into or collect data from PL kernels. Use the following API to instantiate objects to send
and receive data. You can provide any datatype vector/list to send_data or receive_data.

**Instantiating Classes to Send or Receive Data**
The APIs are found in the following library path:

```
${XILINX_VIVADO}/data/emulation/python/xtlm_ipc_v2/
```
```
You need to set PYTHONPATH to point to this library.
```
```
export PYTHONPATH=${XILINX_VIVADO}/data/emulation/python/xtlm_ipc_v2/
```
```
hls_input_stream(name, datatype)
hls_output_stream(name, datatype)
```

##### Displayed in the footer

```
✎
```
```
✎
```
```
Parameters:
name: string name to match the HLS kernel
datatype: A string value based on HLS kernel datatype. The supported values are [ "int8", "uint8","int16",
"uint16","int32", "uint32","int64", "uint64","float", "bfloat16"]
```
```
Table: Supported PL Kernel Data Types
```
```
PL Kernel Data Type EOU API Enum String Example
```
```
1 int8 int8 input_port =
hls_input_plio('input_port_name',"int8")
```
```
2 int16 int16 input_port =
hls_input_plio('input_port_name',"int16")
```
```
3 int32 int32 input_port =
hls_input_plio('input_port_name',"int32")
```
```
4 int64 int64 input_port =
hls_input_plio('input_port_name',"int64")
```
```
5 uint8 uint8 input_port =
hls_input_plio('input_port_name',"uint8")
```
```
6 uint16 uint16 input_port =
hls_input_plio('input_port_name',"uint16")
```
```
7 uint32 uint32 input_port =
hls_input_plio('input_port_name',"uint32")
```
```
8 uint64 uint64 input_port =
hls_input_plio('input_port_name',"uint64")
```
```
9 float float input_port =
hls_input_plio('input_port_name',"float")
```
```
10 double double input_port =
hls_input_plio('input_port_name',"double")
```
**send_data()**

```
send_data(data, tlast)
creates a non-blocking call to send data
```
```
Parameters:
data: list of specified datatype for the object
tlast: boolean value, can be true or false
Note: The datatype must be specified during object instantiation
```
**receive_data()**

```
receive_data()
creates a blocking call to receive data
```
```
RETURNS a list of specified datatype
Note: The datatype must be specified during object instantiation
```

##### Displayed in the footer

✎

✎

###### ★

**receive_data_with_size()**

```
receive_data_with_size(data_size)
creates a blocking call to receive a specified amount of data
```
```
RETURNS a list of specified datatype
Parameters:
data_size: integer value indicating the amount of data in bytes to receive
```
```
Note: Data size must be specified in bytes.
```
**receive_data_on_tlast()**

```
receive_data_on_tlast()
creates a blocking call returning data after receiving tlast packet
```
RETURNS list of specified data type
**Note:** The data type must be specified during object instantiation.

General Purpose Python API

Here is the list of general purpose APIs that can be used with custom user-defined datatypes. With these API you can send and receive
data in the form of byte_arrays only. You must convert your custom datatype to byte_arrays prior to transport, and convert the
received value back to your custom datatype for use by your application. Conversion API are provided as described below.

**Instantiating Classes to Send or Receive Data**
The APIs are found in the following library path:

```
${XILINX_VIVADO}/data/emulation/python/xtlm_ipc_v2/
```
```
You need to set PYTHONPATH to point to this library:
```
```
export PYTHONPATH=${XILINX_VIVADO}/data/emulation/python/xtlm_ipc_v2/
```
```
input_port = axis_master(name)
output_port = axis_slave(name)
```
```
Parameters:
name: string matching the AXI4-Stream interface
```
**send_data()**

```
Tip: The general purpose API expects data in the form of byte_array only. To convert it from a user data type, use the
conversion API such as uInt16ToByteArray described below.
```
```
send_data(data, tlast)
creates a non-blocking call to send data
```
```
RETURNS nothing
Parameters:
data: list created using create_byte_array() or conversion API described below
tlast: boolean value, can be true or false
For example:
```
```
input.send_data(data_byte_array, tlast)
```

##### Displayed in the footer

###### ★

```
✎
```
**receive_data()**

```
Tip: The general purpose API expects data in the form of byte_array only. To convert it into a user readable format with data
types after receiving it, use the conversion API such as byteArrayTouInt16 described below.
```
```
receive_data(data)
creates a blocking call to receive data
```
```
RETURNS nothing
Parameters:
data: a byte array that can be converted with conversion API described below
```
**receive_data_with_size()**

```
receive_data_with_size(data, data_size)
creates a blocking call to receive a specified amount of data
```
```
RETURNS a list of specified datatype
Parameters:
data: a byte array that can be converted with conversion API described below
data_size: integer value indicating the amount of data in bytes to receive
Note: Data size is specified in bytes
For example:
```
```
output.receive_data_with_size(<recv_vector>, 512)
```
```
Where recv_vector is an empty byte array that gets filled with the data received in the form of a byte array
```
**receive_data_on_tlast()**

```
receive_data_on_tlast(data)
creates a blocking call returning data after receiving tlast packet
```
```
RETURNS a list of specified datatype
Parameters:
data: a byte array that can be converted with the conversion API described in the following table.
```
**Table: Byte_Array Conversion API**

```
API Description
```
```
byte_array = input_port.uInt8ToByteArray(user_list)
byte_array = input_port.uInt16ToByteArray(user_list)
byte_array = input_port.uInt32ToByteArray(user_list)
byte_array = input_port.uInt64ToByteArray(user_list)
byte_array = input_port.int8ToByteArray(user_list)
byte_array = input_port.int16ToByteArray(user_list)
byte_array = input_port.int32ToByteArray(user_list)
byte_array = input_port.int64ToByteArray(user_list)
byte_array = input_port.floatToByteArray(user_list)
byte_array = input_port.doubleToByteArray(user_list)
byte_array = input_port.bfloat16ToByteArray(user_list)
```
```
Convert the specified data type to byte_array value to send the
data
```
```
byte_array =
input_port.uInt64ToByteArray(user_list)
```
```
Parameters:
Returns list of specified data type
converted from byte_array
user_list → std::vector<T>
// T is the data type present in function
signature.
// For example in byteArrayToFloat user_list
is a vector of type float
byte_array → list created using
create_byte_array or conversion APIs
```
```
user_list = output_port.byteArrayTouInt8(byte_array)
user_list = output_port.byteArrayTouInt16(byte_array)
user_list = output_port.byteArrayTouInt32(byte_array)
```
```
Convert the byte_array value to the specified data type after
receiving
```

##### Displayed in the footer

```
API Description
user_list = output_port.byteArrayTouInt64(byte_array)
user_list = output_port.byteArrayToInt8(byte_array)
user_list = output_port.byteArrayToInt16(byte_array)
user_list = output_port.byteArrayToInt32(byte_array)
user_list = output_port.byteArrayToInt64(byte_array)
user_list = output_port.byteArrayToFloat(byte_array)
user_list = output_port.byteArrayToDouble(byte_array)
user_list = output_port.byteArrayToBfloat16(byte_array)
```
```
user_list =
output_port.byteArrayTouInt16(byte_array)
```
```
Parameters:
Returns list of specified data type
converted from byte_array
byte_array → list created using
create_byte_array or conversion APIs
user_list → std::vector<T>
// T is the data type present in function
signature.
// For example in byteArrayToFloat user_list
is a vector of type float
```
Writing Traffic Generators in C++

#### Introduction

You can include an external traffic generator process while simulating your application to dynamically generate data traffic on the I/O, or
to capture output data from the emulation process. The AMD provided C++ library can be used to create the traffic generator code as
described in the following sections.
The C++ library having the API is divided into API for sending/receiving data in the form of byte array. The user data before sending
needs to be converted into byte array and the data is received in the form of byte array which can be further converted to user data type
using conversion APIs. There are also advanced API for packet level granularity if you need more control over transactions. For
seamless interaction of traffic generators with simulation process, the API gives you control over sending or receiving the data without
much worrying about managing packet level details like TLAST, this is auto managed by the sim_IPC infrastructure.
For C++, the APIs are available at:

```
$XILINX_VIVADO/data/emulation/cpp/inc/xtlm_ipc/axis/
```
You can build the executable with the following include path:

```
-I $XILINX_VIVADO/data/emulation/cpp/inc/xtlm_ipc/axis/
```
and linked against library as:

```
-L $XILINX_VIVADO/data/emulation/cpp/lib/
```
And use -lxtlm_ipc with a gcc compiler.
A full system-level example is available in the External_IO_CPP tutorial on GitHub.

General Purpose C++ API

Here is the list of general purpose APIs that can be used with custom user-defined datatypes. With these API you can send and receive
data in the form of byte_arrays only. You must convert your custom datatype to byte_arrays prior to transport, and convert the
received value back to your custom datatype for use by your application. Conversion API are provided as described below.

**Instantiating Classes to Send or Receive Data**

```
xtlm_ipc::axis_master input_port(std::string name)
xtlm_ipc::axis_slave output_port(std::string name)
```
```
Parameters:
name → string matching the AXI4-Stream interface
```

##### Displayed in the footer

###### ★

###### ★

**send_data()**

```
Tip: The general purpose API expects data in the form of byte_array only. To convert it from a user data type, use the
conversion API such as uInt16ToByteArray described below.
```
```
send_data(data, tlast)
creates a non-blocking call to send data
```
```
Parameters:
RETURNS nothing
data → std::vector<char>
tlast → boolean value, can be true or false
```
**receive_data()**

```
Tip: The general purpose API expects data in the form of byte_array only. To convert it into a user readable format with data
types after receiving it, use the conversion API such as byteArrayTouInt16 described below.
```
```
receive_data(data)
creates a blocking call to receive data
```
```
Parameters:
RETURNS the received data as a byte_array
data → std::vector<char>
```
```
The following example creates an empty vector for data and receives data:
```
```
std::vector<char> recv_data;
output.receive_data(<recv_data>)
```
**receive_data_with_size()**

```
receive_data_with_size(data, data_size)
creates a blocking call to receive a specified amount of data
```
```
Parameters:
RETURNS the received data as a byte_array
data → std::vector<char>
data_size -> integer value indicating the amount of data in bytes to receive
```
```
Note: data_size is specified in bytes
```
```
The following example creates an empty vector for data and receives data:
```
```
std::vector<char> recv_data;
output.receive_data_with_size(<recv_data>, 1024)
```
**receive_data_on_tlast()**

```
receive_data_on_tlast(data)
creates a blocking call returning data after receiving tlast packet
```
```
Parameters:
RETURNS the received data as a byte_array
data → std::vector<char>
```
```
Note: The datatype must be specified during object instantiation
```
```
The following example creates an empty vector for data and receives data on the TLAST signal:
```
```
std::vector<char> recv_data;
output.receive_data_on_tlast(<recv_data>)
```

##### Displayed in the footer

**Table: Byte_Array Conversion API**

```
API Description
```
```
byte_array = input_port.uInt8ToByteArray(user_list)
byte_array = input_port.uInt16ToByteArray(user_list)
byte_array = input_port.uInt32ToByteArray(user_list)
byte_array = input_port.uInt64ToByteArray(user_list)
byte_array = input_port.int8ToByteArray(user_list)
byte_array = input_port.int16ToByteArray(user_list)
byte_array = input_port.int32ToByteArray(user_list)
byte_array = input_port.int64ToByteArray(user_list)
byte_array = input_port.floatToByteArray(user_list)
byte_array = input_port.doubleToByteArray(user_list)
byte_array = input_port.bfloat16ToByteArray(user_list)
```
```
Convert the specified data type to byte_array value to send the
data
```
```
byte_array =
input_port.uInt64ToByteArray(user_list)
```
```
Parameters:
Returns list of specified data type
converted from byte_array
user_list → std::vector<T>
// T is the data type present in function
signature.
// For example in byteArrayToFloat user_list
is a vector of type float
byte_array → list created using
create_byte_array or conversion APIs
```
```
user_list = output_port.byteArrayTouInt8(byte_array)
user_list = output_port.byteArrayTouInt16(byte_array)
user_list = output_port.byteArrayTouInt32(byte_array)
user_list = output_port.byteArrayTouInt64(byte_array)
user_list = output_port.byteArrayToInt8(byte_array)
user_list = output_port.byteArrayToInt16(byte_array)
user_list = output_port.byteArrayToInt32(byte_array)
user_list = output_port.byteArrayToInt64(byte_array)
user_list = output_port.byteArrayToFloat(byte_array)
user_list = output_port.byteArrayToDouble(byte_array)
user_list = output_port.byteArrayToBfloat16(byte_array)
```
```
Convert the byte_array value to the specified data type after
receiving
```
```
user_list =
output_port.byteArrayTouInt16(byte_array)
```
```
Parameters:
Returns list of specified data type
converted from byte_array
byte_array → list created using
create_byte_array or conversion APIs
user_list → std::vector<T>
// T is the data type present in function
signature.
// For example in byteArrayToFloat user_list
is a vector of type float
```
External RTL Traffic Generators using SV/Verilog

Generate traffic using the existing test bench written in the System Verilog/Verilog with slight modification to your test bench hierarchy,
as explained below.

External RTL Traffic Generator and Emulation Process

External RTL traffic generators are used to drive traffic to Vitis emulation process or AI Engine simulation process using SystemVerilog
or Verilog modules.

**Figure: Test Bench Hierarchy**


##### Displayed in the footer

As shown in the preceding figure, the external test bench (on the left) and the Vitis emulation (on the right), both run as separate
simulation processes. To establish communication between two processes using IPC, you must instantiate SIM_IPC Master/Slave
modules.
Perform the following modifications:

1. You need to create a project in Vivado simulator. For details on how to create a project, refer _Vivado Design Suite User Guide:_
    _Design Flows Overview_ (UG892).
2. Once the project is created, you need to instantiate sim_ipc IP in the external SV/Verilog test bench.
3. Then run the export_simulation command in Vivado to generate the scripts for the simulation.
4. Run the simulation in Vivado simulator. For details running simulation refer to _Vivado Design Suite User Guide: Logic Simulation_
    (UG900).

External RTL Traffic Generator and AI Engine Simulation

The same technique can be deployed to drive traffic from the external System Verilog/Verilog traffic generators/test benches to the AI
Engine simulator or x86-simulator.

**Figure: XTLM Test Bench Hierarchy**

To generate the AI Engine wrapper stub module (aie_wrapper_ext_tb.v) use the following steps:

1. The wrapper stubs will be generated based on the external PLIO declarations in the ADF graph. You need to perform the ADF
    graph compilation to generate scsim_config.json file that resides in ./Work/config/scsim_config.json directory. This config file
    contains information on the PLIOs declared in the graph. For more details on how to perform ADF graph compilation and external
    PLIOs declaration, refer to _AI Engine Tools and Flows User Guide_ (UG1076).
2. You can use this config file as argument to the gen_aie_wrapper.py script to auto-generate Verilog stub modules based on ext
    PLIO declared in ADF Graph:


##### Displayed in the footer

###### ★ ✎ ✎ ✎ ✎ ★

```
python3 ${XILINX_VITIS}/data/emulation/scripts/gen_aie_wrapper.py \
-json Work/config/scsim_config.json --mode <wrapper/vivado>
```
```
Tip: The python script is available in the Vitis installation area as shown in the example above. There are two modes for the
script: wrapper and Vivado mode. By default, the script runs in Vivado mode.
```
The name of the instance stubs must be identical to the name of the corresponding external PLIOs in the graph and will be reflected in
the generated aie_wrapper_ext_tb.v file.
After running the gen_aie_wrapper.py script, theaie_wrapper_ext_tb.v is generated with instances of sim_ipc_axis modules that can
be directly instantiated in your external test bench.
**Note:** The module used to send data to/from external traffic generator to AI Engine simulator/x86sim are the XTLM IPC SystemC
modules which are present inside the wrapper stub module which includes all the XTLM IPC modules. This wrapper needs to be
instantiated in the external test bench to establish the connection as shown in the preceding figure.

Instantiating AI Engine Wrapper in the Test Bench

The aie wrapper module (aie_wrapper_ext_tb.v) needs to be instantiated in the external test bench. The aiesim expects data to be in
beats instead of transaction, so you need to keep tlast at high (1'b1) all the time.
**Note:** You can add timescale directive as per your requirement in aie_wrapper_ext_tb.v.
**Note:** Make sure to have the READY signal check in your test bench. Due to the fact that backpressure in the slave is modeled in AIE
simulation, the AIE will not except the data from the external test bench if it is not ready. You need to check if the m_axis_tready
(from the slave) is high or not to drive m_axis_tdata correctly.

Generating sim_ipc_axis IP for Vivado Project

By default, the python script generates aie_wrapper_ext_tb_ip.tcl and aie_wrapper_ext_tb_proj.tcl along with the wrapper Verilog file as
mentioned in the previous section.
There are two ways to proceed based on the existence of a Vivado project:

1. If you have already created Vivado project, use the IP flow described here. From the Tcl console source the
    aie_wrapper_ext_tb_ip.tcl script:

```
source <absolute_path>/aie_wrapper_ext_tb_ip.tcl
```
```
This Tcl script can be used for generating required sim_ipc_axis IP. After sourcing the Tcl file you will see hierarchy created
under simulation_sources. You can add the required files and directories for your project.
```
2. If a Vivado project is not already created, use the project script aie_wrapper_ext_tb_proj.tcl to create one. From a
    terminal use the following command:

```
vivado -mode batch -source aie_wrapper_ext_tb_proj.tcl
```
```
Note: To use third party simulators, you need to update the required paths for SIMULATOR_GCC_PATH,
SIMULATOR_CLIBS_PATH and INSTALL_BIN_PATH. For more details on how to set the third party simulators, refer to the Logic
Simulation chapter of Vivado Design Suite User Guide: Logic Simulation (UG900).
After sourcing aie_wrapper_ext_tb_proj.tcl, the tool will generate the export_sim directory with sub-directories and scripts
required for use with other simulators. This Tcl script sources the aie_wrapper_ext_tb_ip.tcl script.
```
**Tip:** The scripts mentioned above only contain the sim_ipc_axis modules, so you must add any additional required RTL modules
and options to the script. You can modify and directly include required RTL the needed script.

Running the RTL Traffic Generator with AI Engine Simulation

You can launch the external process using the following steps:

1. If already inside the Vivado project, after the project hierarchy is updated after adding the required sources, you can run the
    simulation from Vivado. Refer to _Vivado Design Suite User Guide: Logic Simulation_ (UG900) for more information.
2. If outside the Vivado project, after the export_sim directory is generated with required simulation scripts, you can traverse inside
    the appropriate simulator directory and run the <top_module_name>.sh script to launch the RTL simulation.
3. Also, simultaneously launch the AI Engine simulator process as already mentioned. For details on how to run the aiesimulator
    command, refer to _AI Engine Tools and Flows User Guide_ (UG1076).


##### Displayed in the footer

###### ★

```
✎
```
After simulation is launched you can see the traffic propagating to and from the user RTL.

**Tip:** For more details on how to integrate and launch the external RTL traffic generator with AI simulation process refer to the
tutorial.

Running Traffic Generators in Python/C++

After generating an external process binary as shown above using the headers and sources available at
$XILINX_VIVADO/data/emulation/ip_utils/xtlm_ipc/xtlm_ipc_v1_0/<supported_language>, you can run the emulation using the following
steps:

1. For C++, set the LD_LIBRARY_PATH as export
    LD_LIBRARY_PATH=$XILINX_VIVADO/data/emulation/cpp/lib:$LD_LIBRARY_PATH
2. For Python, set the PYTHONPATH as: export
    PYTHONPATH=$XILINX_VIVADO/data/emulation/hw_em/lib/python:$XILINX_VIVADO/data/emulation/python/xtlm_ipc
3. From another terminal, launch the external process such as Python/C++/C. If you are running multiple I/O or traffic generator-
    based solutions on the same machine, set XTLM_IPC_SOCK_DIR to be unique to each test case on both the emulation terminal in
    addition to the external process terminal. For example, setenv XTLM_IPC_SOCK_DIR <test_case_dir> (same environment
    on both emulation process and external process).
       **Note:** The traffic generator executable and hw_emu or x86sim/aiesim should be run on the same server/machine.

Warning: AMD provides an end_of_simulation() API to terminate emulation from master utilities of memory mapped AXI4 and
AXI4-Stream interfaces. However, you should not use this method unless there is no way to terminate emulation from the host. In a
normal course of emulation, the external process is not expected to terminate emulation. Only use the end_of_simulation() in
exceptional scenarios.

### Speed and Accuracy of Hardware Emulation

Hardware emulation uses a mix of SystemC and RTL co-simulation to provide a balance between accuracy and speed of simulation.
The SystemC models are comprised of purely functional models and performance approximate models. Hardware emulation does not
mimic hardware accuracy 100%, therefore you should expect some differences in behavior between running emulation and executing
your application on hardware. This can lead to significant differences in application performance, and sometimes differences in
functionality can also be observed.
Functional differences with hardware typically point to a race condition or some unpredictable behavior in your design. So, an issue
seen in hardware might not always be reproducible in hardware emulation, though most behavior related to interactions between the
host and the kernel, or the kernel and the memory are reproducible in hardware emulation. This makes hardware emulation an
excellent tool to debug issues with your kernel prior to running on hardware.
The following table lists models that are used to mimic the hardware platform and their accuracy levels.

**Table: Hardware Platform**

```
Hardware Functionality Description
```
```
AMD UltraScale™ DDR Memory, SmartConnect The SystemC models for the DDR memory controller, AXI
SmartConnect, and other data path IPs are usually throughput
approximate. They typically do not model the exact latency of
the hardware IP. The model can be used to gauge a relative
performance trend as you modify your application or the kernel.
```
```
AI Engine The AI Engine SystemC model is cycle approximate, though it is
not intended to be 100% cycle accurate. A common model is
used between AI Engine Simulator and hardware emulation,
thus enabling a reasonable comparison between the two stages.
```
```
AMD Versal™ NoC and DDR Models The Versal NoC and DDR SystemC models are cycle
approximate.
```
```
Arm Processing Subsystem (PS, CIPS) The Arm PS is modeled using QEMU, which is a purely
functional execution model. For more information, see QEMU.
```

##### Displayed in the footer

```
Hardware Functionality Description
```
```
User Kernel Hardware emulation uses RTL for the user kernel. As follows,
the kernel behavior by itself is 100% accurate. However, the
kernel is surrounded by other approximate models.
```
```
Other I/O Models For hardware emulation, there is generic Python or C-based
traffic generator which can be interfaced with the emulation
process. You can generate abstract traffic at AXI protocol level
which mimics the I/O in your design. Because these models are
abstract, any issues observed on the specific hardware board
will not be shown in hardware emulation.
```
Because hardware emulation uses RTL co-simulation as its execution model, the speed of execution is orders of magnitude slower as
compared to real hardware. AMD recommends using small data buffers. For example, if you have a configurable vector addition and in
hardware you are performing a 1024 element vadd, in emulation you might restrict it to 16 elements. This will enable you to test your
application with the kernel, while still completing execution in reasonable time.

# Profiling and Tracing the Application

Running the system, either in emulation or on the system hardware, presents a series of potential challenges and opportunities. When
running the system for the first time, you can profile the application to identify bottlenecks, or performance issues that offer opportunities
to optimize the design, as discussed in the sections below.
A system-level view of program execution can be helpful in identifying problems during program execution, including correctness and
performance issues. Tracing the application is useful for detecting events not synchronized in between PS, PL and AI Engine. On the AI
Engine side, issues such as missing or mismatching locks, buffer overruns, and incorrect programming of DMA buffers are examples
that are difficult to debug by using explicit print statements or by using traditional interactive debuggers. A systematic way of collecting
system level traces for the program execution is needed. The AI Engine architecture has direct support for generation, collection, and
streaming of events as trace data during simulation, hardware emulation, or hardware execution.

## Profiling the Application

The AMD Vitis™ core development kit generates various system and kernel resource performance reports during compilation. These
reports help you establish a baseline of performance for your application, identify bottlenecks, and help to identify target functions that
can be accelerated in hardware kernels as discussed in Methodology for Architecting a Device Accelerated Application in the _Data
Center Acceleration using Vitis_ (UG1700). The Xilinx Runtime (XRT) collects profiling data during application execution in both
emulation and hardware builds. Examples of profiling and event data that can be reported includes:

```
Host and device timeline events
XRT native API call sequences
Kernel execution sequence
Kernel start and stop signals
FPGA trace data including AXI transactions
Power profile data
AI Engine profiling and event trace
User event and range profiling
```
Profiling reports and data can be used to isolate performance bottlenecks in the application, identify problems in the system, and
optimize the design to improve performance. Optimizing an application requires optimizing both the application host code and any
hardware or AI Engine kernels. The host code must be optimized to facilitate data transfers and kernel execution, while the PL kernels
should be optimized for performance and resource usage and AI Engine kernels can be optimized for throughput and latency.
There are four distinct areas to be considered when performing algorithm optimization in the Vitis environment: System resource usage
and performance, kernel optimization (resource, performance, throughput, latency), host optimization, and data transfer optimization.
The following Vitis reports and graphical tools support your efforts to profile and optimize these areas:


##### Displayed in the footer

###### ★

###### ★

```
Guidance
System Estimate Report in the Data Center Acceleration using Vitis (UG1700)
HLS Synthesis Report
Profile Summary Report in the Data Center Acceleration using Vitis (UG1700)
Timeline Trace in the Data Center Acceleration using Vitis (UG1700)
Waveform View and Live Waveform Viewer in the Data Center Acceleration using Vitis (UG1700)
```
When enabled as described in Enabling Profiling in Your Application, these reports are automatically generated while running the active
build, either from the command line as described in Building and Running the System, or when Using the Vitis Unified IDE in the _Vitis
Reference Guide_ (UG1702). Separate reports are generated for the different build targets and can be found in the respective report
directories. Reports can be viewed in the Analysis view in the IDE as described in Working with the Analysis View (Vitis Analyzer) in the
_Vitis Reference Guide_ (UG1702).

Enabling Profiling in Your Application

To enable event trace data during the execution of your application, you must instrument your application for this task. You must enable
additional logic, consume additional device resources to track the host and kernel execution steps, and capture event data. This
process requires optionally modifying your host application to capture custom data, modifying your kernel XO during compilation and
the xclbin during linking to capture different types of profile data from the device side activity. You also need to configure the Xilinx
Runtime (XRT) as described in the xrt.ini File to capture data during the application runtime.

**Tip:** Even though capturing profile data is a critical part of the profiling and optimization process for building your application, it
consumes additional resources and impacts performance. Make sure to clean these elements out of your final production build.
There are many different types of profiling for your applications, depending on which elements your system includes and what type of
data you want to capture. The following table shows some of the levels of profiling that can be enabled, and discusses which are
complimentary and which are not.

**Table: Profiling Host and Kernels**

```
Profile/Trace Description Comments
```
```
Host Application XRT Native API Specified by the use of the
native_xrt_trace option in the xrt.ini
file.
```
```
Generates profile summary and trace
events for the XRT API as described in
Writing the Software Application in the
Data Center Acceleration using Vitis
(UG1700).
```
```
Host Application User-Event Profiling Requires additional code in the host
application as described in Custom
Profiling of the Host Application.
```
```
Generates user range data and user
events for the host application.
```
```
Tip: Can be used to capture event
data for user-managed kernels as
described in Working with User-Managed
Kernels in the Data Center Acceleration
using Vitis (UG1700).
```
```
Device Side Profiling Enabled by the use of --profile
options during v++ compilation and
linking, as described in --profile Options ,
and the use of device_trace in the
xrt.ini file.
```
```
Enables capturing data traffic between
the host and kernel, kernel stalls, the
execution times of kernels and compute
units (CUs), in addition to monitoring
activity in AMD Versal™ AI Engines.
```
```
AI Engine Graph and Kernels Specified by the use of the
aie_profile option in the xrt.ini file.
These options can be specified together
or separately.
```
```
Generates the default.aierun_summary
report containing the Profile. The
aierun_summary can be found in the
aiesimulator_output folder of the AI
Engine graph build directory. Refer to the
AI Engine Simulation-Based Profiling
chapter in the AI Engine Tools and Flows
User Guide (UG1076) for more
information.
```

##### Displayed in the footer

```
✎
```
###### ★

###### ★

```
Profile/Trace Description Comments
```
```
Power Profile Specified by the use of the
power_profile option in the xrt.ini file.
```
```
Generates the
power_profile_<device>.csv
report.
Note: This feature is not supported on
embedded platforms or AWS.
```
```
Vitis AI Profiling Specified by the use of the
vitis_ai_profile option in the xrt.ini
file.
```
```
Enables counter profiling of DPUs to
generate the xrt.run_summary for viewing
in Vitis analyzer.
```
The device binary (xclbin) file is configured for capturing limited device-side profiling data by default. However, using the --profile
option during the Vitis compiler linking process instruments the device binary by adding AXI Performance Monitors, and Memory
Monitors to the system. This option has multiple instrumentation options: --profile.data, --profile.stall, and --
profile.exec, as described in the --profile Options.
As an example, add --profile.data to the v++ linking command line:

```
v++ -g -l --profile.data all:all:all ...
```
**Tip:** Be sure to also use the v++ -g option when compiling your kernel code for debugging with software or hardware emulation.
After your application is enabled for profiling during the v++ compile and link process, data gathering during application runtime must
also be enabled in XRT by editing the xrt.ini file as discussed above. For example, the following xrt.ini file enables power profiling,
event and stall trace capture when the application is run:

```
[Debug]
power_profile=true
device_trace=fine
stall_trace=all
```
To enable the profiling of Kernel Internals data, you must also add the debug_mode tag in the [Emulation] section of the xrt.ini:

```
[Emulation]
debug_mode=batch
```
If you are collecting a large amount of trace data, you can increase the amount of available memory for capturing data by specifying the
--profile.trace_memory option during v++ linking, and add the trace_buffer_size keyword in the xrt.ini.

**--profile.trace_memory**
Indicates what type of memory to use for capturing trace data.

**trace_buffer_size**
Specifies the amount of memory to use for capturing the trace data during the application runtime.

**Tip:** When --profile.trace_memory is not specified but device_trace is enabled in the xrt.ini file, the profile data is captured
to the default platform memory with 1 MB allocated for the trace buffer size.

Custom Profiling of the Host Application

All XRT related actions from the host application are automatically tracked for profiling, through the XRT API calls. However, you can
also profile the host application beyond the XRT related events, capturing event data based on user-specified actions or events.
This feature provides two types of custom profiling:

**User range**
Profiles the specified start/end times across a range of code. This captures the span of time within which an action occurs in the
host application.

**User events**
Marks an event in the timeline. The user event is added to the timeline waveform at whatever point in time it occurs.

The user_range and user_event data can be captured to the Profile Summary and Timeline Trace reports for display in Vitis
analyzer. As seen in the following figure, the Profile Summary shows the number of occurrences of a given event and the range. The


##### Displayed in the footer

User Ranges table also reports the Min/Max/Avg/Total duration of the user-defined ranges in the host code. In the Timeline Trace report
user_range elements in the host code are displayed in a separate row, and user_event markers are added at specific points on the
timeline.

**Figure: Profile Summary – User Range**

Using custom profiling requires a few changes in your host application source code and build process. You must make use of C or C++
API in your code, as described below, and you must include the xrt_coreutil library when linking your host application.

```
The C/C++ API are described below, but can also be found at the following URL:
https://github.com/Xilinx/XRT/blob/master/src/runtime_src/core/include/experimental/xrt_profile.h.
For both C and C++ you must add the following:
```
```
#include experimental/xrt_profile.h
```
```
When linking host code, add -lxrt_coreutil to the compiler command line.
```
Profiling of C++ Code

For C++ code the provided objects are:

**user_range**
This object captures the start time and end time of a measured range of activity with the specified ID. The object constructor is:

```
user_range(const char* label, const char* tooltip);
```
**user_event**
This object marks an event occurring at single point in time, adding the specified label onto the timeline trace. The object
constructor is:

```
user_event()
```
Use the user_range to construct an object and start keeping track of time immediately upon construction. Usage details of the
user_range objects:

```
If a user_range is instantiated using the default constructor, no time is marked until the user calls user_range.start() with
the label and tooltip.
You can instantiate a user_range object passing the label and tooltip strings. This starts monitoring the range immediately.
You must call user_range.start() and user_range.end() to capture ranges of time you are interested in.
If user_range.end() is not called, then any range being tracked lasts until the user_range object is destructed.
The user_range object can be reused any number of times, by calling user_range.start()/user_range.end() pairs in
the host code.
Sequential calls to user_range.start() ignore all but the first call until user_range.end() terminates the range.
Sequential calls to user_range.end() ignore all but the first call until user_range.start() starts a new range.
```
Usage of the user_event objects:


##### Displayed in the footer

```
!!
```
```
A user_event object must be instantiated using the default constructor.
Calls to user_event.mark() creates a user marker on the timeline trace at that particular time.
user_event.mark() takes an optional const char* argument which appears as a label on the timeline trace.
```
With your host application properly instrumented, XRT can capture profile data from these user-defined ranges and events, in addition
to the standard XRT API-based events. You must enable profiling in thexrt.ini file as explained previously.

Profiling of C Code

For C code the provided functions are:

**xrtURStart()**
This function establishes the start time of a measured range of activity with the specified ID. The function signature is:

```
void xrtURStart(unsigned int id, const char* label, const char* tooltip)
```
**xrtUREnd()**
This function marks the end time of a measured range with the specified ID. The function signature is:

```
void xrtUREnd(unsigned int id)
```
**xrtUEMark()**
This function marks an event occurring at single point in time, adding the specified label onto the timeline trace. The function
signature is:

```
void xrtUEMark(const char* label)
```
Use the xrtURStart() and xrtUREnd() functions to start keeping track of time immediately, and specify an ID to pair the start/end
calls and define the user range. Usage details of the user_range functions:

```
Start/End ranges of one ID can be nested inside other Start/End ranges of a different ID.
It is your responsibility to make sure the IDs match for the Start/End range you are profiling.
Important: Multiple calls to xrtURStart and xrtUREnd with the same ID can cause unexpected behavior.
The user range can have a label that is added to the timeline, and a tooltip that is displayed when you place the cursor over the
user range.
```
A call to xrtUEMark() will create a user marker on the timeline trace at the point of the event.

```
xrtUEMark() lets you specify a label for the event. The label will appear on the timeline with the mark.
You can use NULL for the label to add an unlabeled mark.
```
The following is example code:

```
int main(int argc, char* argv[]) {
58
59 xrtURStart(0, "Software execution", "Whole program execution") ;
60 ...
61 //TARGET_DEVICE macro needs to be passed from gcc command line
62 if(argc != 2) {
63 std::cout << "Usage: " << argv[0] <<" <xclbin>" << std::endl;
64 return EXIT_FAILURE;
65 }
....
153 q.enqueueTask(krnl_vector_add);
154
155 // The result of the previous kernel execution will need to be retrieved in
156 // order to view the results. This call will transfer the data from FPGA to
157 // source_results vector
158 q.enqueueMigrateMemObjects({buffer_result},CL_MIGRATE_MEM_OBJECT_HOST);
159 ····
160 q.finish();
161
```

##### Displayed in the footer

```
162 xrtUEMark("Starting verification") ;
163
```
Enabling NoC-DDRMC Profiling

The Versal device programmable Network on Chip (NoC) is an AXI-interconnecting network used for sharing data between IP endpoints
in the programmable logic (PL), the processing system (PS), and other integrated blocks. This device-wide infrastructure is a high-
speed, integrated datapath with dedicated switching. The NoC system is a large-scale interconnection of instances of NoC master units
(NMUs), NoC slave units (NSUs), and NoC packet switches (NPSs), each controlled and programmed from a NoC programming
interface (NPI). There are 16 NMUs/NSUs on the VC1902, each one is capable of 16 Gb/s of throughput in each direction.
Network performance of the NoC interconnecting network can be monitored by the Vperf utility. Vperf is a Vitis tool that uses the
ChipScoPy functionality to profile the NoC and DDRMC in applications built using a v++ flow. ChipScoPy is an open-source Python
project that enables communication with and control of Versal device debug solutions. The ChipScoPy Python package allows users to
program designs and begin debugging in a few simple steps. Refer to _Profiling the NoC_ in _AI Engine Tools and Flows User Guide_
(UG1076) for more information on this process.

AI Engine Profiling

AI Engine profiling is a form of dynamic program analysis that measures the input/output throughput, the space (memory), time
complexity of a program, the usage of specific instructions, or the frequency and duration of function calls. Most commonly, profiling
information serves to aid program optimization, and more specifically, performance tuning.
You can obtain profiling data when you run your design in simulation or in hardware at runtime. Analyzing this data helps you gauge the
efficiency of the kernels, the stall and active times associated with each AI Engine, and pinpoint the AI Engine kernel whose
performance might not be optimal. This also allows you to collect data on design latency, throughput, and bandwidth by using runtime
event APIs in your PS host code or using specific parameter lines in the xrt.ini file.
There are two ways to gather this information:

1. Use performance counters built into the hardware to monitor AI Engine and memory module events. This feature can be used in
    hardware and hardware emulation.
2. Use event APIs to profile the AI Engine graph inputs and outputs from the graph host application. This feature can be used in
    simulation and hardware flows.

Enabling AI Engine Profiling

In the AI Engine, application profiling can be done at different development stage.

```
Simulation using AI Engine simulator (aiesimulator)
Hardware Emulation and Hardware run
```
AI Engine Simulation-Based Profiling

#### Profiling Data Generation

In the simulation framework, the AI Engine simulator can generate a profiling report for the complete application. This report is
generated using the flag –-profile.

```
aiesimulator –pkg-dir=Work –-profile
```
Text files and XML files are generated in the directory aiesimulator_output. Two types of files are generated for the tile located in
column C and row R. The *_funct reports the number of calls and number of cycles for each function. The *_instr is a report that
goes down to the assembly code. To visualize the report, use the Analysis View of the Vitis Unified IDE.

```
vitis -a aiesimulator_output/default.aierun_summary
```
The Profile tab opens the Profile report, which shows a menu of sections that show information.

**Summary**
Reports the total cycle count, total instruction count, and program size in memory.


##### Displayed in the footer

```
✎
```
**!!**

**Function Reports**
Shows several key indicators of the functions in the graphs.
**Number of calls**
Reports the number of times the function is executed
**Total function time (cycles and %)**
Reports the function execution time (in cycles and as a percent). This is the time required to execute the code within a
function, exclusive of any calls to its descendants.
**Total function + descendant time (cycles and %)**
Reports the function execution time, as well as the execution time of the descendant functions (descendant functions are
functions called by the function whose profile information is being reported). The "Total Function+descendant time"
represents the total time required to execute the code within a function and in any function it calls, including the time spent in
its descendant functions.
**Note:** The time includes the time spent in the function itself as well as the time spent in all the functions it calls, directly or
indirectly.
**Min/Avg/Max function time (cycles)**
Reports the minimum/average/maximum function execution time (in cycles and as a percent).
**Min/Avg/Max function + descendant time (cycles)**
Reports the minimum/average/maximum function execution time, as well as the execution time of the descendant functions
(descendant functions are functions called by the function whose profile information is being reported).
**Program counter Low/High**
Reports the lowest and highest program counter value for a specific function.

**Profile Details**
Shows the assembly code, function by function, with useful precisions.

For more details on Profile Details and debugging performance issues, refer to the chapter for AI Engine Simulation Based Profiling in
_AI Engine Tools and Flows User Guide_ (UG1076).

#### Using Printf for Basic Debug

The simplest form of tracing is to use a formatted printf() statement in the code for printing debug messages. Visual inspection of
intermediate values, addresses, etc. can help you understand the progress of program execution. No additional include files are
necessary for using printf() other than standard C/C++ includes (stdio.h). You can add printf() statements to your code to be
processed during simulation, or hardware emulation, and remove them or comment them out for hardware builds.
Adding printf statements to your AI Engine kernel code will increase the compiled size of the AI Engine program. Be careful that the
compiled size of your kernel code does not exceed the per-AI Engine processor memory limit of 16 KB.
**Important:** You must use the aiesimulator --profile command to enable the printf() execution during a simulator run. If -
-profile is not specified, the printf() function is ignored.
A separate driver and binary is used for this functionality to allow the main simulator to remain as fast as possible. Using the debug
simulator driver produces a per-tile profile report under the output directory which gives detailed cycle-level statistics of kernel
execution. In addition, using the --profile option generates a run_summary file that is written to the ./aiesimulator_output folder that
can be viewed as described in the _AI Engine Tools and Flows User Guide_ (UG1076).

Hardware Emulation and Hardware Run Profiling

AI Engine profiling uses performance counters at all level of the device:

```
Runtime event performance counters for the AI Engine modules
Runtime memory counters for memory modules and memory tiles
Runtime interface counters for AI Engine-PL interface tiles.
```
These performance counters can be configured to track a variety of events in the AI Engine, the memory module and the interface tile.
Various features such as error-correction code (ECC) scrubbing, event trace and profiling can use these performance counters.
Performance counters count occurrences of a given event in a profile configuration. The profile feature offers several different
configurations of these performance counters that can be dynamically applied at runtime to collect various profiling statistics.
No changes are required in PS host code when using performance counters. These counters can be configured, read and collected at
runtime while the design is executing in hardware. The following table lists the number of performance counters that are available at


##### Displayed in the footer

different configurations.
Various metrics exist for all different part of the array:

**Table: AI Engine Metrics**

```
Metric Name Description
```
```
heat_map Reports time where the AI Engine was active, stalled, executing
vector instruction.
```
```
stalls Reports time the AI Engine is not active due to memory access,
stream access, cascade access or lock acquisition.
```
```
execution Reports the time spent by the AI Engine on vector instructions,
load/store Instructions and cumulative instruction time
```
```
floating_point Reports time spent on floating-point exceptions
```
```
aie_trace Reports the amount of data for trace, back-pressure, memory
module and memory module back-pressure produced by the AI
Engine.
```
```
write_throughputs Reports the time spent by the AI Engine on executing write
operations on streams, cascade interface. There is also the write
throughput on these interface
```
```
read_throughput Reports the time spent by the AI Engine on executing read
operations on streams, cascade interface. There is also the write
throughput on these interface
```
```
stream_put_get Reports time spent on executing cascade and stream operations
```
**Table: Memory Module Metrics**

```
Metric Name Description
```
```
conflicts Reports time spent on memory conflicts and ECC errors
```
```
dma_locks Reports time spent on stalled locks on both channels
```
```
dma_stalls_s2mm Reports the time spent by each S2MM channel on stalls due to
lock acquisition
```
```
dma_stalls_mm2s Reports the time spent by each MM2S channel on stalls due to
lock acquisition
```
```
s2mm_throughputs Reports the number of BD packets and the throughput of each
S2MM channel. In AI Engine-ML the back-pressure time is also
available.
```
```
mm2s_throughputs Reports the number of BD packets and the throughput of each
MM2S channel. In AI Engine-ML the back-pressure time is also
available.
```
**Table: Memory Tile Metrics**

```
Metric Name Description
```
```
s2mm_channels Reports Transfert/Stalled time, Number of AXI4-Stream packets
and BD packets transferred over memory tile input channel
```
```
s2mm_channels_details Reports Transfer, Backpressure, lock stall and stream starvation
time on input streams
```

##### Displayed in the footer

```
Metric Name Description
```
```
mm2s_channels Reports Transfert/Stalled time, Number of AXI4-Stream packets
and BD packets transferred over memory tile output channel.
```
```
mm2s_channels_details Reports Transfer, Backpressure, lock stall and stream starvation
time on output streams
```
```
memory_stats Reports Group Errors on Memory
```
```
s2mm_throughputs Reports Transfer, Starvation, Backpressure, lock stall time along
with S2MM Channel Throughput.
```
```
mm2s_throughputs Reports Transfer, Starvation, Backpressure, lock stall time along
with MM2S Channel Throughput.
```
```
conflict_stats N Reports the number of 4 consecutive memory bank conflicts,
starting at bank 4 N. N=0,1,2,3
```
**Table: Interface Tile Metrics**

```
Metric Name Description
```
```
input_throughputs Reports Transfer, Stalled, Idle time as well as throughput
```
```
output_throughputs Reports Transfer, Stalled, Idle time as well as throughput
```
```
input_stalls Reports Stall and Idle time for channel 0. For AI Engine-ML it will
be Backpressure and Starvation time for channels 0 and 1
```
```
output_stalls Reports Stall and Idle time for channel 0. For AI Engine-ML it will
be Backpressure and Lock Stall time for channels 0 and 1
```
```
packets Reports number of packets (input/output)
```
```
start_to_bytes_transferred Total clock cycles to transfer byte count for specified graph/port
```
```
interface_tile_latency Total latency in clock cycles between graph1:port1 and
graph2:port2
```
For more details on these metrics, see the chapters on Profiling the AI Engine, Memory Module and Interface Tile in _AI Engine Tools
and Flows User Guide_ (UG1076).

#### Launch AI Engine Profiling

There are two ways to launch AI Engine profiling in Hardware:

```
XRT flow
XSDB flow
```
#### XRT Flow

In order to use the XRT flow, create the xrt.ini file at the same location where the PS host application is located. Specify a line
making AI Engine profiling possible, followed by multiple lines specifying the exact settings of the metrics to be used.
An example of xrt.ini file is as follows:

```
[Debug]
#
# Profile Counters
#
aie_profile = true
```
```
[AIE_profile_settings]
```

##### Displayed in the footer

```
# Sample interval (in usec)
interval_us = 100
# All tiles
tile_based_aie_metrics = all:heat_map
tile_based_aie_memory_metrics = all:conflicts
tile_based_interface_tile_metrics = all:s2mm_throughputs:0
```
where:

**[Debug]**
Specifies debug section for XRT, this is case sensitive.

**aie_profile**
Enables profile configuration.

**[aie_profile_settings]**
Specifies profile settings for XRT.

**aie_profile_interval_us**
Profiles data collection interval in micro seconds.

**tile_based_aie_metrics**
Configures metric to be applied to the AI Engine on a tile basis.

**tile_based_aie_memory_metrics**
Configures memory metric to be applied on a tile basis.

**tile_based_interface_tile_metrics**
Configures interface metric to be applied on a tile basis.

There exist many ways to define the tiles you want to select for profiling based on tiles or on graph.
For more details, see the chapters on Profiling the AI Engine in Hardware, Profiling Flow and XRT Flow in the _AI Engine Tools and
Flows User Guide_ (UG1076).

#### XSDB Flow

When running the application, the profile data is captured in counters that can be retrieved by the debugging and profiling IP. To capture
and evaluate this data, you must connect to the hardware device using xsdb. This command is typically used to program the device
and debug applications. Connect your system to the hardware platform or device over JTAG, launch the xsdb command in a command
shell, and run the following sequence of commands:

```
xsdb% connect
xsdb% ta 1
xsdb% source $::env(XILINX_VITIS)/scripts/vitis/util/aie_profile.tcl
xsdb% aieprofile start -graphs myGraph -work-dir ./Work \
-graph-based-aie-metrics "dut:kernel1:heat_map" \
-tile-based-aie-metrics "all:stalls" \
-graph-based-aie-memory-metrics "dut:all:write_throughputs" \
-tile-based-aie-memory-metrics "{4,1}:{6,2}:conflicts; {8,3}:dma_locks" \
-tile-based-interface-tile-metrics "2:10:input_throughputs:3" \
-interval 20 -samples 100
```
where:

**connect**
Launches the hw_server and connects xsdb to the device.

**source $::env(XILINX_VITIS)/scripts/vitis/util/aie_profile.tcl**
Sources the Tcl trace command to set up the xsdb environment.

For more details, see the chapters on Profiling the AI Engine in Hardware, Profiling Flow and XRT Flow in the _AI Engine Tools and
Flows User Guide_ (UG1076).


##### Displayed in the footer

**!!**

HLS Synthesis Report

The HLS compiler generates a number of reports for simulation, synthesis, co-simulation. These reports provide details about the high-
level synthesis (HLS) compilation of a PL kernel. The main report is the Synthesis Summary report that provides estimated FPGA
resource usage, operating frequency, latency, and interface signals of the custom-generated hardware logic. These details provide
many insights to guide kernel optimization.
When running from the Vitis Unified IDE, this report can be found in the HLS component directory named
<hls_component>.hlscompile_summary. The Summary report can be opened from the Flow Navigator in the HLS component under the
C Synthesis/Reports heading, or by opening the Compile Summary, or the Link Summary as described in Working with the Analysis
View (Vitis Analyzer) in the _Vitis Reference Guide_ (UG1702).

**Figure: Synthesis Summary Report**

Generating and Opening the HLS Report

**Important:** You must specify the --save-temps option during the build process to preserve the intermediate files produced by Vitis
HLS, including the reports. The HLS report and HLS guidance are only generated for hardware emulation and system builds for C
kernels.
The HLS report can be viewed through the Vitis analyzer by opening the <output_filename>.compile_summary or the
<output_filename>.link_summary for the application project. The <output_filename> is the output of the v++ command.
You can launch the Vitis analyzer and open the report using the following command:

```
vitis_analyzer <output_filename>.compile_summary
```
When the Vitis analyzer opens, it displays the Compile Summary and a collection of reports generated during the compile process.
Refer to Working with the Analysis View (Vitis Analyzer) in the _Vitis Reference Guide_ (UG1702) for more information.

Interpreting the HLS Report

The HLS Synthesis report lists the module hierarchy in the left column. This section describes one section of the HLS Synthesis report:
Performance and Resource Estimates. Each module and loop generated by the HLS run is represented in this hierarchy. The HLS
Synthesis report contains the following columns:


##### Displayed in the footer

⚠ **CAUTION!**

```
Issue Type
Slack
Latency in clock cycles
Latency in absolute time (ns)
Iteration Latency
Interval
Trip Count
Pipelined
Utilization Estimates of BRAM, DSP, FF, and LUT
```
If the information is part of a hierarchical block, the information of the blocks contained in the hierarchy are summed up. Therefore, the
hierarchy can also be navigated from within the report when it is clear which instance contributes to the overall design.
The absolute counts of cycles and latency numbers are based on estimates identified during HLS synthesis, especially
with advanced transformations, such as pipelining and dataflow. Therefore, these numbers might not accurately reflect the final results.
If you encounter question marks in the report, this might be due to variable bound loops, and you are encouraged to set trip counts for
such loops to have some relative estimates presented in this report.

Guidance

The Vitis core development kit has a comprehensive design guidance tool that provides immediate, actionable guidance to the software
developer for issues detected in their designs. These issues might be related to the source code, or due to missed tool optimizations.
Also, the rules are generic rules based on an extensive set of reference designs. Therefore, these rules might not be applicable for your
specific design. It is up to you to understand the specific guidance rules and take appropriate action based on your specific algorithm
and requirements.
Guidance is generated from the Vitis HLS, Vitis profiler, and AMD Vivado™ Design Suite when invoked by the v++ compiler. The
generated design guidance can have several severity levels: errors, warning messages, and informational messages are provided
during hardware emulation, and system builds. The profile design guidance helps you interpret the profiling results which allows you to
focus on improving performance.
Guidance includes message text for reported violations, a brief suggested resolution, and a detailed resolution provided as a web link.
You can determine your next course of action based on the suggested resolution. This helps improves productivity by quickly
highlighting issues and directing you to additional information in using the Vitis technology.
Design guidance is automatically generated after building or running an application from the command line or Vitis Unified IDE.
You can open the Guidance report as discussed in Working with the Analysis View (Vitis Analyzer) in the _Vitis Reference Guide_
(UG1702). To access the Guidance report, open the Compile Summary, the Link Summary, or the Run Summary, and open the
Guidance report.

```
Kernel Guidance is generated by the Vitis HLS tool after kernel is built using v++ compile command. This can be viewed in the
Vitis analyzer by opening the Compile Summary report. Kernel guidance and Compile Summary files are generated for each
kernel compiled. Kernel guidance includes recommendations on using Dataflow, as well as possible reasons why the expected
throughout could not be achieved.
System Guidance is generated after the .xclbin or .xsa is built using the v++ link command. This can be viewed in the Vitis
analyzer by opening the Link Summary report. System guidance includes all Kernel Guidance checks, and provides
comprehensive review before running your application.
Run Guidance is generated when your generated .xclbin is run, and is a feature of the XRT. This can be viewed by opening the
Run Summary in the Vitis analyzer. Run Guidance includes checks like if Kernel Stall is above 50%, recommendations if PLRAM
can be used instead of DDR memory, etc.
```
With the Guidance report open, the Guidance view displays the messages along with resolution columns. The resolutions also have
extended weblink help available.
The following image shows an example of the Guidance report displayed in the Vitis analyzer. For example, clicking a link in the Name
column opens a description of the rule check. Links in the Details column can open source code, select a design object such as a
kernel, or navigate to another report.

**Figure: Design Guidance Example**


##### Displayed in the footer

There is one HTML guidance report for each run of the v++ command, including compile and link. The report files are located in the --
report_dir under the specific output name. For example:

```
v++_compile_<output>_guidance.html for v++ compilation
v++_link_<output>_guidance.html for v++ linking
```
You can click the web link in the Resolution column to get additional details about the resolution. The Vitis HLS Guidance Messaging
web page lists all of the current messages for your review.
Kernel objects, in addition to profile reported data values, can also be cross-probed to other views like the Profile Report.

Opening the Guidance Report

When kernels are compiled and when the FPGA binary is linked, guidance reports are generated automatically by the v++ command.
You can view these reports in the Vitis analyzer by opening the <output_filename>.compile_summary or the
<output_filename>.link_summary for the application project. The <output_filename> is the output of the v++ command.
As an example, launch the Vitis analyzer and open the report using this command:

```
vitis_analyzer <output_filename>.link_summary
```
When the Vitis analyzer opens, it displays the link summary report, as well as the compile summaries, and a collection of reports
generated during the compile and link processes. Both the compile and link steps generate Guidance reports to view by clicking the
Build heading on the left-hand side. Refer to Working with the Analysis View (Vitis Analyzer) in the _Vitis Reference Guide_ (UG1702) for
more information.

Interpreting Guidance Data

The Guidance view places each entry in a separate row. Each row might contain the name of the guidance rule, threshold value, actual
value, and a brief but specific description of the rule. The last field provides a link to reference material intended to assist in
understanding and resolving any of the rule violations.
In the GUI Guidance view, guidance rules are grouped by categories and unique IDs in the Name column and annotated with symbols
representing the severity. These are listed individually in the HTML report. In addition, as the HTML report does not show tooltips, a full
Name column is included in the HTML report as well.
The following list describes all fields and their purpose as included in the HTML guidance reports.

**Id**
Each guidance rule is assigned a unique ID. Use this id to uniquely identify a specific message from the guidance report.

**Full Name**
The Full Name provides a less cryptic name compared to the mnemonic name in the Name column.

**Categories**
Most messages are grouped within different categories. This allows the GUI to display groups of messages within logical
categories under common tree nodes in the Guidance view.


##### Displayed in the footer

✎

**Threshold**
The Threshold column displays an expected threshold value, which determines whether or not a rule is met. The threshold values
are determined from many applications that follow good design and coding practices.

**Actual**
The Actual column displays the values actually encountered on the specific design. This value is compared against the expected
value to see if the rule is met.

**Details**
The Details column describes the specifics of the current rule.

**Resolution**
The Resolution column provides a pointer to common ways the model source code or tool transformations can be modified to
meet the current rule. Clicking the link brings up a popup window or the documentation with tips and code snippets that you can
apply to the specific issue.

### Tracing The Application

Tracing the application involves a different process whether you have a Linux OS running on the board or if this is a bare-metal
application. Linux OS opens the possibility of using XRT while a baremetal application forces you to use xsdb to issue trace
commands.
During tracing the system with Linux OS, special events such as XRT API commands in the host code (for example start a kernel, data
transfer, kernel start, stall or lock stall in the AI Engine array) are recorded along with their timestamps to display the sequence of
events. Through this, you can understand the origin of a system stall or data coherency.
Tracing the system can be done at multiple levels:

```
Through the application that uses XRT API to start and monitor PL kernels and AI Engine graphs, or
Through AI Engine array kernels, memories and interfaces.
```
Host application event tracing is enabled in the file xrt.ini, and can be limited within the host code using specific API.
AI Engine event trace tools provide an in-depth investigation into the operation and performance of a design. In support of this, a
number of settings are required to capture trace data at runtime. In hardware, you must prepare the design when compiling the AI
Engine graph application to ensure the libadf.a supports capturing trace data at runtime. In order to trace events in hardware, use the -
-event trace option when compiling the AI Engine graph. This option sets up the hardware device to capture AI Engine runtime
trace data.
The AI Engine event trace flow consists of three parts:

1. Event trace build flow.
2. Running the design in hardware and capturing trace data at runtime.
3. Viewing and analyzing the trace data using the Vitis IDE.

**Note:** Event trace is also supported on BDC platforms vck_190_bdc, as described in Vitis Base Platform for the VCK190 Board, and
vek_280_bdc, as described in Vitis Base Platform for the VEK280 Pre-production Board

Enabling Trace in Your Application

To enable event trace data during the execution of your application, you must instrument your application for this task. You must enable
additional logic, and consume additional device resources to track the host and kernel execution steps, and capture event data. This
process requires optionally modifying your host application to capture custom data, modifying your kernel XO during compilation and
the xclbin during linking to capture different types of profile data from the device side activity, and configuring the Xilinx Runtime (XRT)
as described in the xrt.ini File or using xsdb command line to capture data during the application runtime.
In these traditional hardware event trace, the trace information is stored in DDR memory available in the Versal device initially, and
offloaded to SD card after the application run completes. This imposes limitations on the amount of trace information that can be stored
and analyzed.
The high-speed debug port (HSDP) debug port provides debugging and trace capability for programmable logic (PL), processing
system (PS), and AI Engines through a dedicated Aurora interface and a high-speed debug cable like SmartLynq+. The HSDP
leverages the high-speed gigabit transceivers to make debug less intrusive to the system configuration. AI Engine trace offload via
HSDP has more DDR memory in the SmartLynq+ module and supports analyzing large quantities of trace information for complex
designs. In addition, the SmartLynq+ module offers high bandwidth connectivity to offload trace information via HSDP which is faster
that standard JTAG connection. HSDP bandwidth is lower than direct DDR storage but allows much larger trace data set to be stored


##### Displayed in the footer

###### ★

###### ★

###### ★

and analyzed. More details are available on Event Trace Offload using High Speed Debug Port in the _AI Engine Tools and Flows User
Guide_ (UG1076).

**Tip:** While capturing profile data is a critical part of the profiling and optimization process for building your application, it does
consume additional resources and impacts performance. You should be sure to clean these elements out of your final production build.
There are many different types of profiling for your applications, depending on which elements your system includes, and what type of
data you want to capture. The following table shows some of the levels of profiling that can be enabled, and discusses which are
complimentary and which are not.

**Table: Event Trace For Host Application, PL Kernels, and AI Engine Graphs**

```
Trace Description Comments
```
```
Host Application XRT Native API Specified by the use of the
native_xrt_trace option in the xrt.ini
file.
```
```
Generates profile summary and trace
events for the XRT API as described in
Writing the Software Application in the
Data Center Acceleration using Vitis
(UG1700).
```
```
Host Application User-Event Profiling Requires additional code in the host
application as described in Custom
Profiling of the Host Application.
```
```
Generates user range data and user
events for the host application.
```
```
Tip: Can be used to capture event
data for user-managed kernels as
described in Working with User-Managed
Kernels in the Data Center Acceleration
using Vitis (UG1700).
```
```
AI Engine Graph and Kernels Specified by the use of the aie_trace
options in the xrt.ini file.
```
```
Generates the default.aierun_summary
report containing the Trace reports. The
aierun_summary can be found in the
aiesimulator_output folder of the AI
Engine graph build directory. Refer to the
AI Engine Simulation-Based Profiling
chapter in the AI Engine Tools and Flows
User Guide (UG1076) for more
information.
```
The device binary (xclbin) file is configured for capturing limited device-side profiling data by default. However, using the --profile
option during the Vitis compiler linking process instruments the device binary by adding AXI Performance Monitors, and Memory
Monitors to the system. This option has multiple instrumentation options: --profile.data, --profile.stall, and --
profile.exec, as described in the --profile Options.
As an example, add --profile.data to the v++ linking command line:

```
v++ -g -l --profile.data all:all:all ...
```
**Tip:** Be sure to also use the v++ -g option when compiling your kernel code for debugging with software or hardware emulation.
After your application is enabled for profiling during the v++ compile and link process, data gathering during application runtime must
also be enabled in XRT by editing the xrt.ini file as discussed above.
To enable the profiling of Kernel Internals data, you must also add the debug_mode tag in the [Emulation] section of the xrt.ini:

```
[Emulation]
debug_mode=batch
```
If you are collecting a large amount of trace data, you can increase the amount of available memory for capturing data by specifying the
--profile.trace_memory option during v++ linking, and add the trace_buffer_size keyword in the xrt.ini.

**--profile.trace_memory**
Indicates the type of memory to use for capturing trace data.

**trace_buffer_size**
Specifies the amount of memory to use for capturing the trace data during the application runtime.


##### Displayed in the footer

###### ★

###### ★

**!!**

**Tip:** When --profile.trace_memory is not specified but device_trace is enabled in the xrt.ini file, the profile data is captured
to the default platform memory with 1 MB allocated for the trace buffer size.
Finally, as discussed in Continuous Trace Capture you can enable continuous trace capture to continuously offload device trace data
while the application is running, so in the event of an application or system crash, some trace data is available to help debug the
application.

Continuous Trace Capture

The Vitis tool supports recording continuous trace data while the application is running. The application can run for a very long time thus
leading to the capture of significant trace data, which can result in issues like incomplete trace data especially when the memory
resource used for trace data is not large enough. Using continuous trace, analysis of the trace can be carried out while the application
is still running or if the application has crashed before completion.
With the ability to continuously capture trace data, the Timeline Trace reports can be dynamically updated in the Vitis analyzer tool while
your application is running. Once these reports are loaded in Vitis Analyzer, there is a hyperlink available indicating that the current
report is being modified on the disk. If new data needs to be loaded, Reload or Auto-Reload options are available on the banner to let
you view the updated report as your application runs and trace data is generated.
Continuous trace is not enabled by default. Additionally, the memory resources of an FPGA are not unlimited. So if the application
generates large trace data, a circular buffer for storing the data can be used. The circular buffer can be written, offloaded to the host,
and reused again. By enabling a circular buffer with continuous trace, the memory resources needed are even smaller thus saving
available resources on the device. However, an application run with continuous trace/circular buffer can result in multiple device trace
files.

**Tip:** For Hardware emulation, only host side continuous trace is available, for hardware runs both host side and device side
continuous trace are available.
Here are some scenarios where it is recommended to use the memory resource as a circular buffer.
The circular buffer implementation is automatically turned on when continuous trace is enabled in the xrt.ini. The flow requires the
following settings for enabling continuous trace.

```
In the xrt.ini file, continuous_trace is set to TRUE
v++ linking option --profile.trace_memory is set to DDR or HBM
```
You can optionally set:

```
The size of the trace buffer using trace_buffer_size in the xrt.ini file. This defaults to 1 MB.
The interval at which the trace buffer is offloaded from the device using trace_buffer_offload_interval_ms in the xrt.ini
file. The default is 10 ms.
The interval at which files are dumped by setting trace_file_dump_interval_s. The default is 3 seconds.
```
**Important:** Circular Buffer can be force enabled by setting trace_buffer_offload_interval_ms to 0 ms.
As an example, if you enable continuous_trace with trace_buffer_size as 8k and default
trace_buffer_offload_interval_ms of 10 ms, the trace data rate is 819200 bytes/s which is less than the default of 100 MB/s.
In this scenario, the circular buffer is _NOT enabled_ by default and an XRT warning is reported:

```
[XRT] WARNING: Unable to use circular buffer for continuous trace offload. Please increase trace
buffer size and/or reduce continuous
trace interval. Minimum required offload rate (bytes per second) : 104857600 Requested offload rate :
819200
```
Here is an example of xrt.ini settings:

```
[Debug]
device_trace=fine
stall_trace=all
continuous_trace=true
// The following are optional and needed only in rare circumstances
```
```
trace_buffer_size=20M
trace_buffer_offload_interval_ms=10
trace_file_dump_interval_s=2
```
The following are the results of these settings:


##### Displayed in the footer

```
✎
```
```
device_trace: Enables the collection of kernel activity to be added to profile summary and trace, device_trace_0.csv files
are created with 0 being the device number.
stall_trace: Enables the hardware generation of stalls into kernel instances.
continuous_trace: Enables the continuous dumping of files for trace and the continuous reading of device data into the host.
trace_buffer_size: Specifies the amount of memory to consume for trace data capture.
trace_buffer_offload_interval_ms: Controls the reading of device data from the device to the host in milliseconds.
trace_file_dump_interval_s: Controls the time between dumping of trace files in seconds.
```
As a result, there are several CSV files generated in addition to the xrt.run_summary as part of the application run using the above
xrt.ini file. Vitis Analyzer only needs the generated run_summary file and will use the relevant CSV files to display the profile summary
and timeline trace.
The following are recommendations on setting up an application for trace data dumping:

1. By default the memory used for trace capture is the first memory resource on the platform, which can be determined using the
    platforminfo Utility in the _Vitis Reference Guide_ (UG1702). In most platforms this is either DDR or HBM. The amount of memory
    reserved for trace data is determined by the trace_buffer_size switch in the xrt.ini file, which defaults to 1 MB.
       **Note:** You can also specify the use of FIFO and the size to allocate using the --profile.trace_memory option.
2. If still unable to dump maximum trace, disable stall trace by setting stall_trace=off or stall_trace=on with
    device_trace=coarse.
3. If the application requires larger size of trace buffer, enable circular buffer by setting continuous_trace=true with default
    settings of trace_buffer_offload_interval_ms=10 and trace_file_dump_interval_s=5. Ideally, a continuous trace
    feature should be used for the following cases:
       Long-running design with minimal trace generated
       Debugging application crashes where some .csv files might still be available for debugging
4. If the application run is still unable to dump the maximum trace, the trace_buffer_size can further be increased.
5. If the application still creates huge trace data that the host cannot keep up, use the smaller size of
    trace_file_dump_interval, which creates multiple files equivalent to the interval provided.
6. Lastly, continuous trace can generate several trace files as part of the application run in addition to xrt.run_summary file. The
    Vitis Analyzer only needs the generated run_summary file and can pick the relevant CSV files generated to display profile
    summary and timeline trace to provide a better experience.

Enabling AI Engine Status Generation

The AI Engine provides the ability to output a summary of AI Engine status, which include error events that have occurred. Warnings
are also issued when a deadlock is detected in the hardware. The status and warnings can be further analyzed in the AMD Vitis™ IDE
for debug purposes.
There are two ways to output AI Engine status when running designs in hardware.

**Automated and periodic AI Engine status output**
After initial setup in xrt.ini, this method requires minimal user intervention, because the tool will output periodic status at specified
time intervals.

**Manual output the AI Engine status**
You must run a command each time you want a status output report.

Next, you can open the status output in the IDE for further analysis.
The following section covers the steps needed to obtain status output, and analysis of that output. For more details on understanding
stalls, see the chapter on AI Engine Stall Analysis in _AI Engine Tools and Flows User Guide_ (UG1076).

#### Periodic AI Engine Status Output Using XRT

You can enable the runtime deadlock detection and status output using the xrt.ini file. This is a one-time set up, which results in
periodic output of status data, including deadlock detection. To turn on this feature, add the following to the xrt.ini file:

```
[Debug]
aie_status=true
```
To specify the interval of probing and analysis the AI Engine status, use the code below.


##### Displayed in the footer

```
[Debug]
aie_status=true
aie_status_interval_us=1000
```
The AI Engine status is copied to the following files when the host program is running.

```
xrt.run_summary: Run summary information that can be used by the IDE.
aie_status_<device>_<current time>.json: Status of AI Engine, AI Engine memory and AI Engine Interface tiles.
summary.csv: Always created for other profiling capabilities, like guidance.
```
For more details, see the chapter on Generating AI Engine Status in _AI Engine Tools and Flows User Guide_ (UG1076).

#### Generating AI Engine Status Using XSDB

It is possible to examine the status of the AI Engine using XSDB both on Linux and Bare metal operating systems. This feature allows
you to debug applications and detect the status of the AI Engine in situations where the board is in a deadlock or hung state. Unlike the
xrt-smi command which requires XRT, the XSDB command is run independent of XRT.
XSDB reports the AI Engine status in .json file format using the aiestatus examine command. You can use this command before, during
and after running the application.
XSDB command is very simple:aiestatus examine
For all available options, see the chapter on Generating AI Engine Status using XSDB in _AI Engine Tools and Flows User Guide_
(UG1076).

Enabling Trace in AI Engine Based Simulation

The AI Engine architecture has direct support for generation, collection, and streaming of events as trace data during simulation,
hardware emulation, or hardware execution. For the AI Engine component aiesim launch configuration, you can enable trace and
select trace options as shown below.

**Figure: AI Engine Launch Configuration - Enable Trace**

After selecting the Enable Trace check box, you can then specify trace options:


##### Displayed in the footer

**Trace Type**
Specify the format of the trace data as either online waveform database (WDB) or value change dump (VCD) format. The online
WDB file is generated on-the fly to display in the Analysis view. The VCD can contains a detailed dump of the changing hardware
signals in the form of value change dump (VCD) files which must be post-processed. After simulation, or emulation, the output file
can be processed into events and viewed on a timeline in the Analysis view. The events contain information such as time stamps,
different event types, and data associated with each event. This information can also be correlated to the compiler generated
debug information.

**Trace Modules**
You can enable the capture of trace data from different elements of the AI Engine component as shown above. Enable the specific
modules of interest, or all modules.

Using command-line interface you can also generate a trace file using options of the aiesimulator:

```
aiesimulator –-pkg-dir=./Work --dump-vcd=filename
```
The --dump-vcd=filename allows you to have access to a subset of the events by default. If you want another subset, you need to
add an option file: --options-file=filename.
This option file is a text file that contains a description of the signals that you want to capture:

```
AIE_DUMP_VCD_IO=true/false
AIE_DUMP_VCD_CORE=true/false
AIE_DUMP_VCD_SHIM=true/false
AIE_DUMP_VCD_MEM=true/false
AIE_DUMP_VCD_STREAM_SWITCH=true/false
AIE_DUMP_VCD_CLK=true/false
```
For more details on the aiesimulator options see Simulating an AI Engine Graph Application in the _AI Engine Tools and Flows User
Guide_ (UG1076) and the AI Engine Simulator chapter.

AI Engine Event Trace in Hardware

To obtain trace data during hardware run, there must be routes dedicated to driving trace data from the AI Engine array to the PL or to
the DDR. For this reason, during the graph compilation phase, you need to specify the trace data during hardware run and the interface
to be used. See the following code for details.

```
v++ --c --mode aie --verbose --pl-freq=100 --workdir=./myWork \
--event-trace-port=gmio --event-trace=runtime \
--num-trace-streams=8 --xlopt=0 --include="./" \
--include="./src" --include="./src/kernels" --include="./data" \
./src/graph.cpp
```
```
For --event-trace=runtime: the only possibility here is runtime, indicating that signal selection will be decided at runtime.
For --event-trace-port=plio/gmio: selects GMIO and the NoC pathway instead of PLIO/PL pathway. PLIO uses PL logic,
which can induce timing closure difficulties.
For --num-trace-streams=8: up to 16 streams can be used within the AIE Engine array to drive the trace events to the
GMIO/PLIO.
```
For the profiling flow, you can perform event trace using either XSDB or XRT flow.
The metrics for the array are described below.

**Table: AI Engine Metrics**

```
Metric Name Description
```
```
functions Basic time line of function activity: events generated when kernel functions are being
invoked and returned
```
```
partial_stalls Three types of core stalls are being registered: stream stalls (no data at input or back-
pressure at output), cascade stalls and lock stalls.
```
```
all_stalls Same as partial_stalls with memory_stalls (memory conflict) added.
```

##### Displayed in the footer

```
Metric Name Description
```
```
all_dma Data transfers of all 4 Memory DMA channels (2xS2MM, 2xMM2S)
```
```
all_stalls_dma Core stalls and data transfers of all 4 DMA channels. All core stalls are grouped, no
differentiation on the type of stall.
```
```
all_stalls_s2mm Core stalls and data transfer of two S2MM channels^1
```
```
all_stalls_mm2s Core stalls and data transfer of two MM2S channels^1
```
```
s2mm_channels Data transfers and stalls of two S2MM channels
```
```
mm2s_channels Data transfers and stalls of two MM2S channels
```
```
s2mm_channels_stall Details of one S2MM channel.^2 In AI Engine-ML v2 based devices only
```
```
mm2s_channels_stall Details of one MM2S channel^2. In AI Engine-ML v2 based devices only
```
1. In AI Engine based devices, the stall events are concatenated into a group stall event.
2. Includes Buffer Descriptors, tasks, starvation, back-pressure and lock stalls.

**Table: Interface Tiles**

```
Metric Name Description
```
```
input_ports Data transfers of 4 stream input from the AI Engine Array
```
```
input_port_stalls Data transfers and stalls of 2 inputs from the AI Engine Array
```
```
input_port_details Details on one MM2S channel^1. For GMIOs only
```
```
output_port Data transfers of 4 stream output to the AI Engine Array
```
```
output_port_stalls Data transfers and stalls of 2 outputs to the AI Engine Array
```
```
output_port_details Details on one S2MM channel. Includes Buffer Descriptors, tasks, starvation, back-
pressure and lock stalls. For GMIOs only
```
```
input_output_ports Data transfers of 4 inputs or outputs of AI Engine Array
```
```
input_output_ports_stalls Data transfers and stalls of 2 inputs or output of the AI Engine Array
```
**Table: Memory Tiles (AI Engine-ML and AI Engine-ML v2)**

```
Metric Name Description
```
```
s2mm_channels Buffer Descriptor and Task events for two S2MM channels
```
```
s2mm_channels_stalls Details on one S2MM channels, adding lock stalls, back-pressure and stream
starvation.
```
```
mm2s_channels Buffer Descriptor and Task events for 2 MM2S channels
```
```
mm2s_channels_stalls Details on one MM2S channel, adding lock stalls, back-pressure and stream starvation.
```
```
memory_conflicts1 Memory conflict for data memory banks 0-7
```
```
memory_conflicts2 Memory conflicts for data memory bank 8-15
```
#### XSDB Flow

When running the application, the trace data is stored in DDR memory by the debugging and profiling IP. To capture and evaluate this
data, you must connect to the hardware device using xsdb. This command is typically used to program the device and debug bare-


##### Displayed in the footer

metal applications. Connect your system to the hardware platform or device over JTAG, launch the xsdb command in a command
shell, and run the following sequence of commands. In this sequence, the source
$::env(XILINX_VITIS)/scripts/vitis/util/aie_trace.tcl command sources the Tcl trace command to set up the xsdb
environment.

```
xsdb% connect
xsdb% ta
xsdb% ta 1
xsdb% source $::env(XILINX_VITIS)/scripts/vitis/util/aie_trace.tcl
xsdb% aietrace start -graphs mygraph -work-dir ./Work -link-summary $PROJECT/xsa.link_summary -base-
address 0x900000000 -depth 0x800000 -tile-based-aie-tile-metrics "all:functions; {4,1}:
{6,2}:all_stalls"
```
```
# Execute the PS host application (.elf) on Linux
## After the application completes processing.
xsdb% aietrace stop
```
After the hardware events are captured on the SD card, offload them on your computer and launch the Vitis Unified IDE to import and
analyze data:

```
vitis -a aie_trace_profile.run_summary
```
For more details on this flow, see the chapters on Event Tracing in Hardware and XSDB flow in the _AI Engine Tools and Flows User
Guide_ (UG1076).

## XRT Flow

Within the XRT flow, the selection of trace events is performed in the xrt.ini file in the SDCard. An example of such an xrt.ini file
is shown hereafter:

```
# Main switch to turn on aie trace
[Debug]
aie_trace = true
# Continuous trace knobs
[AIE_trace_settings]
reuse_buffer = true
periodic_offload = true
# Time to wait between trace reads
buffer_offload_interval_us = 100
# Total amount of device memory shared between trace streams
buffer_size = 16M
# granularity
graph_based_aie_tile_metrics = all:all:functions
```
For more details, see the chapters on Event Tracing in Hardware and XRT Flow in the _AI Engine Tools and Flows User Guide_
(UG1076).

# Debugging System Projects

The AMD Vitis™ unified software platform provides application-level debug features and techniques that allow the Application
component, AI Engine component, PL kernels, and the interactions between them to be debugged. However, debugging projects built
from the command line is a challenge because the various elements of the system, the compiled AI Engine graph application (libadf.a),
the device binary (.xclbin), and the top-level application (host.elf), must be gathered together and presented as a system.
The Vitis unified IDE provides an excellent framework for debugging these heterogeneous systems. There are many advantages to
working in the Debug view in the IDE. In fact, you are strongly recommended to debug your command-line driven projects in the IDE.
The process for doing this is broken down into two steps:

1. Import your command-line project into the IDE as described in Getting Started with Vitis in the _Vitis Unified Software Platform_
    _Documentation: Embedded Software Development_ (UG1400).


##### Displayed in the footer

2. Debug the system in the IDE as described in Debugging the System Project and AI Engine Components in the _Vitis Reference_
    _Guide_ (UG1702).

The Vitis tools provide application-level debug features which let the host code, the system project, and the interactions between them
be efficiently debugged in the Vitis unified IDE. These features and techniques are split between software debugging and hardware
debugging flows. The recommended debugging flow consists of three levels of debugging:

```
The AI Engine graph can be simulated and debugged on its own using GDB or the trace and profile tools available within the
aiesimulator (see Simulating an AI Engine Graph Application in the AI Engine Tools and Flows User Guide (UG1076)). The input
and output of the graph can be managed by external traffic generators using C++, Python or Verilog (see External Traffic
Generator in the AI Engine Tools and Flows User Guide (UG1076)). This is a first step where the host application is not debugged,
but the graph can be debugged using realistic data flows.
Debugging in Hardware Emulation in the Data Center Acceleration using Vitis (UG1700) to compile the PL kernels into RTL,
confirm the behavior of the generated logic, and evaluate the simulated performance of the hardware.
Debugging During Hardware Execution to implement the device binary and debug the application running on hardware using
Xilinx virtual cable (XVC) running over the PCIe® bus, or debugged using USB-JTAG cables for embedded processor platforms.
```
This three-tiered approach enables debugging the Application component and System project at different levels of abstraction. Each
provides specific insights into the design providing a comprehensive view of the system from software to hardware. All flows are
supported through the Vitis unified IDE using basic compile time and runtime setup options.

### Hardware Profile and Debug Methodology

Designs running on AMD Versal™ AI Engine devices can target the AI Engine, PL, and Arm® host. To ensure a design targeting such
multi-domain devices is functionally correct and meets the design performance specification, AMD recommends a five-stage profile and
debug methodology in hardware.
The stages are as follows:

1. Design Execution and System Metrics
2. System Profiling
3. PL Kernel Analysis
4. AI Engine Event Trace and Analysis
5. Host Application Debug

**Figure: Five Stages of Profile and Debug Methodology**


##### Displayed in the footer

The goal of each stage along with available tools and techniques are described below.

### Stage 1: Design Execution and System Metrics

The goal of the first stage is to determine if the design is functionally correct and can run successfully in hardware. You can also
analyze results, and proceed to the next stage for further debug and analysis.
The following figure shows the tasks and techniques available in this stage.

**Figure: Design Execution and System Metrics**


##### Displayed in the footer

✎

This stage can help you determine if the design and host application can run successfully in hardware. In this stage you can use APIs in
your host application to profile the design as it is running on hardware, and determine if the design meets throughput, latency and
bandwidth goals. In addition, you can troubleshoot AI Engine stalls and deadlocks using reports generated when running the design in
hardware.
The sections below list the techniques available in this stage.

#### Error Handling and Reporting in Host Application in Hardware

On Linux, XRT provides error reporting APIs. Use these APIs in your host application to catch and report these errors to get to the root
cause of the issue. For details on the XRT error reporting APIs, see AI Engine Error Reporting in the _AI Engine Tools and Flows User
Guide_ (UG1076). To examine error messages reported by the AI Engine array, enable and examine the dmesg logs. Details on AI
Engine array-specific error handling can be found in AI Engine Error Events in the _AI Engine Tools and Flows User Guide_ (UG1076).
_Next stage_ : Proceed to stage 5 if you determine that the host application needs to be debugged further.
**Note:** For errors flagged on the AI Engine, the error handling table provides guidance on next steps.

#### Analyzing Design Stalls in Hardware

If you encounter design stalls in hardware on Linux, track the status of the AI Engine and PL kernels in the design using the XRT xrt-smi
utility on Linux. For more information on how to use the utility to generate a report on the current design status and to visualize the
results in the Vitis IDE, see Analyzing AI Engine Status in Hardware in the _AI Engine Tools and Flows User Guide_ (UG1076).
You can also manually generate the report in XSDB and visualize the results in the Vitis IDE. More more details, see Generating AI
Engine Status in the _AI Engine Tools and Flows User Guide_ (UG1076). Some examples of deadlock visualizations in the Vitis IDE are
found in Analyzing the Automated Status Output in the _AI Engine Tools and Flows User Guide_ (UG1076).
_Next stage_ : Proceed to stage 2 if you determine that the design is stalled and further details are needed to get to the root cause of the
stall.

#### Reporting Design Throughput, Latency, Bandwidth in Hardware

You can also determine the AI Engine graph throughput, latency and bandwidth by profiling the graph inputs and outputs via APIs in the
host application. Careful consideration is needed on when profiling the API is started and stopped in the host. For details on how to use
the APIs in the host application, see Event Profile APIs for Graph Inputs and Outputs in the _AI Engine User Guide_ (UG1076).
_Next stage_ : Proceed to stage 2 if you determine that the design has sub-par throughput or latency, or the design does not meet the
performance goal. In stage 2, you can pinpoint the kernel or I/O that might be contributing to the performance drop.

### Stage 2: System Profiling

The goal of this stage is to profile the design and determine which domain (AI Engine, PL, NoC) is causing a throughput drop, which
causes the design to stall.
The following figure shows the tasks and techniques available in this stage.

**Figure: System Profiling**


##### Displayed in the footer

The section below lists the technique available in this stage.

#### Profiling AI Engine Core, Interface and Memory Module

You can profile the AI Engine Core, Interface, and Memory modules in XRT or XSDB flows. This is a non-intrusive feature which can be
enabled at runtime using the xrt.ini file or running scripts in XSDB. The feature uses performance counters available in the AI Engine
array to gather profile data. The amount and type of data gathered is limited by the number of performance counters available.

**Table: AI Engine Metrics**

```
AI Engine metrics
```
```
heat_map Profiles active, stall, vector instructions and cumulative instructions time. These metrics reveals
the efficiency of the AI Engine not only in terms of code efficiency (vector instructions) but also in
terms of interaction with memory and streams.
```
```
stalls Profiles memory, stream, lock and cascade stalls. These metrics allows a deeper analysis of the
reasons of the stalls detected with heat_map metric set.
```
```
execution Profiles vector, load and store instructions time. With these metrics you can determine the
efficiency of your kernel code.
```
```
floating-point Profiles all floating-point exceptions. If you are using floating-point arithmetic, these metrics
highlight the exceptions that occur in the code.
```
```
aie_trace Profiles AI Engine and memory module trace word count and stall count. This is useful to
determine if you have congestion in the trace stream when you use the event trace feature.
```
```
write_bandwidths Profiles stream write, cascade write and stalls time. This is an indicator of the efficiency of the
stream and cascade output. If there are many stalls, this indicates that the next kernel in the graph
cannot consume data quickly enough and this could impact the design throughput.
```

##### Displayed in the footer

```
AI Engine metrics
```
```
read_bandwidth Profiles stream read, Cascade read and stalls time. This is an indicator of the efficiency of the
stream and cascade input. If there are many stalls, this indicates that the previous kernel in the
graph cannot provide data quickly enough and this could impact the design throughput.
```
**Table: Memory Module Metrics**

```
Memory Module Metrics
```
```
conflicts Profiles memory conflicts and memory errors. Memory conflicts happen when two memory chunks
reside in the same memory bank and are accessed either by the same AI Engine (using the two
read ports) or by two different AI Engines. A potential solution is to constrain the locations of these
memories to different banks. In order to get more details about which bank is causing these
conflicts, you should analyze the events from an emulation-AI Engine simulation or perform event
trace in hardware.
```
```
dma_locks Profiles lock activities on both DMAs. The four DMA channels (2xS2MM and 2xMM2S) are driven
by Buffer Descriptors (BDs). The Cumulative DMA Activity is a count of the time taken due to
stalled lock acquire events on all channels. All these DMA events will help you understand why
some connections through the device are slower than expected.
```
```
dma_stalls_s2mm Profiles DMA stalls on the s2mm channels due to a lock acquisition conflict. A stalling s2mm DMA
indicates that there is a conflict when accessing the target memory. This can be due to another
s2mm or mm2s DMA accessing the same bank or a kernel performing a memory access leading
to a lock acquisition conflict.
```
```
dma_stalls_mm2s Profiles DMA stalls on the mm2s channels due to a lock acquisition conflict. A stalling mm2s DMA
indicates that there is a conflict when accessing the source memory. This can be due to another
s2mm or mm2s DMA accessing the same bank or a kernel performing a memory access leading
to a lock acquisition conflict.
```
```
write_bandwidths Profiles bandwidth used by the s2mm DMA. Allows you to evaluate if you achieve your bandwidth
goals.
```
```
read_bandwidths Profiles bandwidth used by the mm2s DMA. Allows you to evaluate if you achieve your bandwidth
goals.
```
**Table: Interface Tile Metrics**

```
Interface Tile Metrics
```
```
input_bandwidths Profiles input PLIO channel bandwidth in addition to stalls and idle time. If input bandwidth is too
low, this can be due to a high stall rate, which means that the AI Engine array does not consume
the samples at the right rate. Proceed to AI Engine event trace (stage 4). It can also be due to a
high idle rate which means that the PL side of the design does not produce samples at the right
rate. Proceed to PL Kernel analysis (stage 3).
```
```
output_bandwidths Profiles output PLIO channel bandwidth in addition to stalls and idle time. If output bandwidth is
too low, this can be due to a high idle rate, which means that the AI Engine array does not produce
the samples at the right rate. Proceed to AI Engine event trace (stage 4). It can also be due to a
high stall rate which means that the PL side of the design does not consume samples at the right
rate. Proceed to PL Kernel analysis (stage 3).
```
```
packets Profiles number of input and output packets
```
You can run the design multiple times, rebooting the board in between each run, with different parameters in the file xrt.ini. The Vitis IDE
allows you to consolidate the different xrt.run.summary files reports so that you have a global view on the various bandwidths, stalls and
idles at the interface level.
For details on how to enable profiling in hardware and interpreting the results, see Profiling the AI Engine in the _AI Engine Tools and
Flows User Guide_ (UG1076).
The profile results allow you to quickly identify the exact AI Engine, input stream or output stream involved in the design performance
drop.


##### Displayed in the footer

_Next Stage_ :

```
Proceed to stage 3 if you determine that a PL kernel is causing the performance drop. In stage 3, you can identify the exact PL
kernel(s) with the sub-par performance.
Proceed to stage 4 if you determine that an AI Engine kernel is causing the throughput drop.
```
### Stage 3: PL Kernel Analysis

The goal of this stage is to determine the exact PL kernel(s) causing a throughout drop.

**Figure: PL Kernel Analysis**

The sections below list the different techniques available in this stage.

#### Profiling Using PL Profile Monitors

You can insert PL profile monitors using the v++ link command. This allows you to monitor active, stalled cycles and bytes transferred
on specific PL-AI Engine interfaces. This can be enabled along with event tracing in the AI Engine to minimize the build time. This will
allow you to identify specific PL kernel(s) causing a performance drop. For more information on the option for adding PL profile
monitors, see --profile Options.

#### Replacing PL Kernels

You can replace each of the PL kernel that you suspect might be contributing a performance drop with non-throttling PL kernels. This
allows you to determine if the PL kernel is responsible for the performance drop.

#### Inserting ILA(s) to Monitor Specific AXI Interfaces

You can insert one or more ILAs to monitor specific PL AXI interfaces to help identify exactly where and when a throughput drop occurs.
It will also help you identify how frequently a throughput drop occurs. For details on the option to insert ILAs using the v++ command
line, see Enabling Kernels for Debugging with Chipscope.
_Next Stage_ : After you determine the cause of throughput drop and fix the issue, proceed to stage 1 to rerun the design.


##### Displayed in the footer

**!!**

###### ★

Debugging During Hardware Execution

**Important:** The following steps describe debugging from the command line. However, you are strongly encouraged to debug
command-line projects in the Vitis unified IDE by opening them in a workspace as described in Getting Started with Vitis in the _Vitis
Unified Software Platform Documentation: Embedded Software Development_ (UG1400) , and debugging them as described in
Debugging the System Project and AI Engine Components in the _Vitis Reference Guide_ (UG1702).
During hardware execution, the actual hardware platform is used to execute the kernels, and you can evaluate the performance of the
host program and kernels by running the application. However, debugging the hardware build requires additional logic to be
incorporated into the application. This will impact both the FPGA resources consumed by the kernel and the performance of the kernel
running in hardware. The debug configuration of the hardware build includes special ChipScope debug cores, such as Integrated Logic
Analyzer (ILA) and Virtual Input/Output (VIO) cores, and AXI performance monitors for debug purposes.

**Tip:** The additional logic required for debugging the hardware should be removed from the final production build.
The following figure shows the debug process for the hardware build, including debugging the host code using GDB using the Vivado
hardware manager with waveform analysis, kernel activity reports, and memory access analysis to identify and localize hardware
issues.

**Figure: Hardware Execution**

With the system hardware build configured for debugging, the host program running on the CPU and the Vitis kernels running on the
AMD device can be confirmed to be executing correctly on the actual hardware of the target platform. Some of the conditions that can
be identified and analyzed include the following:

```
System hangs caused by protocol violations:
These violations can take down the entire system.
These violations can cause the kernel to obtain invalid data or to hang.
It is hard to determine where or when these violations originate.
To debug this condition, you should use an ILA triggered off of the AXI protocol checker, which needs to be configured on the
Vitis target platform.
Problems with the hardware kernel:
Problems sometimes caused by the implementation: timing issues, race conditions, and bad design constraints.
Functional bugs that hardware emulation does not reveal.
Performance issues:
The frames per second processing that are not what you expected.
Examining data beats and pipelining.
Using an ILA with trigger sequencer, you can examine the burst size, pipelining, and data width to locate the bottleneck.
```
Checking the FPGA Board for Hardware Debug Support

Supporting hardware debugging requires the platform to support several IP components, most notably the Debug Bridge. Talk to your
platform designer to determine if these components are included in the target platform. If an AMD platform is used, debug availability
can be verified using the platforminfo utility to query the platform. Debug capabilities are listed under the chipscope_debug
objects.
For example, to query the a platform for hardware debug support, the following platforminfo command can be used:


##### Displayed in the footer

**!!**

###### ★

```
$ platforminfo --json="hardwarePlatform.extensions.chipscope_debug"
xilinx_u250_gen3x16_xdma_4_1_202210_1
{
"debug_networks": {
"user": {
"bar_number": "0",
"supports_jtag_fallback": "false",
"name": "User Debug Network",
"supports_microblaze_debug": "true",
"pcie_pf": "1",
"axi_baseaddr": "0x000001C00000",
"is_user_visible": "true"
},
"mgmt": {
"bar_number": "0",
"supports_jtag_fallback": "true",
"name": "Management Debug Network",
"supports_microblaze_debug": "true",
"pcie_pf": "0",
"axi_baseaddr": "0x01F60000",
"is_user_visible": "false"
}
}
}
```
The response shows that the target platform contains user and mgmt debug networks, supports debugging a MicroBlaze™ processor,
and also supports JTAG fallback for the Management Debug Network.

Enabling Kernels for Debugging with Chipscope

#### System ILA

The key to hardware debugging lies in instrumenting the kernels with the required debug logic. The following topic discusses the v++
linker options that can be used to list the available kernel ports, enable the System Integrated Logic Analyzer (ILA) core on selected
ports, and enable the AXI Protocol Checker debug core for checking for protocol violations.
The ILA core provides transaction-level visibility into an instance of a kernel running on hardware. AXI traffic of interest can also be
captured and viewed using the ILA core. The ILA provides custom event triggering on one or more signals to allow waveform capture at
system speeds. The waveforms can be analyzed in a viewer and used to debug hardware, finding protocol violations, or performance
issues. It can also be crucial for debugging difficult situation like application hangs.
Captured data can be accessed through the Xilinx virtual cable (XVC) using the Vivado tools. See the _Vivado Design Suite User Guide:
Programming and Debugging_ (UG908) for complete details.
The ILA core can be added to an existing RTL kernel to enable debugging features within that design, or it can be inserted automatically
by the v++ compiler during the linking stage. The v++ command provides the --debug option as described in --debug Options to
attach System ILA cores at the interfaces to the kernels for debugging and performance monitoring purposes.
**Important:** ILA debug cores require system resources, including logic and local memory to capture and store the signal data.
Therefore they provide excellent visibility into your kernel, but they can affect both performance and resource utilization.
The -–debug option to enable ILA IP core insertion has the following syntax:

```
--debug.chipscope <kernel_instance_name>[:<interface_name>]>
```
**Tip:** The <interface_name> is optional, and if not specified all ports on the kernel instance will be analyzed. You can use the --
debug.list_ports option to return the interface names on the kernel to use with --debug options.
In case of a flattened design or any design where there would be multiple debug bridges in master mode, the flow will not pick one to
stitch the debug cores, a constraint is needed to define the connectivity. For example in a Samsung Smart SSD U.2 flat shell, there is
no partitioning between the static and dynamic regions while generating the kernels with the debug (ILA) options enabled. It is required
to specify the connectivity of the kernel AXI ports that needs to be under debug to the user debug bridge in the dynamic region.
To specify the connectivity, you must provide the option below in the v++ command line:

```
--advanced.paramcompiler.userPostDebugProfileOverlayTcl=<path to post_dbg_profile_overlay.tcl >
```

##### Displayed in the footer

✎

###### ★

**!!**

Inside the post_dbg_profile_overlay.tcl, the file must call the XDC file with the connect debug core command and mention its processing
order.
For example, the contents in the post_dbg_profile_overlay.tcl file are given below.

```
read_xdc < path to the connect_debug_core.xdc file>
set_property used_in_implementation TRUE [get_files <path to the connect_debug_core.xdc file>]
set_property PROCESSING_ORDER EARLY [get_files <path to the connect_debug_core.xdc file>]]
```
In the connect_debug_core.xdc file, you have to specify the connect_debug_cores constraint.
For example:

```
connect_debug_cores -master [get_cells -hierarchical -filter {NAME =~ *debug_bridge_xsdbm/inst/xsdbm}]
-slaves [get_cells -hierarchical -filter {NAME =~ level0_i/ulp/system_ila_0}]
```
#### AXI Protocol Checker

The AXI Protocol Checker core monitors AXI interfaces. When attached to an interface, it actively checks for protocol violations and
provides an indication of which violation occurred. You can assign it for all kernel instances in the design, or for specific kernel instances
and ports.
The -–debug option to enable AXI Protocol Checker insertion has the following syntax:

```
--debug.protocol all
```
The protocol checker can be specified with the keyword all, or the <cu_name>:<interface_name>.
**Note:** The --debug.list_ports option can be specified to return the actual names of ports on the kernel to use with protocol
or chipscope.
An example flow you could use for adding ILA or protocol checkers to your design is outlined below:

1. Compile the kernel source files into an XO file, using the -g option to instrument the kernel for debug features:

```
v++ -c -g -k <kernel_name> --platform <platform> -o <kernel_xo_file>.xo <kernel_source_files>
```
2. After the kernel has been compiled into an XO file, use --debug.list_ports to cause the v++ compiler to print the list of valid
    kernel instances and port combinations for the kernel:

```
v++ -l -g --platform <platform> --connectivity.nk <kernel_name>:<kernel instances>:<kernel_nameN>
--debug.list_ports <kernel_xo_file>.xo
```
3. Add the ILA or AXI debug cores on the desired ports by replacing list_ports with the appropriate --debug.chipscope or --
    debug.protocol command syntax:

```
v++ -l -g --platform <platform> --connectivity.nk <kernel_name>:<kernel_instances>:<kernel_nameN>
--debug.chipscope <kernel_instances>:<interface_name> <kernel_xo_file>.xo
```
**Tip:** The --debug option can be specified multiple times in a single v++ command line, or configuration file to specify multiple
kernel instances and interfaces.
When the design is built, you can debug the design using the Vivado hardware manager as described in Debugging with ChipScope.

Adding Debug IP to RTL Kernels

**Important:** This debug technique requires familiarity with the Vivado Design Suite, and RTL design.
You can also enable debugging in RTL kernels by manually adding ChipScope debug cores like the ILA and VIO in your RTL kernel
code before packaging it for use in the Vitis development flow. From within the Vivado Design Suite, edit the RTL kernel code to
manually instantiate an ILA debug core, or VIO IP from the AMD IP catalog, similar to using any other IP in Vivado IDE. Refer to the
HDL Instantiation flow in the _Vivado Design Suite User Guide: Programming and Debugging_ (UG908) to learn more about adding
debug cores to your design.
The best time to add debug cores to your RTL kernel is when you create it. However, debug cores consume device resources and can
affect performance, so it is good practice to make one kernel for debug and a second kernel for production use. The


##### Displayed in the footer

rtl_vadd_hw_debug of the RTL Kernels examples on GitHub shows an ILA debug core instantiated into the RTL kernel source file.
The ILA monitors the output of the combinatorial adder as specified in the src/hdl/krnl_vadd_rtl_int.sv file.

```
// ILA monitoring combinatorial adder
ila_0 i_ila_0 (
.clk(ap_clk), // input wire clk
.probe0(areset), // input wire [0:0] probe0
.probe1(rd_fifo_tvalid_n), // input wire [0:0] probe1
.probe2(rd_fifo_tready), // input wire [0:0] probe2
.probe3(rd_fifo_tdata), // input wire [63:0] probe3
.probe4(adder_tvalid), // input wire [0:0] probe4
.probe5(adder_tready_n), // input wire [0:0] probe5
.probe6(adder_tdata) // input wire [31:0] probe6
);
```
You can also add the ILA debug core using a Tcl script from within an open Vivado project, using the Netlist Insertion flow described in
_Vivado Design Suite User Guide: Programming and Debugging_ (UG908), as shown in the following Tcl script example:

```
create_ip -name ila -vendor xilinx.com -library ip -version 6.2 -module_name ila_0
set_property -dict [list CONFIG.C_PROBE6_WIDTH {32} CONFIG.C_PROBE3_WIDTH {64} \
CONFIG.C_NUM_OF_PROBES {7} CONFIG.C_EN_STRG_QUAL {1} CONFIG.C_INPUT_PIPE_STAGES {2} \
CONFIG.C_ADV_TRIGGER {true} CONFIG.ALL_PROBE_SAME_MU_CNT {4} CONFIG.C_PROBE6_MU_CNT {4} \
CONFIG.C_PROBE5_MU_CNT {4} CONFIG.C_PROBE4_MU_CNT {4} CONFIG.C_PROBE3_MU_CNT {4} \
CONFIG.C_PROBE2_MU_CNT {4} CONFIG.C_PROBE1_MU_CNT {4} CONFIG.C_PROBE0_MU_CNT {4}] [get_ips ila_0]
```
After the RTL kernel has been instrumented for debug with the appropriate debug cores, you can analyze the hardware in the Vivado
hardware manager as described in Debugging with ChipScope.

Enabling ILA Triggers for Hardware Debug

To perform hardware debug of both the host program and the kernel code running on the target platform, the application host code must
be modified to let you set up the ILA trigger conditions _after_ the kernel has been programmed into the device, but _before_ starting the
kernel.

Pausing the Host Application Using GDB

If you are running GDB to debug the host program at the same time as performing hardware debug on the kernels, you can also pause
the host program as needed by inserting a breakpoint at the appropriate line of code. Instead of making changes to the host program to
pause the application as needed, you can set a breakpoint prior to the kernel execution in the host code. When the breakpoint is
reached, you can set up the debug ILA triggers in Vivado hardware manager, arm the trigger, and then resume the host program in
GDB.

Debugging with ChipScope

You can use the ChipScope debugging environment and the Vivado hardware manager to help you debug your host application and
kernels quickly and more effectively. These tools enable a wide range of capabilities from logic to system-level debug while your kernel
is running in hardware. To achieve this, at least one of the following must be true:

```
Your Vitis application project has been designed with debug cores, using the --debug.xxx compiler switch, as described in
Enabling Kernels for Debugging with Chipscope.
The RTL kernels used in your project need to be instantiated with debug cores (as described in Adding Debug IP to RTL Kernels).
```
Running XVC and HW Servers

The following steps are required to run the Xilinx virtual cable (XVC) and HW servers, host applications, and also trigger and arm the
debug cores in the Vivado hardware manager.

1. Add debug IP to the kernel as discussed in Enabling Kernels for Debugging with Chipscope.
2. Modify the host program to pause at the appropriate point as described in Enabling ILA Triggers for Hardware Debug.
3. Set up the environment for hardware debug, using an automated script described in Automated Setup for Hardware Debug, or
    manually as described in Manual Setup for Hardware Debug.


##### Displayed in the footer

###### ★

###### ★

4. Run the hardware debug flow using the following process:
    a. Launch the required XVC and the hw_server of the Vivado hardware manager.
    b. Run the host program and pause at the appropriate point to enable setup of the ILA triggers.
       c. Open the Vivado hardware manager and connect to the XVC server.
    d. Set up ILA trigger conditions for the design.
    e. Continue execution of the host program.
       f. Inspect kernel activity in the Vivado hardware manager.
    g. Rerun iteratively from step b (above) as required.

Automated Setup for Hardware Debug

1. Set up your Vitis core development kit as described in Setting Up the Vitis Environment.
2. Use the debug_hw script to launch the xvc_pcie and hw_server apps as follows:

```
debug_hw --xvc_pcie /dev/xfpga/xvc_pub.<driver_id> --hw_server
```
```
The debug_hw script returns the following:
```
```
launching xvc_pcie...
xvc_pcie -d /dev/xfpga/xvc_pub.<driver_id> -s TCP::10200
launching hw_server...
hw_server -sTCP::3121
```
```
Tip: The /dev/xfpga/xvc_pub.<driver_id> driver character path is defined on your machine, and can be found by examining the
/dev folder.
```
3. Modify the host code to include a pause statement _after_ the kernel has been created/downloaded and _before_ the kernel execution
    is started, as described in Enabling ILA Triggers for Hardware Debug.
4. Run your modified host program.
5. Launch Vivado Design Suite using the debug_hw script:

```
debug_hw --vivado --host <host_name> --ltx_file
./_x/link/vivado/vpl/prj/prj.runs/impl_1/debug_nets.ltx
```
```
Tip: The <host_name> is the name of your system.
As an example, the command window displays the following results:
```
```
launching vivado... ['vivado', '-source', 'vitis_hw_debug.tcl', '-tclargs',
'/tmp/project_1/project_1.xpr', 'workspace/vadd_test/System/pfm_top_wrapper.ltx',
'host_name', '10200', '3121']
```
```
****** Vivado v2019.2 (64-bit)
**** SW Build 2245749 on Date Time
**** IP Build 2245576 on Date Time
** Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
```
```
start_gui
```

##### Displayed in the footer

###### ★

###### ★

6. In Vivado Design Suite, run the ILA trigger.
7. Press Enter to continue running the host program.
8. In the Vivado hardware manager, see the interface transactions on the kernel instance slave control interface in the Waveform
    view.

Manual Setup for Hardware Debug

**Tip:** The following steps can be used when setting up Nimbix and other cloud platforms.
There are a few steps required to start the debug servers prior to debugging the design in the Vivado hardware manager.

1. Set up your Vitis core development kit as described in Setting Up the Vitis Environment.
2. Launch the xvc_pcie server. The file name passed to xvc_pcie must match the character driver file installed with the kernel
    device driver, where <driver_id> can be found by examining the /dev folder.

```
>xvc_pcie -d /dev/xfpga/xvc_pub.<device_id>
```
```
Tip: The xvc_pcie server has many useful command line options. You can issue xvc_pcie -help to obtain the full list of
available options.
```
3. Start the hw_server on port 3121 , and connect to the XVC server on port 10201 using the following command:

```
>hw_server -e "set auto-open-servers xilinx-xvc:localhost:10201" -e "set always-open-jtag 1"
```
4. Launch Vivado Design Suite and open the hardware manager:

```
vivado
```
Debugging Designs Using Vivado Hardware Manager

Traditionally, a physical JTAG connection is used to perform hardware debug for AMD devices with the Vivado hardware manager. The
Vitis unified software platforms also makes use of the Xilinx virtual cable (XVC) for hardware debugging on remote accelerator cards. To
take advantage of this capability, the Vitis debugger uses the XVC server, an implementation of the XVC protocol that allows the Vivado
hardware manager to connect to a local or remote target device for debug, using the standard AMD debug cores like the ILA or the VIO
IP.
The Vivado hardware manager, from the Vivado Design Suite or Vivado debug feature, can be running on the target instance or it can
be running remotely on a different host. The TCP port on which the XVC server is listening must be accessible to the host running
Vivado hardware manager. To connect the Vivado hardware manager to XVC server on the target, the following steps should be
followed on the machine hosting the Vivado tools:

1. Launch the Vivado debug feature, or the full Vivado Design Suite.


##### Displayed in the footer

2. Select Open Hardware Manager from the Tasks menu, as shown in the following figure.
3. Connect to the Vivado tools hw_server, specifying a local or remote connection, and the Host name and Port, as shown below.


##### Displayed in the footer

###### ★

###### ★

4. Connect to the target instance Virtual JTAG XVC server.
5. Select the debug_bridge instance from the Hardware window in the Vivado hardware manager.
    Specify the probes file (.ltx) for your design adding it to the Probes > File entry in the Hardware Device Properties window. Adding
    the probes file refreshes the hardware device, and Hardware window should now show the debug cores in your design.

```
Tip: If the kernel has debug cores as specified in Enabling Kernels for Debugging with Chipscope, the probes file (.ltx) is
written out during the implementation of the kernel by the Vivado tool.
```
6. The Vivado hardware manager can now be used to debug the kernels running on the Vitis software platform. Arm the ILA cores in
    your kernels and run your host application.

**Tip:** Refer to the _Vivado Design Suite User Guide: Programming and Debugging_ (UG908) for more information on working with the
Vivado hardware manager to debug the design.

Utilities for Hardware Debugging

In some cases, the normal Vitis IDE and command line debug features are limited in their ability to isolate an issue. This is especially
true when the software or hardware appears not to make any progress (hangs). These kinds of system issues are best analyzed with
the help of the utilities mentioned in this section.

Using the Linux dmesg Utility

Well-designed kernels and modules report issues through the kernel ring buffer. This is also true for Vitis technology modules that allow
you to debug the interaction with the accelerator board on the lowest Linux level.
The dmesg utility is a Linux tool that lets you read the kernel ring buffer. The kernel ring buffer holds kernel information messages in a
circular buffer. A circular buffer of fixed size is used to limit the resource requirements by overwriting the oldest entry with the next
incoming message.


##### Displayed in the footer

###### ★ Tip: In most cases, it is sufficient to work with the less verbose xrt-smi feature to localize an issue. Refer to Using the xrt-smi

Utility for more information on using this tool for debug.
In the Vitis technology, the xocl module and xclmgmt driver modules write informational messages to the ring buffer. Thus, for an
application hang, crash, or any unexpected behavior (like being unable to program the bitstream, etc.), the dmesg tool should be used
to check the ring buffer.
The following image shows the layers of the software platform associated with the target platform.

**Figure: Software Platform Layers**

To review messages from the Linux tool, you should first clear the ring buffer:

```
sudo dmesg -c
```
This flushes all messages from the ring buffer and makes it easier to spot messages from the xocl and xclmgmt. After that, start your
application and run dmesg in another terminal.

```
sudo dmesg
```
The dmesg utility prints a record shown in the following example:

**Figure: dmesg Utility Example**

In the example shown above, the AXI Firewall 2 has tripped, which is better examined using the xrt-smi utility.

Using the xrt-smi Utility

The Xilinx board utility (xrt-smi) is a powerful standalone command line utility that can be used to debug lower level
hardware/software interaction issues. A full description of this utility can be found in xrt-smi Utility in the _Vitis Reference Guide_
(UG1702).
With respect to debugging, the following xrt-smi options are of special interest:

**examine**
Provides an overall status of a card including information on the kernels in card memory.

**program**
Downloads a binary (xclbin) to the programmable region of the AMD device.

**xrt-smi exmaine -r debug-ip-status -d <BDF>**
Extracts the status of the Performance Monitors (aim and asm) and the Lightweight AXI Protocol Checkers (lapc).

### Stage 4: AI Engine Event Trace and Analysis

The goal of this stage is to determine the AI Engine kernel or graph construct causing design performance drop or stall, or causing a
deadlock.
The following figure shows the tasks and techniques available in this stage.


##### Displayed in the footer

**Figure: AI Engine Event Trace and Analysis**

The sections below list the different debug techniques available in this design stage.

#### Running and Analyzing Runtime Trace Data Using AI Engine Event Trace Flow

The AI Engine Event Trace feature offers a comprehensive view of design trace data when running the design in hardware, which is a
three step process to:

1. Compile the design with event trace enabled and other event trace related options.
2. Run the design in hardware and collect event trace data.
3. Open the trace summary file in the Vitis IDE, which provides a waveform view of the trace data collected above.

Event trace data lets you identify the AI Engine kernel that contributes to a stall, deadlock or throughput drop, and also view events prior
to a stall/throughput drop in addition to other detailed trace information. Details on the event trace feature can be found in AI Engine
Event Trace in Hardware.

**Figure: Event Tracing**


##### Displayed in the footer

For detailed resolution to specific techniques encountered running event trace in hardware, see Troubleshooting Event Trace in
Hardware in the _AI Engine Tools and Flows User Guide_ (UG1076). The feature is limited by the event trace counters, streams, DDR
memory and design resources available for event trace in the device.

#### Profiling Intra-Kernel Performance

You can also profile code blocks inside a specific kernel using aie::tile::cycles() API.
To get this value in hardware, you can write this value to memory or to an output stream. An example of writing to output stream is
shown below. This stream of data can then be examined in the host application to read back the profile data.

```
// get the current tile
aie::tile tile=aie::tile::current();
unsigned long long time=tile.cycles(); //cycle counter of [SS1] the AI Engine tile
writeincr(out,time);
{//loop to be profiled
}
time=tile.cycles();//cycle counter of the AI Engine tile
writeincr(out,time);
```
This is a very intrusive method of profiling kernel code. AMD recommends that you use this method to simulate the graph with the AI
Engine Simulator. In addition, trace and profile data in simulation can also be used for this purpose.
For details on the aie::tile::cycles() API, see _AI Engine Kernel and Graph Programming Guide_ (UG1079).

#### Vitis IDE Debugger

You can also use the Vitis IDE debugger to debug kernel source code. Details on the Vitis Debugger can be found in Debugging the
System Project and AI Engine Components in the _Vitis Reference Guide_ (UG1702).
_Next Stage_ : After you determine the cause of throughput drop and fix the issue, proceed to stage 1 to rerun the design.

### Stage 5: Host Application Debug

The goal of this stage is to debug the host application, and address application exceptions or crashes, if any exist.

**Figure: Host Application Debug**


##### Displayed in the footer

###### ★

The sections below list the different techniques available in this stage.

#### Vitis IDE Debugger

You can use the Vitis IDE debugger to debug host application source. Details on the Vitis Debugger can be found in Debugging the
System Project and AI Engine Components in the _Vitis Reference Guide_ (UG1702).

#### Printf

You can also use printf() statements in your host code to help debug the host application.
_Next Stage_ : After you determine the cause of the failure and fix the issue, you can recompile the host application, and proceed to stage
1.

Techniques for Debugging Application Hangs

This section discusses debugging issues related to the interaction of the host code and the kernels. Problems with these interactions
manifest as issues such as machine hangs or application hangs. Although the GDB debug environment might help with isolating the
errors in some cases (xprint), such as hangs associated with specific kernels, these issues are best debugged using the dmesg and
xrt-smi commands as shown here.
If the process of hardware debugging does not resolve the problem, it is necessary to perform hardware debugging using the
ChipScope feature.

AXI Firewall Trips

The AXI firewall should prevent host hangs. This is why the AXI Protocol Firewall IP is included in all production Vitis platforms. When
the firewall trips, one of the first checks to perform is confirming if the host code and kernels are set up to use the same memory banks.
The following steps detail how to perform this check.

1. Use xrt-smi to program the FPGA:

```
xrt-smi program -p <xclbin>
```
```
Tip: Refer to xrt-smi Utility in the Vitis Reference Guide (UG1702) for more information on xrt-smi.
```
2. Run the xrt-smi examine option to check memory topology:

```
xrt-smi examine -r memory -d <bdf>
```

##### Displayed in the footer

```
✎
```
```
✎
```
```
In the following example, there are no kernels associated with memory banks:
```
3. If the host code expects any DDR banks/PLRAMs to be used, this report should indicate an issue. In this case, it is necessary to
    check kernel and host code expectations.

Kernel Hangs Due to AXI Violations

It is possible for the kernels to hang due to bad AXI transactions between the kernels and the memory controller. To debug these
issues, it is required to instrument the kernels.

1. The Vitis core development kit provides two options for instrumentation to be applied during v++ linking (--link). Both of these
    options add hardware to your implementation, and based on resource usage it can be necessary to limit instrumentation.
       a. Add Lightweight AXI Protocol Checkers (lapc). These protocol checkers are added using the -–debug.protocol option,
          as explained in --debug Options. The following syntax is used:

```
--debug.protocol <kernel_instance_name>:<interface_name>
```
```
In general, the <interface_name> is optional. If not specified, all ports on the CU are expected to be analyzed. The --
debug.protocol option is used to define the protocol checkers to be inserted. This option can accept a special keyword,
all, for <kernel_instance_name> and/or <interface_name>.
Note: Multiple --debug.xxx options can be specified in a single command line, or configuration file.
b. Adding Performance Monitors (am, aim, asm) enables the listing of detailed communication statistics (counters). Although
this is most useful for performance analysis, it provides insight during debugging on pending port activities. The Performance
Monitors are added using the --profile option as described in --profile Options. The basic syntax for the --profile
option is:
```
```
--profile.data <krnl_name>|all:<kernel_instance_name>|all:<intrfc_name>|all:<counters>|all
```
```
Three fields are required to determine the specific interface to attach the performance monitor to. However, if resource
consumption is not an issue, the keyword all lets you apply the monitoring to all existing kernels, their instances, and
interfaces with a single option. Otherwise, you can specify the kernel_name, kernel_instance_name, and
interface_name explicitly to limit instrumentation.
The last option, <counters>|all, allows you to restrict the information gathering to counters for large designs, while all
(default) includes the collection of actual trace information.
Note: Multiple --profile options can be specified in a single command line, or configuration file.
```
```
[profile]
dataernel1:cu1:m_axi_gmem0
dataernel1:cu1:m_axi_gmem1
dataernel2:cu2:m_axi_gmem
```
2. When the application is rebuilt, rerun the host application using the xclbin with the added AIM IP and LAPC IP.
3. When the application hangs, you can use xrt-smi examine to check for any errors or anomalies.
4. Check the AIM output:


##### Displayed in the footer

###### ★

###### ★

###### ★

###### ★

```
Run the following command a couple of times to check if any counters are moving. If they are moving then the kernels are
active.
```
```
xrt-smi examine -d <bdf> -r debug-ip-status -e aim
```
```
Tip: Testing AIM output is also supported through GDB debugging using the command extension xstatus aim.
If the counters are stagnant, the outstanding counts greater than zero might mean some AXI transactions are hung.
```
5. Check the LAPC output:
    Run the following command to check if there are any AXI violations.

```
xrt-smi examine -d <bdf> -r debug-ip-status -e lapc
```
```
Tip: Testing LAPC output is also supported through GDB debugging using the command extension xstatus lapc.
If there are any AXI violations, it implies that there are issues in the kernel implementation.
```
Host Application Hangs When Accessing Memory

Application hangs can also be caused by incomplete DMA transfers initiated from the host code. This does not necessarily mean that
the host code is wrong; it might also be that the kernels have issued illegal transactions and locked up the AXI.

1. If the platform has an AXI firewall, such as in the Vitis target platforms, it is likely to trip. The driver issues a SIGBUS error, kills the
    application, and resets the device. You can check this by running the following command:

```
xrt-smi examine -d <bdf> -r firewall
```
```
The following figure shows such an error in the firewall status:
```
```
Firewall Last Error Status:
0: 0x0 (GOOD)
1: 0x0 (GOOD)
2: 0x80000 (RECS_WRITE_TO_BVALID_MAX_WAIT).
Error occurred on Tue 2017-12-19 11:39:13 PST
```
```
Xclbin ID: 0x5a39da87
```
```
Tip: If the firewall has not tripped, the Linux tool, dmesg, can provide additional insight.
```
2. When you know that the firewall has tripped, it is important to determine the cause of the DMA timeout. The issue could be an
    illegal DMA transfer, or kernel misbehavior. However, a side effect of the AXI firewall tripping is that the health check functionality
    in the driver resets the board after killing the application; any information on the device that might help with debugging the root
    cause is lost. To debug this issue, disable the health check thread in the xclmgmt kernel module to capture the error. This uses
    common Unix kernel tools in the following sequence:
       a. sudo modinfo xclmgmt: This command lists the current configuration of the module and indicates if the health_check
          parameter is ON or OFF. It also returns the path to the xclmgmt module.
       b. sudo rmmod xclmgmt: This removes and disables the xclmgmt kernel module.
          c. sudo insmod <path to module>/xclmgmt.ko health_check=0: This re-installs the xclmgmt kernel module with
             the health check disabled.

```
Tip: The path to this module is reported in the output of the call to modinfo.
```
3. With the health check disabled, rerun the application. You can use the kernel instrumentation to isolate this issue as previously
    described.

Typical Errors Leading to Application Hangs

The user errors that typically create application hangs are listed below:


##### Displayed in the footer

```
Read-before-write in 5.0+ target platforms causes a Memory Interface Generator error correction code (MIG ECC) error. This is
typically a user error. For example, this error might occur when a kernel is expected to write 4 KB of data in DDR, but it produces
only 1 KB of data, and then try to transfer the full 4 KB of data to the host. It can also happen if you supply a 1 KB buffer to a
kernel, but the kernel tries to read 4 KB of data.
An ECC read-before-write error also occurs if no data has been written to a memory location as the last bitstream download which
results in MIG initialization, but a read request is made for that same memory location. ECC errors stall the affected MIG because
kernels are usually not able to handle this error. This can manifest in two different ways:
```
1. The kernel instance might hang or stall because it cannot handle this error while reading or writing to or from the affected
    MIG. The xrt-smi query shows that the kernel instance is stuck in a BUSY state and is not making progress.
2. The AXI Firewall might trip if a PCIe® DMA request is made to the affected MIG, because the DMA engine is unable to
    complete the request. AXI Firewall trips result in the Linux kernel driver killing all processes which have opened the device
    node with the SIGBUS signal. The xrt-smi query shows if an AXI Firewall has indeed tripped and includes a timestamp.
If the above hang does not occur, the host code might not read back the correct data. This incorrect data is typically 0s and is
located in the last part of the data. It is important to review the host code carefully. One common example is compression, where
the size of the compressed data is not known up front, and an application might try to migrate more data to the host than was
produced by the kernel.

Defensive Programming

The Vitis compiler is capable of creating very efficient implementations. In some cases, however, implementation issues can occur. One
such case is if a write request is emitted before there is enough data available in the process to complete the write transaction. This can
cause deadlock conditions when multiple concurrent kernels are affected by this issue and the write request of a kernel depends on the
input read being completed.
To avoid these situations, a conservative mode is available on the adapter. In principle, it delays the write request until it has all of the
data necessary to complete the write. This mode is enabled during compilation by applying the following --advanced.param option to
the v++ compiler:

```
--advanced.param:compiler.axiDeadLockFree=yes
```
Because enabling this mode can impact performance, you might prefer to use this as a defensive programming technique where this
option is inserted during development and testing and then removed during optimization. You might also want to add this option when
the accelerator hangs repeatedly.

Hardware Debug for Embedded Processors

For hardware builds the setup involves the following steps:

1. Copy the contents of the <project>/Hardware/sd_card/sd_card folder to a physical SD card. This creates a bootable medium for
    your target platform.
2. Insert the SD card into the card reader of your embedded processor platform.
3. Change the boot-mode settings of the platform to SD boot mode, and power up the board.
4. After the device is booted, enter the mount command at the command prompt to get a list of mount points. SD cards are typically
    mounted as /run/media/mmcblk*
5. Execute the following commands, for example:

```
cd /run/media/mmcblk0p1
source init.sh
cat /etc/xocl.txt
```
```
The cat command will display the platform name xilinx_vck190_base_202420_1 to let you confirm it is the same as your
specified platform and that your setup is correct.
```
6. Run ifconfig to get the IP address of the target card. You will use the IP address to set up a TCF agent connection in the Vitis
    unified IDE to connect to the assigned IP address of the embedded processor platform.
7. Create a target connection to the remote accelerator card. Use the Vitis > Target Connections menu command to open the Target
    Connections dialog box.
8. Right-click the Linux TCF Agent and select the New Target command to open the New Target Connection dialog box.
9. Specify the Target Name, enable the Set as default target check box, and specify the Host IP address of the accelerator card that
    you obtained in an earlier step.


##### Displayed in the footer

10. Click OK to close and continue.
    11. In the Flow Navigator view, click the Open Settings command to open the Launch Configuration editor to create a new launch
       configuration for the hardware design

```
Set the following fields on the Main tab of the dialog box:
Name
Specifies a name for your Hardware debug configuration.
Target Connection
Select a Linux TCF agent as previously configured.
Host Executable
Specify the location of the software application to drive the hardware.
Cmd Line Args
Specify any needed command line arguments for the host application, such as the .xclbin file to load.
Work Directory
Specifies the location where the system is run and output files will be written.
XRT Config File
Specifies the xrt.ini file to add to the hardware run as described in Enabling Profiling in Your Application.
Stop at Main
Puts a breakpoint in the host application to stop at the entry to the main() function to enable debug operations.
Stop At Program Entry
Places a breakpoint at the entry to the hardware program to enable debug operations.
```
12. Select Debug from the Flow Navigator to open the Debug view as described in Debug View in the _Vitis Reference Guide_
    (UG1702).
    This opens the Debug view in the Vitis unified IDE, and connects to the PS application on your hardware platform. The application
    automatically breaks at the main() function to let you set up and configure the debug environment.


