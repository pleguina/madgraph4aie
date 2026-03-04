## Embedded Design

## Development Using Vitis

## User Guide (UG1701)


###### Building and Running the System

```
Integrating AIE and PL Components
Managing Vivado Synthesis, Implementation, and Timing Closure
Integrating the System
Deploying and Running the System
Incremental Design Management
```

###### ✎

# Building and Running the

# System

The AMD Vitis™ tool simplifies hardware design and integration with a
software-like compilation and linking flow, integrating the four domains of
the AMD Versal™ device: the AI Engine array, the programmable logic
(PL) region, the network-on-chip (NoC) and the processing system (PS).
The Vitis linker flow lets you integrate your compiled AI Engine design
graphs (libadf.a) with additional kernels implemented in the PL region of
the device, including HLS and RTL kernels, and link them for use on a
target platform. The Vitis linker provides abstract directives for accessing
system memory, CPU control, and streaming I/O, so it is often possible to
develop AI Engine graphs and kernels on a standard development
platform and quickly re-target the AI Engine code to a custom platform
developed for your specific application. You can control AI Engine graphs
and PL kernels from code running on an embedded Arm® processor on
the Versal device or from an external CPU.
The following figure shows the high-level steps required to use the Vitis
tools flow to integrate your application. The command-line process to run
this flow is described here.

**Note:** You can also use this flow from within the Vitis IDE as explained
in Launching Vitis Unified IDE in the _Vitis Reference Guide_ (UG1702).

**Figure: Vitis Integrated Flow**


###### ✎

###### !!

**Note:** Running the application is either on hardware or in emulator.
**Important:** Using Vitis tools and AI Engine tools require the setup
described in Setting Up the Vitis Environment.

The following steps can be adapted to any Vitis project targeting a Versal
device.


###### ✎

1. AMD provides Pre-built Base Platforms for select devices and
    recommends using the Vitis Integrated Flow. Pre-built embedded
    base platforms installed with the Vitis installer can be targeted for
    the design flow or a Custom Platform created can be targeted for the
    design flow.
2. As described in Compiling an AI Engine Graph Application in the _AI_
    _Engine Tools and Flows User Guide_ (UG1076), the first step is to
    create and compile the AI Engine graph into a libadf.a file using the
    AI Engine compiler. You can iterate between the AI Engine compiler
    and the AI Engine simulator to develop the graph until you are ready
    to proceed.
3. PL Kernel Compilation: PL kernels are compiled for implementation in
    the PL region of the target platform using the v++ --mode hls
    command, see HLS Kernel Development. In addition to Vitis
    compilation, you can use Vivado to package RTL modules as kernels
    in the compiled .xo format as described in Packaging the RTL Code
    as a Vitis XO. See v++ Mode HLS in the _Vitis Reference Guide_
    (UG1702).
4. Linking the System: Link the compiled AI Engine graph with the HLS
    kernels and RTL kernels onto a target platform. The process creates
    an XSA file to encapsulate the implemented hardware system to
    create boot and loadable images.
       **Note:** During linking, a NoC design rule check is performed. See
    Validate NoC DRCs in the _Versal Adaptive SoC Hardware, IP, and_
    _Platform Development Methodology Guide_ (UG1387) for more
    details.
5. PS application compilation: Optionally compile a host application to
    run on the Cortex®-A72 core processor using the GNU Arm cross-
    compiler to create an ELF file. The host program interacts with the AI
    Engine kernels and kernels in the PL region. This compilation step is
    optional because there are several ways to deploy and interact with
    the AI Engine kernels, and the host program running on the PS is one
    way.
6. Packaging for Vitis Flow: Use the v++ --package process to gather
    the required files to configure and boot the system, to load and run
    the application, including the AI Engine graph and PL kernels. The
    packager can also be invoked to build the necessary package to run
    emulation and debug, or run your application on hardware.


###### ✎

### Integrating AIE and PL Components

Vitis components (AIE graphs and PL kernels) are assembled together
and integrated with an extensible platform using the Vitis v++ linker.
The following sections provide details of the linking process.

###### Linking the System

The following figure shows two paths through the linking process,
starting with the v++ --link command. The first path is the Vitis
Integrated Flow, where the v++ command links the elements of the
system, automatically launches the Vivado tools for implementation of
the design, and outputs a .xsa file. The second path is the Vitis Export to
Vivado flow, where the v++ command links the elements of the system
and outputs a .vma file for you to use in the Vivado tools for synthesis,
implementation, and timing closure. These two paths are explained
below.

**Figure: Linking the System Design**

**Note:** The Export to Vivado flow is currently only supported for custom
Versal platforms.

At the design level, the v++ link command operates within a Vitis
managed region as a hierarchy in the extensible hardware platform block


##### ★

design. Based on source input files, the v++ linker instantiates user-
defined PL kernels, configures platform IP such as AI Engine, NoC, and
soft interconnects, adds required design IP for AXI buses, clock domain
crossing, data width conversion, and FIFO buffering, adds networks for
hardware debugging, trace, and clocking, and creates connections
between IP within the Vitis managed region.

After Linking

After the linking step is complete, any reports generated during this
process are collected into the <kernel_name>.link_summary. This
collection of reports can be viewed by opening the link_summary in the
Analysis view of Vitis analyzer, and includes a Summary report, System
Estimate providing timing and resources estimates andSystem Guidance
offering suggestions for improving linking and the performance of the
system. Refer to Working with the Analysis View (Vitis Analyzer) in the
_Vitis Reference Guide_ (UG1702) for additional information.

**Tip:** Refer to Output Directories of the v++ Command in the _Vitis
Reference Guide_ (UG1702) for an understanding of the location of
various output files.
The linking process defines important architectural details of the system
design. In particular, this is where the design is enabled for profiling or
debug, where you specify the number of kernel instances to instantiate
into hardware, where kernel instances are assigned to SLRs, and where
you define connections from PL kernel ports to global memory or to AI
Engine applications. The following sections discuss some of these build
options: Profiling and Tracing the Application and Debugging System
Projects.

Linking with the Vitis Integrated Flow

In the Vitis Integrated Flow, the linker automatically runs Vivado
synthesis and implementation on the project (hence the name
"Integrated"), and creates a fixed hardware platform (.xsa) for use by the
Vitis packaging process as described in Packaging for Vitis Flow.
The following is an example command line to link a PL kernel (vadd.co)
with an AI Engine graph (libadf.a) and a Versal adaptive SoC platform,
using specific linking options (system.cfg):


```
v++ -t hw_emu --platform xilinx_vck190_base_202520_1 --link
vadd.xo libadf.a --config ./system.cfg -o
binary_container.xsa
```
This command contains the following arguments:

**-t <arg>**
Specifies the build target. When linking, you must use the same -t
and --platform arguments specified when compiling the PL kernels
and AI Engine graph application.

**--platform <arg>**
Specifies the platform to link with the system design. In the example
command above the custom_vck190 platform is a custom platform
designed to work with the --export_archive command.

**--link**
Link the kernels, graph, and platform into a system design.

**<input>.xo**
Specifies the input PL kernel object files (.xo) to link with the AI
Engine graph and the target platform. This is a positional parameter.

**libadf.a**
Specifies the input compiled AI Engine graph application to link with
the PL kernels and the target platform. This is a positional
parameter. If your design uses multiple partitions, you need to
specify each corresponding libadf file when linking the design. For
detailed instructions on handling multiple partitions, refer to
Compiling AI Engine Graph for Independent Partitions in the _AI
Engine Tools and Flows User Guide_ (UG1076).

**--config ./system.cfg**
Specify a configuration file that is used to provide v++ command
options for a variety of uses, including connectivity and debug
options. Refer to Vitis Compiler Configuration File in the _Vitis
Reference Guide_ (UG1702) for more information on the --config
option.

**-o binary_container.xsa**


##### ★

###### !!

```
Specifies the output file name. The output file in the link stage will
be an .xsa file due to the use of a Versal platform. The default output
name is a.xsa.
```
```
Tip: For AMD Zynq™ MPSoC based platforms, the output of the
link command will be an .xclbin file rather than the .xsa.
```
Linking with the Vitis Export to Vivado Flow

In the Vitis Export to Vivado flow, the linker stops before running RTL
synthesis and outputs a project archive (VMA file) for export to the
Vivado Design Suite. This flow lets you open the Vitis archive in the
Vivado tools for directed synthesis, place and route, and timing closure.
The Vitis Export to Vivado flow requires custom Versal platforms designed
specifically to support this feature.
An example command follows:

```
v++ --target hw --platform custom_vck190 --link vadd.xo
libadf.a --config ./system.cfg \
--export_archive -o hw-vadd.vma
```
**Important:** The --export_archive command cannot be used with the
--target hw_emu (or -t) command. An error will be returned.

This command is similar to the prior command, with the following
differences:

**--export_archive**
Specifies the creation of the .vma file to export to the Vivado Design
Suite. This option stops v++ from automatically running Vivado
synthesis and place and route, and instead lets you manually launch
and direct the implementation and timing closure of the design as
described in Vitis Export to Vivado Flow.

**--platform custom_vck**
The --export_archive command can only be used with a custom
platform compatible with the Vitis export to Vivado flow.

**-o hw-vadd.vma**
Specifies the output file name for the .vma file produced by the --
export_archive command.


The .vma file is imported into the original extensible platform project in
the Vivado Design Suite using the vitis::import_archive Tcl
procedure. Development can then continue in the Vivado project,
including additional design modifications, simulation, synthesis, and
implementation.
After implementation and timing closure, the write_hw_platform -
fixed command has been enhanced to encapsulate the XRT metadata
from the .vma file into the output .xsa. You can also export the XSA for
the hardware emulation target from the Vivado tool and run emulation in
the Vitis tool.
If the .vma is updated or iterative design methodology is used, the
previously imported VMA needs to be removed using
vitis::remove_archive_hierarchy before the updated VMA is
imported.
Vitis Export to Vivado Flow Detailed Example further explains the flow.

Linking the VSS Component to the Platform

The linking and generation of the VMA with a VSS component follows the
same setup as Vitis export to Vivado, with the addition of the .vss
archive.

```
v++ --link --target hw --export_archive -save-temps --
platform <platform_name> --config ./src/system.cfg
<list_of_xo> <VSS archive> --output <VMA file>
```
Enabling Profile and Debug when Linking

To capture and visualize profiling and trace information, or to enable your
design for debugging, you will need to add specific commands during the
v++ linking phase, and sometimes during v++ compilation. The tool must
instrument the profile IP using --profile Optionsin the v++ linking phase
and enable the profiling during the runtime. To enable debugging the
application you can specify one of the --debug Options.
During v++ linking, the application developer needs to add profile IP to
the design to capture the profiling data and preferably choose the
memory resources for storing and offloading data during the runtime.


- You can add PL monitors to capture tracing information on their
    design by using --profile command. This adds the logic to capture
    profile data for data traffic between the kernel and host, kernel
    stalls, the execution times of kernels, and compute units (CUs). The
    instrumentation can be added using options, --profile.data, --
    profile.stall, and --profile.exec, as described in --profile
    Options.
- You also can specify the choice of memory resources or FIFO in the
    PL to store captured data. On large designs that span multiple SLRs,
    the tracing infrastructure can cause timing issues as there is one
    offload point and all trace data must cross SLRs to reach it. For these
    use cases, multiple memory resources can be used for offloading the
    trace data.

To enable the capture of profile data or trace information during the
application runtime, you can choose from a variety of options to add to
the xrt.ini File, which configures the runtime. See Profiling the
Application in the _Data Center Acceleration using Vitis_ (UG1700) for more
information.

Creating Multiple Instances of a Kernel

By default, the linker builds a single hardware instance from a kernel.
However, you can instantiate multiple hardware compute units (CUs)
from a single kernel. This can improve performance as the host program
can now make multiple calls to a given kernel, and see them executed in
parallel on the different compute units.
Multiple CUs of a kernel can be created by using the connectivity.nk
option in the v++ config file during linking. Edit a config file to include the
needed options, and specify it in the v++ command line with the --
config option, as described in v++ Command in the _Vitis Reference
Guide_ (UG1702).
For example, for the vadd kernel, two hardware instances can be
implemented in the config file as follows:

```
[connectivity]
#nk=<kernel name>:<number>:<cu_name>,<cu_name>...
nk=vadd:
```

##### ★

Where:

**<kernel_name>**
Specifies the name of the kernel to instantiate multiple times.

**<number>**
The number of kernel instances, or CUs, to implement in hardware.

**<cu_name>,<cu_name>...**
Specifies the instance names for the specified number of instances.
This is optional, and the CU name will default to kernel_1 when it is
not specified. The delimiter between kernel instances is a comma.
In the example above, the kernel_name and the number of CUs are
specified, but not the cu_name. In this case vadd_1 and vadd_2 will
be added to the design.

Then the config file is specified on the v++ command line:

```
v++ --config vadd_config.cfg ...
```
**Tip:** You can check the results by using the xclbinutil command to
examine the contents of the xclbin file. Refer to xclbinutil Utility in the
_Vitis Reference Guide_ (UG1702).
The following example results in three CUs of the vadd kernel, named
vadd_X, vadd_Y, and vadd_Z in the xclbin binary file:

```
[connectivity]
nk=vadd:3:vadd_X,vadd_Y,vadd_Z
```
Mapping Kernel Ports to Memory

The link phase is when the memory ports of the kernels are connected to
memory resources such as DDR, HBM, and PLRAM.
By default, all kernel memory interfaces are connected to the same
global memory bank (or gmem). As a result, only one kernel interface can
transfer data to or from the memory bank at one time, limiting the
performance of the application due to memory access.
Because of this, it is important to explicitly specify which global memory
bank each kernel argument (or interface) is connected to. Proper
configuration of kernel to memory connectivity is important to maximize


###### !!

bandwidth, optimize data transfers, and improve overall performance.
Even if there is only one compute unit in the device, mapping its input
and output arguments to different global memory banks can improve
performance by enabling simultaneous accesses to input and output
data.
The following block diagram illustrates the Using Multiple DDR Banks
example in Vitis-Tutorials on GitHub. This example connects the input
pointer interface of the kernel to DDR bank 0, and the output pointer
interface to DDR bank 1.

**Figure: Global Memory Two Banks Example**

**Important:** Up to 15 separate kernel interfaces can be connected to a
given global memory bank. Therefore, if there are more than 15 memory
interfaces in the design you must explicitly perform the memory
mapping as described here, using the --conectivity.sp option to
distribute connections across different memory banks.

Start by assigning the kernel arguments to separate bundles to increase
the available interface ports, then assign the arguments to separate
memory banks. The following example uses the interfaces described in
HW Interfaces in the _Data Center Acceleration using Vitis_ (UG1700).

1. In the C/C++ kernel code assign arguments to separate bundles
    using the INTERFACE pragma prior to compiling them:

```
void cnn( int *pixel, // Input pixel
int *weights, // Input Weight Matrix
int *out, // Output pixel
... // Other input or Output ports
```
```
#pragma HLS INTERFACE m_axi port=pixel offset=slave
bundle=gmem
#pragma HLS INTERFACE m_axi port=weights offset=slave
```

###### !!

```
bundle=gmem
#pragma HLS INTERFACE m_axi port=out offset=slave
bundle=gmem
```
```
In this example, the cnn kernel has 3 arguments: pixel, weights
and out. Using the bundle attribute of the INTERFACE pragma, each
argument is mapped to a specific interface. The pixel and out
arguments are both mapped to the same interface called gmem. The
weights argument is mapped to a different interface called gmem1.
The resulting kernel has therefore 2 distinct interfaces (gmem and
gmem1) which can be connected to different memory banks.
Important: You must specify bundle= names using all lowercase
characters to be able to assign it to a specific memory bank using
the --connectivity.sp option.
```
2. Use the --connectivity.sp option, or include it in a config file, as
    described in --connectivity Options.
    For example, for the cnn kernel shown above, the connectivity.sp
    option in the config file would be as follows:

```
[connectivity]
#sp=<compute_unit_name>.<argument>:<bank name>
sp=cnn_1.pixel:DDR[0]
sp=cnn_1.weights:DDR[1]
sp=cnn_1.out:DDR[0]
#sp=<aie_instance>.<gmio_port>:<memory_sp_tag_name>[bank_
number]
sp=ai_engine_0.my_gmio:LPDDR[0]
```
```
Where:
```

##### ★

###### ✎

##### ★

```
◦ <compute_unit_name> is an instance name of the CU as
determined by the connectivity.nk option, described in
Creating Multiple Instances of a Kernel, or is simply
<kernel_name>_1 if multiple CUs are not specified.
◦ <argument> is the name of the kernel argument. Alternatively,
you can specify the name of the kernel interface as defined by
the HLS INTERFACE pragma for C/C++ kernels, including m_axi_
and the bundle name. In the cnn kernel above, the ports would
be m_axi_gmem and m_axi_gmem1.
```
```
Tip: For RTL kernels, the interface is specified by the
interface name defined in the kernel.xml file.
◦ <bank_name> is denoted as DDR[0], DDR[1], DDR[2], and
DDR[3] for a platform with four DDR banks. You can also specify
the memory as a contiguous range of banks, such as DDR[0:2],
in which case XRT will assign the memory bank at runtime.
Some platforms also provide support for LPDDR, PLRAM, HBM,
HP or MIG memory, in which case you would use LPDDR[0],
PLRAM[0], HBM[0], HP[0] or MIG[0]. You can use the
platforminfo utility to get information on the global memory
banks available in a specified platform. Refer to platforminfo
Utility in the Vitis Reference Guide (UG1702) for more
information.
In platforms that include both DDR and HBM memory banks,
kernels must use separate AXI interfaces to access the different
memories. DDR and PLRAM access can be shared from a single
port.
Note: The SP tag name is declared in Vivado block design as
part of the platform properties. The SP tag names can be
customized by user to help distinguish specific memory
controllers.
```
```
Tip: Assigning kernel interfaces to specific memory banks
might also require you to specify the SLR to place the kernel
into. For more information, refer to Assigning Compute Units to
SLRs on Alveo Accelerator Cards in the Data Center Acceleration
using Vitis (UG1700).
```
You can use the Device Hardware Transaction view in Vitis Analyzer to


###### !!

observe the actual DDR Bank communication, and to analyze DDR usage.

**Figure: Device Hardware Transaction View Transactions on DDR
Bank**

Specifying Streaming Connections

Support for hardware accelerator pipelines that communicate through
streams is one of the major advantages of FPGAs, FPGA-based SoCs, and
Versal adaptive SoC devices; it can be used in DSP and image processing
applications, in addition to communication systems. Kernel ports involved
in streaming are defined within the kernel, and are not addressed by the
host program. There is no need to send data back to global memory
before it is forwarded to another kernel for processing. The connections
between the kernels are directly defined during the v++ linking process
as described below.
A streaming data output port of one kernel can be connected to the
streaming data input port of another kernel, or between a PL kernel and
the PLIO of an ADF graph application, during linking using the --
connectivity.sc option. This option can be specified at the command
line, or from a config file that is specified using the --config option, as
described in v++ Command in the _Vitis Reference Guide_ (UG1702).

**Important:** An error occurs if the --connectivity.sc kernel drives
itself.

To connect the streaming output port of a producer kernel to the
streaming input port of a consumer kernel, set up the connection in the
v++ config file using the connectivity.stream_connect option as
follows:


###### !!

```
[connectivity]
#stream_connect=<cu_name>.<output_port>:<cu_name>.<input_port
>:[<fifo_depth>]
stream_connect=vadd_1.stream_out:vadd_2.stream_in
stream_connect=vadd_2.stream_in:ai_engine_0.DataIn
```
Where:

- <cu_name> is an instance name of the CU as determined by the
    connectivity.nk option, described in Creating Multiple Instances of
    a Kernel. The cu_name can be specified in the config file as described
    in Creating Multiple Instances of a Kernel, or is defined automatically
    by the tool when not otherwise specified.
- <output_port> or <input_port> is the streaming port defined in
    the producer or consumer kernel.
       **Important:** If the port-width of the output and input ports do not
    match, the Vitis compiler automatically inserts a data-width
    converter between the two ports as part of the build process. The
    inclusion of the data-width converter is either truncate a larger bit-
    width output to a smaller bit-width input, or expand a smaller bit-
    width to a larger bit-width.
- [:<fifo_depth>] inserts a FIFO of the specified depth between the
    two streaming ports to prevent stalls. The value is specified as an
    integer.

Specifying SLR Region for SSI Devices

In Vitis linker, a CU or kernel is associated to instances marked with
PFM.REGION by using the same string label with the additional
connectivity directive connectivity.region. This indicates that the
kernel is to be placed close with other kernels or IPs in the same SLR.
For the v++ linker, the region is set with the command line option --
connectivity.region arg <cu_name>:<label>. In the linker
configuration file, it is specified as follows:

```
[connectivity]
region=<region_label>:<cu_name_1>,<cu_name_2>
```
Managing Clock Frequencies


###### ✎

The selection of clocks and associated frequencies is an important part of
defining the performance of algorithms and signal processing blocks of a
system. Depending on the source of the input data and destination of the
results, different solutions need to be engineered to meet the design
requirements. This section describes the clocking of processing elements
such HLS, RTL PL kernels or AI Engine kernels added with Vitis to an
extensible platform. The interface to input data and output results can be
categorized as streaming or memory access.

**Note:** The number of clocking resources available is device dependent
and it is recommended to carefully plan the clock usage.

For streaming access, the bit-width of the data and clock frequency
determines the throughput. The HLS, RTL or AI Engine kernels processing
the data need to sustain the throughput to avoid loss of data. The
throughput used here is defined as:
throughput = bit-width * clock frequency in Hz / initiation interval (bits /
second)
For AI Engine, the interface clock frequency is specified on PLIO to
determine the DMA scheduling or stream access rate, but the kernel
itself always run at AIE Clock. For details see the topic AI Engine-to-PL
Rate Matching in the _AI Engine Kernel and Graph Programming Guide_
(UG1079).

**Table: Clocking Examples for Kernels with Stream Access**

```
Data access type Design impact for Vitis Comments
```
```
Synchronous single-
rate
```
```
Connect to platform
clocks and data paths
```
```
Kernel clocks match
the source and
destination in the
platform.
```
```
Synchronous multi-
rate
```
```
To reduce the clock
rate while
maintaining
throughput
requirements, the bit-
width can be
increased. Vitis will
add a Data Width
Converter (DWC)
```

###### ✎

**Data access type Design impact for Vitis Comments**

- Connect to
    platform clocks
    and data paths
- Optional: Add
    new clocks
- Optional: Add
    DWC
- Optional: Add
    FIFO

```
block to manage the
relationship between
the bit-width and
clocks whose
frequency is in a
powers of 2 relation.
If necessary, Vitis will
also infer missing
clocks. For non
powers of 2 relations,
you need advanced
clocking and
handshaking
techniques. Refer to
Versal Adaptive SoC
Clocking Resources
Architecture Manual
(AM003) and Clocking
Wizard for Versal
Adaptive SoC
LogiCORE IP Product
Guide (PG321).
Note: A multi-rate
design is
synchronous if the
clocks have rational
relation and a
common reference
(originates from the
same PLL/MMCM). If
only one of the multi-
rate clocks exist in
the platform, Vitis will
infer a clock wizard to
satisfy this condition.
```

###### ✎

###### ✎

```
Data access type Design impact for Vitis Comments
```
Time division
multiplexing • Connect to
platform clocks
and data paths

- Optional: Add
    new clocks
- Optional: Add
    DWC
- Add FIFO or
    buffers

```
The kernel exploits
running at higher
throughput than
incoming data by
buffering the
incoming data and
processing each
buffer in sequence.
This is closely related
to multi-rate signal
processing, except
that having buffers is
mandatory.
Note: Vitis can
infer DWC and
additional clocks to
support powers of 2
rate changes. The
multiplexing
mechanism and
buffers need to be
designed by the
kernel developer.
```
Packet-switching Need buffers and
logic for handling the
control and payload.

```
Similar to multi-rate,
but can require clock
rate overhead to
manage the control
headers.
Note: Vitis does
not support
automatically
inferring packet
switching. The packet
handling mechanism
need to be designed
by the kernel
```

###### ✎

```
Data access type Design impact for Vitis Comments
developer.
```
```
Asynchronous
```
- Connect to
    platform clocks
    and data paths
- Optional: Add
    clocks
- Add CDC
- Add FIFO

```
CDC (Clock domain
crossing) logic is
required to transfer
data across unrelated
clock domains. The
processing kernel
throughput needs to
be equal or higher
than the input data.
FIFO buffers need to
be inserted to handle
differences in
throughput and stall
handshaking. Refer
to Specifying
Streaming
Connections for
adding FIFO.
Note: AI Engine is
clocked by a separate
PLL. Even if the PLIO
has same frequency,
the phase relation is
unknown so CDC are
always inserted by
Vitis.
```
**Figure: Synchronous Single-Rate with Clock from Platform**


**Figure: Synchronous Multi-rate with Clock from Platform and
Inferred DWC**

**Figure: Synchronous Multi-rate with Inferred Clocks and CDC**

**Figure: Asynchronous Single-rate with CDC Related to AI Engine
PLIO**


###### ✎Note: The figures above are simplified for illustrative purposes. The

data produced and consumed can be the same block or multiple blocks.
The HLS and AI Engine can interact with as many other kernels as
resource and interface permits.

In Vitis, all memory types are modeled as AXI slave interfaces with
metadata, and any clock conversion is handled implicitly through the AXI
network connecting kernel AXI master to the memory AXI slave. In cases
with HLS kernels converting from memory to stream or stream to
memory, or if the memory access has contention from several kernels,
the HLS kernel coding could need performance optimizations to meet the
required access type. This is also true if the access is to a shared
resource. For details on connecting memories, refer to Mapping Kernel
Ports to Memory. For details on HLS coding, refer to Memory Mapped
Interfaces in the _Vitis High-Level Synthesis User Guide_ (UG1399).
The extensible platform XSA contains information of available clock
domains. Running the platforminfo -d <platformname>.xsa utility will
list the platform clock domains under Clocking Information. For further
details and examples, refer to Identifying Platform Clocks.

**Table: Using Clock Options in Vitis**

```
Expected outcome Link options Comment
```
```
Use default
platform
clock
source pin
```
```
Not needed Vitis automatically
connects unspecified
kernel to default clock
source pin.
```
```
Use non-
default
platform
clock
```
```
Use clock.id All kernel clock pins will be
driven by the platform
clock source pin with ID. If
the kernel contains
```

```
Expected outcome Link options Comment
multiple clock pins, you
can specify
<kernel>.<clk pin> to
differentiate between the
clocks.
```
- Use freqhz
- Optional: Use
    clock.default_tolerance

```
The requested frequency
must match any existing
platform clocks within the
tolerance range. Unless
explicitly set, the default
tolerance will be 5%.
```
```
Add a new
clock
```
```
Use freqhz Vitis will add a clock
wizard to generate the
requested clock frequency.
In some cases when it's
not possible to generate
the exact frequency, the
tool will generate the
closest acceptable
frequency within the
tolerance range.
```
For details on how to use link options, refer to --clock Options.

Identifying Platform Clocks

The kernels can have any number of independent and edge-aligned
clocks, and platforms can have multiple kernels running at different clock
frequencies under user control. Platforms have a variety of clocking:
processor, PL, and AI Engine clocking. The following table explains the
clocking for each type.

**Table: Platform Clocks**

```
Clock Description
```
```
AI Engine Can be configured in the platform in the AI Engine
```

##### ★

```
Clock Description
IP. The AI Engine PLL frequency must match the
CIPS HSM0 frequency.
```
```
Processor Can be configured in the platform in the CIPS IP. The
HSM0 frequency must match the AI Engine IP PLL
frequency.
```
```
Programmable
Logic (PL)
```
```
Can have multiple clocks and can be configured in
the platform.
```
```
NoC Device dependent and can be configured in the
platform in the CIPS and NoC IP.
```
1. These clocks are derived from the platform and are affected by
    the device, speed grade, and operating voltage.

Vitis clocking automation differentiates between three types of platform
clock: scalable, fixed, and fixed_non_ref, which are specified in Vivado
during platform capture, and define how automation can employ clocks
to meet v++ clocking directives.

**Tip:** You cannot mix fixed and scalable clocks on a single kernel, but
they can be mixed across different kernels within a single .xclbin file.

You can determine the clocks available in the target platform by using
the platforminfo command.

```
=================
Clock Information
=================
Default Clock Index: 2
Default Clock Frequency: 312.499712
Default Clock Pretty Name: PL 2
Clock Index: 0
Frequency: 156.249856
Status: fixed
Name: clk_wizard_0_clk_out2
Pretty Name: PL 0
Inst Ref: clk_wizard_0
```

Comp Ref: clk_wizard
Period: 6.400006
Normalized Period: .006400
Clock Index: 1
Frequency: 104.166570
Status: fixed
Name: clk_wizard_0_clk_out1
Pretty Name: PL 1
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 9.600009
Normalized Period: .009600
Clock Index: 2
Frequency: 312.499712
Status: fixed
Name: clk_wizard_0_clk_out3
Pretty Name: PL 2
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 3.200003
Normalized Period: .003200
Clock Index: 3
Frequency: 78.124928
Status: fixed
Name: clk_wizard_0_clk_out4
Pretty Name: PL 3
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 12.800012
Normalized Period: .012800
Clock Index: 4
Frequency: 208.333141
Status: fixed
Name: clk_wizard_0_clk_out5
Pretty Name: PL 4
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 4.800004
Normalized Period: .004800
Clock Index: 5
Frequency: 416.666283
Status: fixed


```
Name: clk_wizard_0_clk_out6
Pretty Name: PL 5
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 2.400002
Normalized Period: .002400
Clock Index: 6
Frequency: 624.999425
Status: fixed
Name: clk_wizard_0_clk_out7
Pretty Name: PL 6
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 1.600001
Normalized Period: .001600
```
For Versal devices, the --part option can be used instead of --platform
with the v++ --link and v++ --package commands. With --part the
tool generates a base design for use on the device, and can generally be
used while waiting for the development of a full platform specification.
However, the v++ linker generated base platform design employs PLRAM
only, so is unsuitable for running Linux applications on the target.

**Fixed Clocks**
Platform clocks declared as 'fixed' can be used to drive kernels at
their specified frequency, or as a reference clock for v++ clock
automation to derive clocks as needed.
Fixed clocks use MMCMs to generate frequencies other than the
fixed frequencies defined on the platform. For example, if you
specify clock frequencies: 60, 200, and 450, the Vitis compiler adds
all the necessary logic to generate the required clocks from the
available platform fixed clocks.
Use the --freqhz option to specify the clock frequency for a kernel.
The --clock Optionscan also be used to specify PL kernel
connections to specific platform clocks, or to specify clock
frequencies that are generated from fixed clocks on the platform.


##### ★

```
fixed_non_ref
Platform clocks declared as fixed_non_ref can be used to drive
kernels at their specified frequency, but cannot be used as a
reference clock by v++ clock automation.
The canonical justification for declaring a platform clock
fixed_non_ref is because it is driven by an MBUFG logical
clock resource.
Refer to --clock Optionsfor more details.
```
**Tip:** You cannot mix fixed and scalable clocks on a single kernel, but
they can be mixed across different kernels within a single .xclbin file.
You can determine the clocks available in the target platform by using
the platforminfo command.

```
=================
Clock Information
=================
Default Clock Index: 2
Default Clock Frequency: 312.499712
Default Clock Pretty Name: PL 2
Clock Index: 0
Frequency: 156.249856
Status: fixed
Name: clk_wizard_0_clk_out2
Pretty Name: PL 0
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 6.400006
Normalized Period: .006400
Clock Index: 1
Frequency: 104.166570
Status: fixed
Name: clk_wizard_0_clk_out1
Pretty Name: PL 1
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 9.600009
Normalized Period: .009600
Clock Index: 2
Frequency: 312.499712
Status: fixed
```

Name: clk_wizard_0_clk_out3
Pretty Name: PL 2
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 3.200003
Normalized Period: .003200
Clock Index: 3
Frequency: 78.124928
Status: fixed
Name: clk_wizard_0_clk_out4
Pretty Name: PL 3
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 12.800012
Normalized Period: .012800
Clock Index: 4
Frequency: 208.333141
Status: fixed
Name: clk_wizard_0_clk_out5
Pretty Name: PL 4
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 4.800004
Normalized Period: .004800
Clock Index: 5
Frequency: 416.666283
Status: fixed
Name: clk_wizard_0_clk_out6
Pretty Name: PL 5
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 2.400002
Normalized Period: .002400
Clock Index: 6
Frequency: 624.999425
Status: fixed
Name: clk_wizard_0_clk_out7
Pretty Name: PL 6
Inst Ref: clk_wizard_0
Comp Ref: clk_wizard
Period: 1.600001
Normalized Period: .001600


For Versal devices, the --part option can be used instead of --platform
with the v++ --link and v++ --package commands. With --part the
tool generates a base design for use on the device, and can generally be
used while waiting for the development of a full platform specification.
However, the v++ linker generated base platform design employs PLRAM
only, so is unsuitable for running Linux applications on the target.

Syncing PL Clocks with the AI Engine

In the ADF programming model, a PLIO represents an AXI4-Stream
attachment point from the AI Engine to a PL component (either a PL
kernel or a platform IP). PLIO clock frequencies can be specified explicitly
to match the PL interfaces in simulation. In addition, when you link the
ADF graph into the platform using the v++ -link command, you can
direct the tools to generate precisely the clock frequencies required by
your application. PL kernels can be independently clocked, and the v++
linker will automatically insert clock domain crossing circuitry into the
design as needed.
The recommended best practice to sync AI Engine clocks with PL is as
follows:

1. If you know the PLIO clock frequencies, they should be specified in
    the ADF graph PLIO constructors. If every PLIO will be clocked at the
    same frequency, instead of specifying in constructors, use v++ -c
    --mode aie --freqhz <frequency>.
    If PLIO clock frequencies are not known at ADF graph compilation
    time, defer the actual binding until v++ link time and the compiler
    will assume all are clocked at 100 MHz for simulation.
2. For the Vitis compilation of a PL kernel peer of a PLIO, specify the
    same clock frequency.

```
v++ -c --mode hls -freqhz <frequency>
```
3. When linking the compiled ADF graph (libadf.a) with the compiled
    .xo PL kernels, specify the same PL frequencies. If needed, the link-
    time clock frequencies can override compile time frequencies. In
    respect to PLIOs, AI Engine simulation behavior can differ from
    hardware.


###### ✎

```
v++ -l --platform <pfm_name> --freqhz <frequency>
```
Vitis Export to Vivado Flow Detailed Example

This section explains the Vitis Export flow as shown in the following
diagram. The complete flow (from hardware design creation to exporting
.xclbin) has seven steps.

**Figure: Vitis Export to Vivado Flow**

You can execute the flow in the following steps.

1. Start by creating the Versal custom platform design. The Vitis export
    flow can use either a Flat Hardware Design Platform or a BDC Based
    Hardware Platform. You can use HLS-based components, or RTL-
    based packaged IP, or standard IP from the IP catalog in your Vivado
    design.
       **Note:** For details on exporting platform from Vivado to Vitis, see
    Exporting Platforms to Vitis in the _Vivado Design Suite User Guide:_


##### ★

_Designing IP Subsystems Using IP Integrator_ (UG994).

After creating the platform, run synthesis to clean any issues related
to hardware realization. Only use the flow for the Versal device
platforms. Export the platform into extensible.xsa using following
steps:
a. Click on Export Platform under IP Integrator in the Flow
Navigator.

```
In Export Platform, select the Presynthesis option to generate
the extensible.xsa.
```
```
Tip: If the Export Platform option is disabled, go to Project
Settings and select Project is an extensible Vitis Platform.
```

###### !!

```
b. From the Tcl console, enter the command write_hw_platform
-f <filename>.xsato generate the extensible XSA.
```
2. In the Vitis tool, the exported extensible XSA from Vivado is used to
    compile the AI Engine graph, HLS kernels and run v++ link to export
    VMA. The steps are described below:
       ◦ Compiles the AI Engine graph (libadf.a)
       ◦ Compiles PL kernels (.xo)
       ◦ Updates system.cfg file for connectivity
       ◦ Runs v++ linker with the --export_archive option
    In the Vitis Export to Vivado flow, the system linking process occurs
    as usual, but the automatic launch of Vivado synthesis and place
    and route is skipped. Instead, the v++ --link --export_archive
    command is used to generate the Vitis metadata archive (.vma) to
    export to the Vivado Design Suite.

```
v++ --link --export_archive --platform ../<>.xsa --config
../system.cfg \
<>.xo ./libadf.a -o <vma_file>.vma
```
```
Important: The --export_archive command supports either
target=hw (--target hw) only, or without specifying either --
target or -t). An error is reported if target = hw_emu is used.
```
3. Open the Vivado project after generating the .vma file from the Vitis
    tools. Import the .vma file in the Vivado project with the following Tcl
    procedure:

```
vitis::import_archive ./vma_path/<vma_file>.vma
```

```
a. A new variant for the dynamic region block design container will
be created by cloning the VMA's block design and made active.
b. Within the dynamic region block design, Vitis content is
encapsulated within a level of hierarchy that the user should
consider essentially read-only whenever they intend to use XRT
after implementing the design in Vivado. See the Vitis Export
Flow Guidelines and Limitations section for more details on how
to preserve XRT metadata consistency while updating the
design in Vivado.
c. The .vma region can be opened in Vivado. Alternatively, to
rerun Vitis flows, you can re-export an extensible XSA after
removing Vitis content via vitis::remove_archive_hierarchy.
```
4. After importing the .vma to the Vivado tools, design modifications
    can only be completed in Vivado. See the Vitis Export Flow
    Guidelines and Limitations section for more information.
       a. If there are changes to the .vma file, such as changes to the AI
          Engine design, PL kernels, or the PLIO boundary, go to Step 5 to
          regenerate the .xsa file and reiterate through the Vitis Export to
          Vivado flow.
       b. If the design changes are only related to Vivado, simulate the
          design, synthesize and implement the design to meet timing,
          and go to Step 6 for generating the fixed.xsa.
5. If the design requires changes related to the AI Engine, PL kernels, or
    updates in the PLIO boundary, these changes require updates to the
    linked system design, and regenerating the .vma in the Vitis tool.
    Thus, you must first remove the previously imported .vma from the
    Vivado project using one of two Tcl procedures:
       a. Use the vitis::remove_archive_hierarchy procedure to
          remove the imported .vma file while preserving any work done
          to the Vivado project after importing the .vma.
       b. Use the vitis::remove_archiveprocedure to restore the
          Vivado project to its state prior to importing the .vma file,
          removing both the .vma and any changes to the project.
    After removing .vma from the Vivado design, you can make any
    changes to the project. Vitis depends only upon the dynamic region
    block design and to potential connectivity points declared through
    PFM APIs. Update system.cfg to update the boundary connections.
    If there is a need to export the extensible.xsa for the second


```
iteration or later from Vivado, use the vitis::remove_archive
command and repeat Step 1 to export the extensible.vma; repeat
Step 2 and 3 to export the VMA from Vitis and import VMA to Vivado
respectively.
```
6. Once you have implemented your design, you can generate a
    fixed.xsa from the Vivado project by using the following command:

```
write_hw_platform -fixed ./<fixed_xsa>.xsa
```
```
You can use this XSA to perform application development for
PetaLinux / Yocto or XRT-based apps development. You can use it to
perform PS-based apps development through Vitis embedded
software flow, or baremetal flow as it has been done traditionally.
You can use the fixed XSA from the preceding command to test the
design on the hardware target only. If you want to run hardware
emulation, remove the VMA by the vitis::remove_archive
command and generate the extensible XSA, then take the extensible
XSA through the Vitis flow.
```
7. After modifying the design, you can test the design on hardware or
    run hardware emulation. Follow the steps below to generate the
    fixed XSA for testing the design on hardware and hardware
    emulation.
    To test the design on hardware, first run synthesis and
    implementation on the design. Use the command
    write_hw_platform -fixed ./<path to fixed.xsa> to generate
    fixed XSA.
    To run hardware emulation, follow the steps below to generate the
    fixed XSA:
       a. Re-generate the output products
       b. Execute the command launch_simulation -scripts only
          c. Run compile.sh
       d. Run elaborate.sh
       e. Execute the command write_hw_platform -fixed -
          include_sim_content <path to fixed xsa>
    When you are ready to run the design on the hardware target (-
    t=hw) or hardware emulation target (-t=hw_emu), run v++ --
    package to generate the .xclbin. To generate the xclbin for
    hardware and hardware emulation, use the respective fixed.xsa.
    Set the t= hw or hw_emu, then provide the required software binaries


```
and files to generate the .xclbin.
```
Vitis Export Flow Guidelines and Limitations

The v++ compiler operates on a Vivado project that has been
encapsulated in an extensible XSA built in Vivado. Conversely, the block
design of the VMA is imported into a project as a design source that the
user can continue to modify in Vivado.
In general, any modification to the Vivado project after
vitis::import_archive that does not invalidate the contract between
the imported design and the .xclbin metadata contained within the VMA
is supported. The following table enumerates supported and prohibited
operations.
Supported Vivado modifications after importing a VMA:

- Adding, removing, and reconfiguring IPs and RTL modules outside of
    and unconnected to the Vitis region hierarchy within the dynamic
    region block design.
- Add, removing, or changing connections unconnected to the Vitis
    region hierarchy within the dynamic region block design.
- Changing clock frequencies on clock wizard instances outside of the
    Vitis region hierarchy within the dynamic region block design.
- Changing QoS settings on axi_noc instances in the dynamic region
    block design.
- Adding .xdc constraints associated with any part of the design,
    including within the Vitis region hierarchy within the dynamic region
    block design.

Vivado modifications that require removing VMA hierarchy, updating Vitis
kernels, connectivity and relink VMA export, and then reimport the VMA:

- Adding or deleting any IP instances or connections within the Vitis
    region hierarchy within the dynamic region block design.
- Adding or deleting connections between the dynamic region and the
    Vitis region hierarchy.
- Changes to the address map that modifies any address APERTURES
    or IP addressing in the Vitis region hierarchy within the dynamic
    region block design.

Current limitations of the Vitis Export flow include the following:


##### ★

- Supported for Versal platforms only
- Project changes that modify the netlist path to the Vitis region
    hierarchy within the dynamic region block design.

### Managing Vivado Synthesis,

### Implementation, and Timing Closure

**Tip:** This topic requires an understanding of the Vivado Design Suite
tools and design methodology as described in _UltraFast Design
Methodology Guide for FPGAs and SoCs_ (UG949), or the _Versal Adaptive
SoC Design Guide_ (UG1273).

All the flows introduced in Vitis Flows and Build Environment use the
Vivado Design Suite for synthesis and implementation of the linked
system design. The difference is how the user interacts with Vivado tools.
In the Vitis Integrated Flow, this is controlled by command line or
configuration file argument sections applying strategies and settings,
while the Vitis Export to Vivado Flow uses traditional Vivado methods.
This document covers how this is managed for the Vitis Integrated Flow.

###### Working with Vivado in the Vitis Integrated Flow

The Vitis Integrated Flow automatically launches the Vivado Design Suite
to synthesize the linked system design, place and route the elements of
the design, resolve timing, and generate the bitstream for the design. In
most cases, the Vitis Integrated Flow completely abstract away the
underlying process of synthesis and implementation of the hardware
design. This removes the application developer from the typical hardware
development process and the need to manage constraints such as logic
placement and routing delays. The Vitis Integrated Flow automates much
of the FPGA implementation process.
While automated, this flow does offer some opportunity for manual
intervention. The process is broken down into a series of major steps that
can be interrupted to enable customization when necessary. In some
cases, you might want to exercise some control over the synthesis and
implementation processes deployed by the Vitis linker, especially when
large designs are being implemented. The Vitis Integrated Flow offers
some control through specific options that can be specified in a v++
configuration file, or from the command line. The following sections


describe some of the methods you can use to control the Vivado
synthesis and implementation results.

- Using the --vivado options to manage the Vivado tool.
- Using multiple implementation strategies to achieve timing closure
    on challenging designs.
- Using the -to_step and -from_step options to run the compilation
    or linking process to a specific step, perform some manual
    intervention on the design, and resume from that step.
- Interactively editing the Vivado project, and using the results for
    generating the FPGA binary.

Using the --vivado and --advanced Options

Using the --vivado option, as described in --vivado Options, and the --
advanced option as described in --advanced Options, you can perform a
number of interventions on the standard Vivado synthesis or
implementation.

1. You can specify the strategy to be used by the Vivado tool during
    synthesis, implementation, or when generating reports during the
    build process. The strategy specified can be one of the standard tool
    strategies, or can be a custom-defined strategy that you have
    previously created in the Vivado tool. Use the --vivado.prop
    command as shown below.
    The original Tcl command to set the property on a run object looks
    like the following:

```
set_property strategy Flow_AreaOptimized_medium [get_runs
synth_1]
```
```
The v++ command is rewritten as shown below:
```

##### ★

```
◦ Synthesis Strategy:
```
```
--vivado.prop
run.synth_1.strategy=Flow_AreaOptimized_medium
```
```
◦ Implementation Strategy:
```
```
--vivado.prop
run.impl_1.strategy=Performance_ExtraTimingOpt
```
```
◦ Report Strategy: Can be specified for synthesis or
implementation runs.
```
```
--vivado.prop
run.synth_1.report_strategy=MyCustom_Reports
```
```
--vivado.prop run.impl_1.report_strategy={Timing
Closure Reports}
```
```
The command line is broken down as follows:
◦ --vivado.prop is the v++ command-line option to assign
properties to objects as described in --vivado Options.
◦ run.<run_name>.strategy=<strategy_name> to assign a
strategy property (or report_strategy) to the specified
synthesis or implementation run. Default run names are
synth_1 for synthesis or impl_1 for implementation.
◦ Strategy names with spaces in them, such as {Timing Closure
Reports} require braces or double-quotes to group the words as
shown above.
```
```
Tip: You can also specify multiple implementation strategies
to run as described in Running Multiple Implementation
Strategies for Timing Closure.
```
2. Pass Tcl scripts with custom design constraints or scripted
    operations.
    You can create Tcl scripts to assign XDC design constraints to objects
    in the design, and pass these Tcl scripts to the Vivado tools using the
    PRE and POST Tcl script properties of the synthesis and
    implementation steps. For more information on Tcl scripting, refer to


##### ★

##### ★

```
the Vivado Design Suite User Guide: Using Tcl Scripting (UG894).
While there is only one synthesis step, there are a number of
implementation steps as described in the Vivado Design Suite User
Guide: Implementation (UG904). You can assign Tcl scripts for the
Vivado tool to run before the step (PRE), or after the step (POST).
The specific steps you can assign Tcl scripts to include the following:
SYNTH_DESIGN, INIT_DESIGN, OPT_DESIGN, PLACE_DESIGN,
ROUTE_DESIGN, WRITE_BITSTREAM.
```
```
Tip: There are also some optional steps that can be enabled
using the --vivado.prop
run.impl_1.steps.phys_opt_design.is_enabled=1 option. When
enabled, these steps can also have Tcl PRE and POST scripts.
An example of the Tcl PRE and POST script assignments follow:
```
```
--vivado.prop run.impl_1.STEPS.PLACE_DESIGN.TCL.PRE=/.../
xxx.tcl
```
```
In the preceding example a script has been assigned to run before
the PLACE_DESIGN step. The command line is broken down as
follows:
◦ --vivado is the v++ command-line option to specify directives
for the Vivado tools.
◦ prop keyword to indicate you are passing a property setting.
◦ run. keyword to indicate that you are passing a run property.
◦ impl_1. indicates the name of the run.
◦ STEPS.PLACE_DESIGN.TCL.PRE indicates the run property you
are specifying.
◦ /.../xx.tcl indicates the property value.
```
```
Tip: Both the --advanced and --vivado options can be specified
on the v++ command line, or in a configuration file specified by the
--config option. The example above shows the command line use,
and the following example shows the config file usage. Refer to Vitis
Compiler Configuration File in the Vitis Reference Guide (UG1702)
for more information.
```
3. Setting properties on run, file, and fileset design objects.
    This is very similar to passing Tcl scripts as described above, but in
    this case you are passing values to different properties on multiple
    design objects. For example, to use a specific implementation


```
strategy such as Performance_Explore and disable global buffer
insertion during placement, you can define the properties as shown
below:
```
```
[vivado]
prop=run.impl_1.STEPS.OPT_DESIGN.ARGS.DIRECTIVE=Explore
prop=run.impl_1.STEPS.PLACE_DESIGN.ARGS.DIRECTIVE=Explore
prop=run.impl_1.{STEPS.PLACE_DESIGN.ARGS.MORE OPTIONS}={-
no_bufg_opt}
prop=run.impl_1.STEPS.PHYS_OPT_DESIGN.IS_ENABLED=true
prop=run.impl_1.STEPS.PHYS_OPT_DESIGN.ARGS.DIRECTIVE=Expl
ore
prop=run.impl_1.STEPS.ROUTE_DESIGN.ARGS.DIRECTIVE=Explore
```
```
In the example above, the Explore value is assigned to the
STEPS.XXX.DIRECTIVE property of various steps of the
implementation run. Note the syntax for defining these properties is:
```
```
<object>.<instance>.property=<value>
```
```
Where:
◦ <object> can be a design run, a file, or a fileset object.
◦ <instance> indicates a specific instance of the object.
◦ <property> specifies the property to assign.
◦ <value> defines the value of the property.
In this example the object is a run, the instance is the default
implementation run, impl_1, and the property is an argument of the
different step names, In this case the DIRECTIVE, IS_ENABLED, and
{MORE OPTIONS}. Refer to --vivado Optionsfor more information on
the command syntax.
```
4. Enabling optional steps in the Vivado implementation process.
    The build process runs Vivado synthesis and implementation to
    generate the device binary. Some of the implementation steps are
    enable and run as part of the default build process, and some of the
    implementation steps can be optionally enabled at your discretion.
    Optional steps can be listed using the --list_steps command, and
    include: vpl.impl.power_opt_design,
    vpl.impl.post_place_power_opt_design,
    vpl.impl.phys_opt_design, and


###### !!

```
vpl.impl.post_route_phys_opt_design.
An optional step can be enabled using the --vivado.prop option.
For example, to enable PHYS_OPT_DESIGN step, use the following
config file content:
```
```
[vivado]
prop=run.impl_1.steps.phys_opt_design.is_enabled=1
```
```
When an optional step is enabled as shown above, the step can be
specified as part of the -from_step/-to_step command as
described below in Running --to_step or --from_step , or enable a Tcl
script to run before or after the step as described in --linkhook
Options.
```
5. Passing parameters to the tool to control processing.
    The --vivado option also allows you to pass parameters to the
    Vivado tools. The parameters are used to configure the tool features
    or behavior prior to launching the tool. The syntax for specifying a
    parameter uses the following form:

```
--vivado.param <object><parameter>=<value>
```
```
The keyword param indicates that you are passing a parameter for
the Vivado tools, rather than a property for a design object. You
must also define the <object> it applies to, the <parameter> that
you are specifying, and the <value> to assign it.
The following example project indicates the current Vivado project,
writeIntermedateCheckpoints, is the parameter being passed and
the value is 1, which enables this boolean parameter.
```
```
--vivado.param project.writeIntermediateCheckpoints=1
```
6. Managing the reports generated during synthesis and
    implementation.
       **Important:** You must also specify --save-temps on the v++
    command line when customizing the reports generated by the
    Vivado tool to preserve the temporary files created during synthesis
    and implementation, including any generated reports.
    You might also want to generate or save more than the standard
    reports provided by the Vivado tools when run as part of the Vitis


tools build process. You can customize the reports generated using
the --advanced.misc option as follows:

```
[advanced]
misc=report=type report_utilization name
synth_report_utilization_summary steps {synth_design}
runs {__KERNEL__} options {}
misc=report=type report_timing_summary name
impl_report_timing_summary_init_design_summary steps
{init_design} runs {impl_1} options {-max_paths 10}
misc=report=type report_utilization name
impl_report_utilization_init_design_summary steps
{init_design} runs {impl_1} options {}
misc=report=type report_control_sets name
impl_report_control_sets_place_design_summary steps
{place_design} runs {impl_1} options {-verbose}
misc=report=type report_utilization name
impl_report_utilization_place_design_summary steps
{place_design} runs {impl_1} options {}
misc=report=type report_io name
impl_report_io_place_design_summary steps {place_design}
runs {impl_1} options {}
misc=report=type report_bus_skew name
impl_report_bus_skew_route_design_summary steps
{route_design} runs {impl_1} options {-warn_on_violation}
misc=report=type report_clock_utilization name
impl_report_clock_utilization_route_design_summary steps
{route_design} runs {impl_1} options {}
```
The syntax of the command line is explained using the following
example:

```
misc=report=type report_bus_skew name
impl_report_bus_skew_route_design_summary steps
{route_design} runs {impl_1} options {-warn_on_violation}
```
**misc=report=**


###### !!

```
Specifies the --advanced.misc option as described in --
advanced Options, and defines the report configuration for the
Vivado tool. The rest of the command line is specified in name/
value pairs, reflecting the options of the create_report_config
Tcl command as described in Vivado Design Suite Tcl Command
Reference Guide (UG835).
type report_bus_skew
Relates to the -report_type argument, and specifies the type
of the report as the report_bus_skew. Most of the report_* Tcl
commands can be specified as the report type.
name impl_report_bus_skew_route_design_summary
Relates to the -report_name argument, and specifies the name
of the report. Note this is not the file name of the report, and
generally this option can be skipped as the report names will be
auto-generated by the tool.
steps {route_design}
Relates to the -steps option, and specifies the synthesis and
implementation steps that the report applies to. The report can
be specified for use with multiple steps to have the report
regenerated at each step, in which case the name of the report
will be automatically defined.
runs {impl_1}
Relates to the -runs option, and specifies the name of the
design runs to apply the report to.
options {-warn_on_violation}
Specifies various options of the report_* Tcl command to be
used when generating the report. In this example, the -
warn_on_violation option is a feature of the report_bus_skew
command.
Important: There is no error checking to ensure the specified
options are correct and applicable to the report type specified. If
you indicate options that are incorrect the report will return an
error when it is run.
```
Associating an ELF File with MicroBlaze Processor

You can use the following steps to associate an ELF file with a


###### !!

MicroBlaze™ processor in your design. Associating the ELF file configures
a memory target, such as a set of Block RAMs. The information that is
needed for ELF file association includes:

- The location of the ELF file to be loaded
- The address space accessible via a master interface to a memory
    location, which the ELF file will be stored
- The mapped peripheral within that address space representing the
    memory where ELF file will be stored and from where it will be
    accessed at run time

**Important:** This flow requires you to have access to the level of
design hierarchy containing the MicroBlaze processor, and an existing
ELF file.

This process uses SCOPED_TO_REF and SCOPED_TO_CELLS properties on
the MicroBlaze processor itself, and not to the Block RAMs that are the
actual target of the ELF file data.
You can associate the ELF file to the MicroBlaze processor during the v++
--link process using the --advanced.param
<param_name>=<param_value> command as described in --advanced
Options. An example for a config file is shown below.

```
[advanced]
param=hw_emu.post_sim_settings=<file_path>/link.tcl
```
The link.tcl should add the ELF file to the Vivado Design Suite project,
exclude it for use in simulation, and associate it with the MicroBlaze
processor, as shown in the example below.

```
add_files <file_path>/executable.elf
set_property used_in_simulation 0 [get_files <file_path>/
executable.elf]
set_property SCOPED_TO_REF base_microblaze_design [get_files
-all \
-of_objects [get_fileset sources_1] {<file_path>/
executable.elf}]
set_property SCOPED_TO_CELLS { microblaze_0 } \
[get_files -all -of_objects [get_fileset sources_1]
{<file_path>/executable.elf}]
```

###### !!

This information will be used to generate a BMM file which will be used
by programs such as data2mem to generate a .mem file that will populate
the Block RAMs that are generated from the block_memory_generator.

Running Multiple Implementation Strategies for Timing Closure

For challenging designs, it can take multiple iterations of Vivado
implementation using multiple different strategies to achieve timing
closure. This topic shows you how to launch multiple implementation
strategies at the same time in the hardware build (-t hw), and how to
identify and use successful runs to generate the device binary and
complete the build.
As explained in --vivado Options, the --vivado.impl.strategies
command enables you to specify multiple strategies to run in a single
build pass. The command line would look as follows:

```
v++ --link -s -g -t hw --platform xilinx_zcu102_base_202010_1
-I. \
--vivado.impl.strategies "Performance_Explore,Area_Explore" -
o kernel.xclbin hello.xo
```
In the example above, the Performance_Explore and Area_Explore
strategies are run simultaneously in the Vivado build to see which returns
the best results. You can specify the ALL to have all available strategies
run within the tool.

**Important:** Running ALL implementation strategies might launch 30 or
more runs in the Vivado tool, including any user-defined strategies stored
in your home directory (~/.Xilinx/Vivado/202X.X/strategies). This can be a
tremendous drain on resources, and is not advised. You can prevent this
by defining specific strategies to run, and using a command queue to
distribute the process load in some managed way, such as through the
--vivado.impl.jobs or the --vivado.impl.lsf commands.

You can also determine this option in a configuration file in the following
form:

```
#Vivado Implementation Strategies
[vivado]
impl.strategies=Performance_Explore,Area_Explore
```

The Vitis compiler automatically picks the first completed run results that
meets timing to proceed with the build process and generate the device
binary. However, you can also direct the tool to wait for all runs to see
the result of all strategies. This would require the use of the {}
ALL_IMPL{} macro to apply custom settings to all runs and the
multiStrategiesWaitOnAllRuns directive to see the result of all
strategies:

```
[advanced]
#param=compiler.multiStrategiesWaitOnAllRuns=1
```
```
[vivado]
```
```
impl.strategies=ALL
prop=run.{}ALL_IMPL{}.STEPS.PLACE_DESIGN.TCL.PRE=../../
vpp_cfg/place_design_pre.tcl
prop=run.{}ALL_IMPL{}.STEPS.ROUTE_DESIGN.TCL.PRE=../../
vpp_cfg/route_design_pre.tcl
```
compiler.multiStrategiesWaitOnAllRuns=0 represents the default
behavior. If you want v++ to wait for all runs to complete, which will get
their report files, change that parameter value to 1. This includes an
overview of the implementation results, in addition to a Timing Summary
report. Seeing all results will give you an indication on how hard it is to
close timing for the tool for the all strategies that are allowed to run to
completion. You can use this feature to review the different strategies
and results.
You can also manually review the results of all implementation strategies
after they have completed. Then, use the results of any of the
implementation runs by using the --reuse_impl option as described in
Using --to_step and Launching Vivado Interactively.

Using --to_step and Launching Vivado Interactively

The Vitis compiler lets you stop the build process after completing a
specified step (--to_step), manually intervene in the design or files in
some way, and then continue the build by specifying a step the build
should resume from (--from_step). The --from_step directs the Vitis
compiler to resume compilation from the step where --to_step left off,


###### !!

##### ★

###### ✎

or some earlier step in the process. The --to_step and --from_step are
described in v++ Command in the _Vitis Reference Guide_ (UG1702).

**Important:** The --to_step and --from_step options are sequential
build options that require you to use the same project directory when
launching v++ --link --from_step as you specified when using v++ --
link --to_step.

The Vitis compiler also provides a --list_steps option to list the
available steps for the compilation or linking processes of a specific build
target. For example, the list of steps for the link process of the hardware
build can be found by:

```
v++ --list_steps --target hw --link
```
This command returns a number of steps, both default steps and optional
steps that the Vitis compiler goes through during the linking process of
the hardware build. Some of the default steps include: system_link, vpl,
vpl.create_project, vpl.create_bd, vpl.generate_target,
vpl.synth, vpl.impl.opt_design, vpl.impl.place_design,
vpl.impl.route_design, and vpl.impl.write_bitstream.
Optional steps include: vpl.impl.power_opt_design,
vpl.impl.post_place_power_opt_design,
vpl.impl.phys_opt_design, and
vpl.impl.post_route_phys_opt_design.

**Tip:** An optional step must be enabled before specifying it with --
from_step or --to_step as previously described in Using the --vivado
and --advanced Options.

#### Launching the Vivado IDE for Interactive Design

**Note:** The Packaging for Vitis Export to Vivado Flow in the _Embedded
Design Development Using Vitis_ (UG1701) is the preferred method.

With the --to_step command, you can launch the build process to
Vivado synthesis and then start the Vivado IDE on the project to
manually place and route the design. To perform this you would use the
following command syntax:


##### ★

###### !!

```
v++ --target hw --link --to_step vpl.synth --save-temps --
platform <PLATFORM_NAME> <XO_FILES>
```
**Tip:** As shown in the example above, you must also specify --save-
temps when using --to_step to preserve any temporary files created by
the build process.
This command specifies the link process of the hardware build, runs the
build through the synthesis step, and saves the temporary files produced
by the build process.
You can launch the Vivado tool directly on the project built by the Vitis
compiler using the --interactive command. This opens the Vivado
project found at <temp_dir>/link/vivado/vpl/prj in your build directory,
letting you interactively edit the design:

```
v++ --target hw --link --interactive impl --save-temps --
platform <PLATFORM_NAME> <XO_FILES>
```
When invoking the Vivado IDE in this mode, you can open the synthesis
or implementation runs to manage and modify the project. You can
change the run details as needed to close timing and try different
approaches to implementation. You can save the results to a design
checkpoint (DCP), or generate the project bitstream (.bit) to use in the
Vitis environment to generate the device binary.
After saving the DCP from within the Vivado IDE, close the tool and return
to the Vitis environment. Use the --reuse_impl option to apply a
previously implemented DCP file in the v++ command line to generate
the xclbin.

**Important:** The --reuse_impl option is an incremental build option
that requires you to apply the same project directory when resuming the
Vitis compiler with --reuse_impl that you specified when using --
to_step to start the build.

The following command completes the linking process by using the
specified DCP file from the Vivado tool to create the project.xclbin from
the input files.

```
v++ --link --platform <PLATFORM_NAME> -o'project.xclbin'
project.xo --reuse_impl ./_x/link/vivado/routed.dcp
```
You can also use a bitstream file generated by the Vivado tool to create


###### ✎

the project.xclbin:

```
v++ --link --platform <PLATFORM_NAME> -o'project.xclbin'
project.xo --reuse_bit ./_x/link/vivado/project.bit
```
**Note:** The project.bit used for --reuse_bit is a partial bit and not a
full bit.

#### Additional Vivado Options

Some additional switches that can be used in the v++ command line or
config file include the following:

- --export_script/--custom_script edit and use Tcl scripts to
    modify the compilation or linking process.
- --remote_ip_cache specify a remote IP cache directory for Vivado
    synthesis.
- --no_ip_cache turn off the IP cache for Vivado synthesis. This
    causes all IP to be re-synthesized as part of the build process,
    scrubbing out cached data.

### Integrating the System

This section describes how the hardware design and software
applications can be combined to form a complete integrated system. A
prerequisite for software application development is the information
about the hardware design and their corresponding address map, which
is the hardware specification. The software applications are compiled to
binaries, then packaged together with hardware configuration data into
device images, which also contain boot instructions.
The packaging process consists of two steps:

1. The first step involves generating loadable images from the Vivado
    hardware design and AI Engine handoffs into a binary container
    using Vitis packager. Also, in this step, a draft BIF file is created.
2. In the second step, the binary containers and software executables
    are collected and assembled into a delivery package which contains
    instructions for the boot process loading. This delivery package can
    be an SD card, QSPI flash image, or other similar type of package.


```
This second step can be performed either using Vitis packager or
Bootgen.
```
During the system integration packaging process, several different types
of files are used as listed in the following table:

**Table: System Integration Packaging Process Files**

```
Input File Type Name Description Applicable Devices
```
```
XSA Support
Archive
```
```
Primary design
handoff archive
file from
Vivado. The
XSA file can be
fixed or
extensible.
```
```
Versal adaptive
SoC / Zynq /
Zynq
UltraScale+
```
```
BIF Boot Image
Format
```
```
File that is used
by Bootgen to
determine how
to generate
boot images,
configure them
in a PDI file.
```
```
Versal adaptive
SoC / Zynq /
Zynq
UltraScale+
```
```
PDI Programmable
Device Image
```
```
Output of
Bootgen. It is
the image file
containing
bootloader,,
descriptions
and partitions
related to
processing
input data files
(ELF, PL
configuration
and other
binary files).
```
```
Versal adaptive
SoC
```

**Input File Type Name Description Applicable Devices**

XCLBIN XCLBIN Enhanced PDI
container with
metadata on PL
kernels and AI
Engine. XCLBIN
is exclusive to
XRT API usage
on the Linux
OS.

```
Versal adaptive
SoC / Zynq /
Zynq
UltraScale+
```
CDO Configuration
data objects

```
List of
commands
executed in
sequence to
configure
various
components in
the system.
```
```
Versal adaptive
SoC
```
libadf.a AI Engine
graph library

```
Versal adaptive
SoC
```

**Input File Type Name Description Applicable Devices**

- Output
    archive:
    libadf.a
    is the
    primary
    output of
    compiling
    an AI
    Engine
    graph. It
    contains
    the
    compiled
    program
    for the AI
    Engine. It
    includes
    both CDO
    and Elf
    files.
- CDOs
    (Compiled
    Device
    Objects):
    These
    objects
    define the
    setup and
    configuration
    of the AI
    Engine,
    including
    its
    resources
    and
    topology.


**Input File Type Name Description Applicable Devices**

- ELFs
    (Executable
    and
    Linkable
    Format):
    These files
    contain
    the
    program
    instructions
    that the AI
    Engine's
    tile
    processors
    execute.
- Partitions:
    When
    enabled,
    the AI
    Engine
    allows for
    dividing
    the
    workload
    into
    partitions.
    Each
    partition
    targets a
    subset of
    the
    columns of
    the AI
    Engine
    array and
    generates
    its own


**Input File Type Name Description Applicable Devices**

```
libadf
file.
```
- libadf files:
    These files
    contain
    the
    compiled
    program
    and
    configuration
    for each
    partition.
    All of these
    files must
    be
    specified
    when
    packaging
    a design
    containing
    partitions.

```
See Compiling
AI Engine
Graph for
Independent
Partitions in the
AI Engine Tools
and Flows User
Guide
(UG1076)
```
FSBL First stage
bootloader

```
Image for PL
bitstream, code
and data to
start the initial
design. This
```
```
Zynq / Zynq
UltraScale+
```

```
Input File Type Name Description Applicable Devices
can bring up
the entire
design or pass
on to a second
bootloader to
finalize the
boot.
```
```
DTSI Devicetree
system
includes
```
```
Adding user
settings to
override
devicetree
defaults like
MAC address,
UART baud
rates, etc on
hardware
devices.
```
```
Versal adaptive
SoC / Zynq /
Zynq
UltraScale+
```
```
qemu_args.txt /
pmc_args.txt
```
```
QEMU
command
arguments file
```
```
Command line
arguments
used when
launching
QEMU as the
DTB for
emulation
differs from
Linux device
tree.
```
```
Versal adaptive
SoC
```
The package process varies depending on the domain selected, as this
has an impact on the boot order and how the hardware specification is
extracted for the host application. The following sections discuss these
variants.
The first package step require using Vitis package, while the second step
can use either Vitis package or Bootgen. It's recommended to be familiar
with the boot components and how the BIF setup the boot order


###### ✎

###### !!

described in Software Platform. Advanced users should consider using
_Bootgen User Guide_ (UG1283) for details and custom packaging control
options. The sections below describe using the Vitis package.

**Note:** Packaging for HW emulation require the design to be linked
using --target hw_emu.

#### Packaging Process for Bare-metal and RTOS

#### Applications

Bare-metal applications interact with the hardware through registers
defined in the hardware specification and low level drivers. Additionally
drivers for frequently used services like ethernet, file handling, FPGA
management, etc., can added via BSP (board support package).
The hardware specification is extracted from the fixed XSA, and when
creating the Vitis platform component for bare metal or RTOS domains,
the xparameters.h file is generated. See Board Support Package
Settings Page in the _Vitis Unified Software Platform Documentation:
Embedded Software Development_ (UG1400) on how to add and configure
Bare-metal and RTOS domains. If changes to hardware affect the
hardware specification, the BSP and xparameters.h needs to be
regenerated.
Alternatively, PetaLinux tools for multiconfig can be used to regenerate
BSP: Building multiconfig Applications in the _PetaLinux Tools
Documentation: Reference Guide_ (UG1144).
First, generate a loadable PDI and extract the AI Engine CDO below.

```
v++ -p -s -f <fixed.xsa> <libadf.a> --temp-dir <temp_dir> --
save-temps
```
**Important:** If Bootgen is used for step 2, it is required to use --save-
temps option as shown above to preserve the files needed by Bootgen.

Next, integrate the hardware and software platform using Vitis packager:

```
v++ -p -s -f <fixed.xsa> <libadf.a> --
package.generate_sd_card --package.sd_file <pdi, elf, xclbin,
etc.> --package.sd_dir <outdir>
```
Full details on command line options for v++ are described in v++


Command in the _Vitis Reference Guide_ (UG1702).

#### Packaging Process for Linux Applications

There are two ways that Linux applications can use drivers: the standard
driver approach with the system device tree, or the XRT API.
With the XRT API, drivers executed in user space query address
information from the XCLBIN file. When design changes affect hardware
specification registers on Vitis managed components, the XCLBIN file is
automatically updated during linking and packaging.
Refer to Software Platform for system device tree drivers that need to be
regenerated using the fixed XSA.
User specific settings to the Linux drivers are adjusted through DTSI, see
Device Tree Configuration in the _PetaLinux Tools Documentation:
Reference Guide_ (UG1144).
Once the device tree and configurations are set up, packaging is done
similar to Bare-metal and RTOS packaging.

#### Packaging and Boot Configuration using Bootgen

After the first packaging step, the AMD Vitis™ packager collects and
assembles the binaries and executables to boot and run a design on AMD
SoC devices using the BIF file. Details on device specific boot and
configuration are available in the following user guides:

- Versal adaptive SoC: Boot and Configuration in the _Versal Adaptive_
    _SoC System Software Developers Guide_ (UG1304)
- Zynq UltraScale+: Validate NoC DRCs in the _Versal Adaptive SoC_
    _Hardware, IP, and Platform Development Methodology Guide_
    (UG1387)
- Zynq: Boot and Configuration in the _Versal Adaptive SoC System_
    _Software Developers Guide_ (UG1304)

#### Packaging Specifies for Versal Designs

The Versal AI Engine compiler generates output in the form of a library
file, libadf.a, which contains ELF and CDO files, as well as tool-specific
data and metadata, for hardware and hardware emulation flows. To


create a loadable image binary, this data must be combined with PL-
based configuration data, boot loaders, and other binaries. The Vitis
packager performs this function, combining information from libadf.a and
the Vitis linker generated XSA file.
For Versal adaptive SoCs, the programmable device image (PDI) file is
used to boot and program the hardware device. For hardware emulation
the --package command adds the PDI, EMULATION_DATA sections and
the XSA file, and outputs an XCLBIN file. For hardware builds, the
package process creates an XCLBIN file containing ELF files and graph
configuration data objects (CDOs) for the AI Engine application. The
XCLBIN file includes the following information:

**PDI**
Programming information for the AI Engine array

**Debug data**
Debug information when included in the build

**Memory topology**
Defines the memory resources and structure for the target platform

**IP layout**
Defines layout information for the implemented hardware design

**Metadata**
Various elements of platform meta data to let the tool load and run
the XCLBIN file on the target platform

###### Packaging for Vitis Flow

**Figure: Linking the System Design**


The Vitisv++ --package command generates SD card and other Flash
images required for booting the system, in addition to the .xclbin device
binary from the .xsa generated for Versal devices. The v++ --package
step, or -p option, packages the final system at the end of the v++
compile, link, and package process. As described in Packaging for Vitis
Export to Vivado Flow, this is a required step for all Versal platforms,
including AI Engine platforms, and embedded processor platforms.
A fixed .xsa can be used to create custom boot, and software platform
images as described in the _Versal Adaptive SoC System Software
Developers Guide_ (UG1304). However, the --package Options in the _Vitis
Reference Guide_ (UG1702) let you package your design and define
various files required for booting and configuring the AMD device for use
during emulation or in production systems. It collects the various
elements to create an SD card, or other means to program the device, to
define the operating system, and to load the application and kernel code.
In the Vitis unified IDE, the package process is automated and the tool
creates the required files based on the build target, platform, and OS.
However, in the command line flow, you must specify the Vitis packaging
command (v++ --package) with the correct options for the job.
After packaging the design the AMD Vitis™ compiler generates a v+
+.package_summary that includes the packaging command and log file.
The summary file can be viewed in Analysis view of the Vitis analyzer
alongside the compile, link, and run summaries as explained in Working


###### ✎

with the Analysis View (Vitis Analyzer) in the _Vitis Reference Guide_
(UG1702).

###### Packaging for Vitis Export to Vivado Flow

For AMD Zynq™ UltraScale+™ MPSoC and AMD Zynq™ 7000 embedded
platforms, the --package command line is shown below:

```
v++ --package -t [hw_emu | hw] --platform <platform>
input.xclbin [ -o output.xclbin ]
```
**Note:** If the output option (-o) is not specified, the tool creates an
output file with the default name of a.xclbin. It is recommended to always
specify an output name (e.g., -o binary_container_1.xclbin) for clarity and
consistency with documentation examples.

For Versal devices the v++ --link command creates an .xsa file instead
of the .xclbin file. In this case the .xsa must be provided to the package
process to generate the .xclbin file. The --package command line for
Versal devices is as follows:

```
v++ --package -t [hw_emu | hw] --platform <platform>
input.xsa [ -o output.xclbin ]
```
In the case of Versal platforms, the package process takes the .xsa file
generated during the v++ --link command, and also takes the libadf.a
file produced by the aiecompiler command and integrates it into the
output device binary.
The --package command has a range of options for use with the
different platforms and build targets supported by the Vitis tools. In the
Vitis IDE, the package process is automated and the tool creates the
required files as needed. However, in the command line flow, you must
specify the v++ --package command or add the [package] tag in the
config file with the right options for the job. The following is an example
command for hardware emulation:

```
v++ --package --config package.cfg ./aie_graph/libadf.a \
./project.xsa -o aie_graph.xclbin
```
Where, the --config package.cfg option specifies a configuration file
for the Vitis compiler with the various options specified for the package
process. The following is an example configuration file:


###### !!

##### ★

```
platform=xilinx_vck190_base_202520_1
target=hw_emu
save-temps=1
```
```
[package]
boot_mode=sd
out_dir=./emulation
rootfs=<downloaded_common_image>/rootfs.ext4
image_format=ext4
kernel_image=<downloaded_common_image>/Image
sd_file=host.exe
```
For hardware emulation, the command takes the .xclbin or .xsa file as
input, produces a script to launch emulation (launch_hw_emu.sh). To
specify the output folder, use the --package.out_dir option.
For Linux, and with fixed .xsa, it is required to add --
package.dtb=<path to system.dtb>, --package.bl31_elf=<path to
bl31.elf>, and --package.uboot=<path to linux u-boot.elf>.
Additional files required for running the application, such as data files
needed as input or to validate the application, or the xrt.ini file for
profiling and debug, must be included in the output files, and can be
transferred individually using the --package.sd_file option, or
transferred as a directory using the sd_dir option as explained in --
package Options in the _Vitis Reference Guide_ (UG1702).

**Important:** For Linux a boot recipe is mandatory to automate the boot
process. This is added by using --package.sd_file <path to
boot.scr>.

For hardware builds, the --package command creates an sd_card folder,
or the QSPI.img depending on the boot mode specified with the --
package.boot_mode option.

**Tip:** For bare metal ELF files running on PS cores, you should also add
the following option to the command line:

```
--package.ps_elf <elf>,<core>
```
The package command creates an output folder called sd_card, that
contains all of the files needed to run hardware emulation for the
application, modeling the boot process of an sd_card. For hardware


###### ✎

builds, it contains the files required for creating an SD card to boot the
device. The following is an example of the packaging output for hardware
emulation:

```
|-- BOOT_bh.bin //Boot header
|-- BOOT.BIN //Boot File
|-- boot_image.bif
|-- launch_hw_emu.sh //Hardware emulation launch script
|-- libadf //AIE emulation data folder
| `-- cfg
| |-- aie.control.config.json
| |-- aie.partial.aiecompile_summary
| |-- aie.shim.solution.aiesol
| |-- aie.sim.config.txt
| `-- aie.xpe
|-- plm.bin //PLM boot file
|-- pmc_args.txt //PMC command argument
specification file
|-- pmc_cdo.bin //PMC boot file
|-- qemu_args.txt //QEMU command argument
specification file
|-- sd_card
| |-- BOOT.BIN
| |-- boot.scr
| |-- aie_graph.xclbin
| |-- host.exe
| |-- Image
|-- sd_card.img
`-- sim //Vivado simulation folder
```
After creating the sd_card folder for the hardware build, copy the
contents to an SD card to create the boot image for your physical device.

**Note:** On Windows, you must use a third-party tool, such as Etcher, to
write on the SD card for use in booting the AMD device.

###### Packaging Segmented Configuration

The packaging process for segmented configuration has two steps. The
first step is for extracting the loading and configuration meta-data for PL
and AI Engine. The second step is for combining that with the PS and NoC
DDRMC boot and configuration. For Versal AI Edge Series Gen 2, Versal


###### ✎

###### ✎

Prime Series Gen 2, and Versal Premium Series Gen 2, segmented
configuration is mandatory and v++ --package automatically identifies it
require to create both artifacts. For first generation Versal devices, you
need to run v++ --package a second time to create a SD card.

**Note:** If EDF is used, the PDI and dtbo needs to be added to the EDF
wic image, see AMD Embedded Development Framework.

**Note:** Segmented configuration is primarily for separating the boot
process and loading the PL/AIE running Linux on hardware. For hardware
emulation, the boot and load is performed in one step, see Packaging the
System in HW Emulation.

The PL/AIE package require these inputs:

```
v++ -p -s \
-f <fixed_design>.xsa \
<graph_name>.libadf \
--package.out_dir <path_to_sdcard> \
-o <design_name>.xclbin
```
To create a SD card combining the boot artifacts, Linux, host application
and the prepared PL/AIE, run v++ package a second time using the
following inputs.

```
v++ -p -t hw \
-f <fixed_design>.xsa \
<graph_name>.libadf \
--package.generate_sdcard \
--package.defer_aie_run \
--package.dtb <vitis_workspace>/<pfm_name>/export/
<pfm_name>/sw/boot/system.dtb \
--package.image_format <ext4|fat32> \
--package.bl31_elf <path_to_linux>/bl31.elf \
--package.uboot <path_to_linux>/uboot.elf \
--package.sd_file <path_to_linux>/Image \
--package.sd_file <path_to_linux>/boot.scr \
--package.sd_file <host_app>.exe \
--package.sd_file <design_name>.xclbin \
--package.sd_file <path_to_sdcard>/<PL_PDI_NAME>.pdi \
--package.sd_file <path_to_sdcard>/<PL_PDI_NAME>.dtbo \
--package.out_dir <path_to_sdcard> \
-o <design_name>.xclbin
```

On hardware, load the second stage after the PS/NoC boot procedure
completes using the fpgautil tool.

```
fpgautil -b <PL_PDI_NAME>.pdi -o <PL_PDI_NAME>.dtbo
```
The PL.pdi represents the PL and AI Engine design. PL.dtbo represents
the device tree blob overlay containing the meta-data information on
how to configure and setup the PL. XRT queries the XCLBIN file for
controlling the PL and AIE graph when the host application executes.

###### Packaging for DFX Platforms

Vitis Dynamic Function eXchange (DFX) platforms contain a static region
and DFX regions.

**Static Region**
Hardened block that is loaded in system booting and is not
reconfigurable at runtime.

**DFX region**
A reconfigurable partition (RP) that is implemented by the Vivado IP
integrator block design container (BDC). This partition can be
reprogrammed repeatedly during runtime to dynamically trade out
one function, or reconfigurable module (RM), for another.

AMD provides base DFX platforms, such as
xilinx_zcu102_base_dfx_202520_1 and
xilinx_vck190_base_dfx_202520_1. These are single RP platforms,
meaning that it contains a static region and only one DFX region in which
RMs can be dynamically exchanged at runtime. In the case of
xilinx_vck190_base_dfx_202520_1, the RM contains an AI Engine array
and PL kernels. When loading the RM the entire AI Engine array and all PL
kernels are loaded at the same time. The following picture shows the
block design (BD) of the platform.

**Figure: Block Design of the Base DFX Platform**


The static partition contains the following:

- CIPS and PS NoC
- Clock, Reset, Interrupt, and AXI interconnections
- NoC Interface and DDR memory controller

The reconfigurable module (RM) to load into the DX region of the
xilinx_vck190_base_dfx_202520_1 platform contains an AI Engine
array and PL kernels that are linked together by the Vitis linker (v++ -l).
Refer to _Versal Adaptive SoC Hardware, IP, and Platform Development
Methodology Guide_ (UG1387). The following picture shows an example of
the RM.

**Figure: Block Design of the RM**

The RM, with interfaces to the static region, includes:

- Clock and Reset
- Inter-NoC Interface (INI) outputs to general memory
- INI input from PS to control AI Engine
- AXI Interface from PS to control PL kernels


The contents of the RM can include:

- AI Engine
- PL kernels
- AXI-NoC IP
- Clock and reset modules
- AXI Interconnect module
- ILA
- FIFO, Data Width Converter (DWC), and CDC modules

The v++ --compile and v++ --link commands are the same for both a
DFX platform and a non-DFX platform. However, the v++ --package
command is different for the DFX platform than the non-DFX platform,
and is different between the hardware and hardware emulation targets
as described below.

#### Hardware Packaging and Deployment

For a hw target the v++ --package command for the DFX platform
requires two steps:

1. The first step generates an xclbin file that contains the RM PDI that
    will be loaded at runtime.

```
v++ -p -t hw -f xilinx_vck190_base_dfx_202520_1
--package.defer_aie_run \
-o rm.xclbin \
${XSA} \
libadf.a
```
2. The second step packages the RM xclbin file (rm.xclbin) into the SD
    card that can be read by the host program.

```
v++ -p -t hw -f xilinx_vck190_base_dfx_202520_1
--package.rootfs ${ROOTFS}
--package.kernel_image ${IMAGE} \
--package.boot_mode=sd \
--package.image_format=ext4 \
--package.sd_dir data \
--package.sd_file ${HOST_EXE} \
```

###### ✎

```
--package.sd_file rm.xclbin
```
When running the hardware, after Linux has started type the following
command to load the RM:

```
cd /run/media/mmcblk0p1
./host.exe rm.xclbin
```
**Note:** If the XRT detects that the same XCLBIN has been downloaded
already, it will not download the XCLBIN again by default. Instead, it
loads metadata from the xclbin file only.

To make sure XCLBIN is downloaded and the device is clean every time,
set xrt.ini to enforce XCLBIN download to the device as follows:

1. Add following configuration to the xrt.ini file:

```
[Runtime]
force_program_xclbin=true
```
2. Add the xrt.ini file to the SD card during the package process: --
    package.sd_file xrt.ini

This ensures the XRT downloads the XCLBIN to device every time.

#### Hardware Emulation Packaging and Deployment

For hw_emu target the v++ --package command can be:

```
v++ -p -t hw_emu -f xilinx_vck190_base_dfx_202520_1 \
--package.defer_aie_run \
--package.rootfs ${ROOTFS} \
--package.kernel_image ${IMAGE} \
--package.boot_mode=sd \
--package.image_format=ext4 \
--package.sd_dir data \
--package.sd_file ${HOST_EXE} \
--package.sd_file emconfig.json \
-o rm.xclbin \
${XSA} \
libadf.a
```

###### !!

###### ✎

Launch the emulation with the following command:

```
./launch_hw_emu.sh
```
**Important:** In hardware emulation you can load the XCLBIN only once.
The RM cannot be reloaded.

In QEMU, following commands can be run:

```
cd /run/media/mmcblk0p1
export XCL_EMULATION_MODE=hw_emu
./host.exe rm.xclbin
```
To run multiple RMs on a DFX platform during hardware emulation you
must start and stop the emulator, and load each RM .xclbin for a
separate test run.

### Deploying and Running the System

This section describes deploying and running the system in the Hardware
Emulator or on the target device on a physical board.

###### Running an Application on the Board

The Linux application can be launched automatically at boot time. You
can also execute the control application on the command line. For bare
metal, the application is launched automatically during boot.
When launching, the packaging uses the defer running AIE at boot
option. See --package Options in the _Vitis Reference Guide_ (UG1702).
If you are using the common Linux components that are provided by
AMD, perform the following steps to run an application on the platform:

1. Write the sd_card.img generated by the Vitis compiler command v++
    --package to the SD card.
2. Boot the board.
3. Run the command cd /run/media/sd-mmcblk0p1/.
4. Run the application. The xclbin filename depends on whether you specified
    an output name with -o in the v++ package command. If no -o option was
    specified, the default name is a.xclbin. For example:
    - With output name specified: ./host.exe binary_container_1.xclbin ./data
    - With default name: ./host.exe a.xclbin ./data

The application uses Xilinx Runtime (XRT) to communicate with kernels.

```
Note: If the sd_card.img file has already been written to the SD card
```

###### ✎

and you are only updating the application, you can save time in the
debugging phase by copying all files from <Vitis System Project>/
Hardware/package/sd_card to a FAT32 partition on a SD card to replace
existing files. The Ext4 partition does not change in sd_card.img.

###### Handling Files and Images for SD Card

This section provides guidance for managing SD Card files and images.

#### Writing Images to the SD Card

You can use the Vitis Unified Software Platform accelerated flow to target
an embedded platform. initramfs uses Double Data Rate SDRAM (DDR
SDRAM) for file system storage; targeting an embedded platform
facilitates packaging and creates an SD image with RootFS as an EXT4
partition. The process limits the real usable DDR memory for Linux kernel
and applications when the file system size increases; it cannot retain
RootFS changes after reboot.

**Automated Method (Recommended):**

For this project, an automated SD card preparation script is provided:

```bash
cd /home/pelayo/work/vitis_workspace/system_project
./prepare_sd_card.sh
```

This script will:
- Automatically detect your SD card device
- Partition the card (1GB FAT32 boot + ext4 rootfs)
- Format both partitions
- Copy all boot files (BOOT.BIN, xclbin, kernel, etc.)
- Install the root filesystem
- Ensure correct file naming (binary_container_1.xclbin)
- Provide VCK190 setup instructions

**Manual Method:**

To write EXT4 RootFS to an SD Card manually:

1. Prepare an SD card binary image file with FAT32 partition for boot
    and EXT4 partition for RootFS.
2. Write SD card images to the SD card via a tool such as Etcher on
    Windows or the dd command on Linux.
       **Note:** Refer to AMDAnswer Record 73711 for detailed information
    about these tools.

There are various ways to prepare an SD card image. You can use the v+
+ package tool to generate it, or use an open source tool. The v++
package tool generated sd_card.img has two partitions:

**FAT32 Partition**
1 GB size, initialized with the kernel image provided by common
Linux components.

**EXT4 Partition**
2 GB size, initialized with RootFS provided by common Linux
components.

To make the pre-built SD card image boot, you must copy the following


###### ✎

###### ✎

boot components to the FAT32 partition:

- pre-built/BOOT.BIN
- boot.scr from the <VITIS_INSTALL_DIR>/base_platform/
    <PLATFORM_NAME>/sw/xrt/image directory
- bl31.elf, u-boot.elf, and system.dtb from the <VITIS_INSTALL_DIR>/
    base_platform/<PLATFORM_NAME>/sw/boot directory
- A platform_desc.txt file containing the name of the platform must
    also be added to the FAT32 partition.

The pre-built SD card image can be used for evaluation usage and by
Windows users. It does not require Vitis or PetaLinux to be installed.

**Note:** The v++ --package with Ext4 partition is not supported on
Windows.

For hands on examples, review the packaging step in the tutorial
available on GitHub: https://github.com/Xilinx/Vitis-Tutorials/tree/HEAD/
Vitis_System_Design/Design_Tutorials/02-Versal_Vitis_Subsystem_Flow.

#### Using FAT32 Formatted SD Card

For Windows OS, there is no native support for EXT4 formats, so Windows
can only access SD Card with FAT32 partitions. In this case, you can
format a single primary partition to FAT32 and copy the files from the
generated output to the sd_card folder.

**Note:** The BOOT partition using FAT32 is currently limited to a
maximum size of 16 GB.

### Incremental Design Management

Constructing the project in a single, monolithic build process as described
in this chapter so far does not account for the iterative nature of practical
design scenarios. As a project grows in complexity, managing changes
and updates to individual components becomes essential. By adopting
an incremental build strategy, you can significantly reduce the project
build time by focusing only on the components that are directly affected
by a change. This section explains the methods of managing design
changes, allowing you to identify the specific steps that require
rebuilding, streamlining the development process and optimizing project


workflows.

#### Understanding the Build Order

To effectively manage the build process, it is crucial to understand the
order in which various tools and steps are executed. The Vitis Integrated
Flow and Vitis Export to Vivado Flow, provide a high-level overview of the
build sequence. Next, it is important to consider the file-level handoffs
between Vivado and Vitis flows. The following diagram serves as a
starting point for the discussion, illustrating the key components involved
in this process.

**Figure: System Level Dependencies and Build Handoffs**

The diagram uses boxes to represent activities and arrows to depict the
flow of inputs and outputs between them. The activities are governed by
a range of settings which can be specified through the IDE GUI,
command line options, or a configuration file. Typically these are defined
using tcl, python scripts, Makefiles, and configuration text files. In
addition to the settings, the build process also relies on source files


###### ✎

representing the design objects.
While it can seem intuitive that any step following an activity would
necessitate a rebuild, there are nuanced distinctions to consider. The
following table highlights some scenarios to shed light on what triggers a
rebuild.

#### Rebuild Strategies

This section lists common scenarios encountered during incremental
design evolution. The scenarios are categorized from both hardware and
software perspectives. For hardware changes, the software might need to
be rebuilt, because software is often used to control and monitor
hardware devices. Conversely, pure software changes allow for
significant shortcuts as it only involves simple recompile and
repackaging, leading to shorter iteration cycles.

**Table: Use Case Scenarios for Hardware Rebuild Strategy**

```
Scenario Action Comments
```
```
Initial build Build/Compile all
steps according to
Vitis Integrated Flow
or Vitis Export to
Vivado Flow.
```
```
When using the
"part" selection
method for the
device, all activities
that generate inputs
for the Vitis linker can
be compiled in any
order. This flexibility
allows for parallel
processing and
potentially faster
build times.
Note: For Vitis
Integrated Flow
projects with XRT
host applications, the
extensible XSA can
be used to setup
Linux OS. This allows
```

###### !!

```
Scenario Action Comments
concurrent
development of
software applications
alongside the AIE and
PL kernels.
Important: While
incremental builds
offer significant
benefits, it's crucial
to perform regular
full builds as part of
regression testing.
This ensures the
design's integrity and
readiness for reuse or
branching into other
projects.
```
Modify Vivado RTL or
IP integrator blocks
not related to the
Vitis region.

1. Open the Vivado
    project
    containing the
    imported VMA.
2. Edit the block
    design, validate
    and save
3. Reset synthesis
    and
    implementation
    runs, and then
    rerun them as
    performed in
    initial design
    iteration.
4. Rebuild/
    recompile all
    steps following

```
This scenario
involves modifying
Vivado RTL or IP
integrator blocks that
are not directly
related to the Vitis
region, assuming
these changes do not
impact the AI Engine
or Vitis region itself.
Typical use cases
include modifying
synthesis and
implementation
strategies,
addressing
engineering change
orders (ECOs), or
adding additional
```

```
Scenario Action Comments
the Vivado
synthesis and
implementation
step.
```
```
debug ILAs.
```
Modify Vivado RTL or
IP integrator blocks
connected to Vitis
region

1. Open the Vivado
    project
    containing the
    imported VMA.
2. Disconnect and
    remove the Vitis
    region with
    vitis::remove_archive_hierarchy
3. Update the block
    design, validate
    and write a new
    extensible xsa.
4. Update and
    recompile
    affected PL
    kernels or AIE
    graph.
5. Update the link
    instructions and
    run Vitis linker.
6. Rebuild/
    recompile all
    steps
    subsequent to
    Vitis linker.

```
This scenario
involves
modifications to
Vivado RTL or IP
integrator blocks that
have connections to
the Vitis region.
Typical use cases
include implementing
incremental design
strategies or merging
ECOs into the base
design.
```
Modify RTL or HLS PL
kernels 1. Open the Vivado
project
containing the
imported VMA.

```
This scenario
addresses changes to
RTL or HLS IP kernels
within the Vitis
region, assuming
```

###### ✎

##### ★

```
Scenario Action Comments
```
2. Disconnect and
    remove the Vitis
    region with
    vitis::remove_archive_hierarchy
3. Update and
    recompile
    affected PL
    kernels.
4. Update the link
    instructions and
    run Vitis linker.
5. Rebuild/
    recompile all
    steps
    subsequent to
    Vitis linker.

```
these modifications
do not impact the
physical interfaces
to/from the AI Engine.
Note: If you adjust
the number of
instances of an
existing kernel XO,
step 3 can be
skipped.
```
Modify AI Engine
graph 1. Open the Vivado
project
containing the
imported VMA.

2. Disconnect and
    remove the Vitis
    region with
    vitis::remove_archive_hierarchy
3. Update and
    recompile AI
    Engine graph as
    required.
4. Update and
    recompile any
    affected PL
    kernels.
5. Update the
    linking
    instructions and

```
Tip: If the
interface protocols
between AI Engine
and PL or GMIO
remain unchanged,
recompiling the
graph can be treated
as a software update.
This allows for
skipping all steps
except recompiling
SW host application
and rerunning Vitis
package and Flash
package steps.
```

```
Scenario Action Comments
run Vitis linking.
```
6. Rebuild/
    recompile all
    steps
    subsequent to
    Vitis linker.

```
Modify VSS
components 1. Follow the
methods
presented in
previous
scenarios to
update and
recompile PL
kernels and AI
Engine graph.
```
2. Update and
    recompile the
    affected VSS
    components.
3. Follow the
    methods
    presented in
    previous
    scenarios to
    rebuild/
    recompile with
    Vitis linker.

```
Changes to VSS
follow the same
principle as listed in
the previous
scenarios with the
addition of
recompiling the VSS
components and
repeating the steps
mentioned in the
respective scenarios.
```
**Table: Use Case Scenarios for Software Rebuild Strategy**

```
Scenario Action Comments
```
```
Modify host software
applications 1. Modify the
software source
```
```
This assumes no new
hardware changes
are made to the
```

###### ✎

```
Scenario Action Comments
code and
recompile to
executable.
```
2. Rerun Vitis flash
    package.

```
design.
Note: The new
executable can
alternatively be
downloaded directly
to the boards Linux
file system using FTP
or secure copy. To
ensure the new file is
properly saved, use
the poweroff or
reboot command to
synchronize the file
system.
```
```
Modify device drivers
```
1. Modify the
    device tree
    system include.
2. Rebuild Vitis
    platform
    component to
    update system
    device tree and
    board support
    packages.
3. Update and
    recompile
    affected SW host
    applications
4. Rerun Vitis
    package steps.

Key takeaways:


- By analyzing these scenarios and the dependencies between the
    flow steps, you can leverage concurrent development strategies to
    streamline the design process.
- When multiple designers collaborate in developing a design, it is
    crucial to ensure the ability to rebuild the design from scratch to a
    known state.
- A clear understanding of the merge points and opportunities for
    splitting activities across a team is essential for efficient
    collaboration.


