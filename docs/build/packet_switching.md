###### Confidential - Copyright © Fluid Topics


###### Displayed in the footer

#### Packet Switching

###### Packet Stream Based AI Engine Kernels

###### Buffer-Based AI Engine Kernels

###### Buffer-Based AI Engine Kernels with Mixed Data Types


###### Displayed in the footer

# Packet Switching

##### Version: Vitis 2024.

##### AI Engine kernels can share a single processor and execute in an interleaved

##### manner. In addition, they can also share multiple stream connections on a single

##### physical channel. The explicit packet switching feature allows fine-grain control over

##### how packets are generated, distributed, and consumed in a graph computation.

##### Explicit packet switching is typically recommended in cases where many low

##### bandwidth streams from a common programmable logic (PL) source can be

##### distributed to different AI Engine destinations. Similarly many low bandwidth

##### streams from different AI Engine sources to a common PL destination can also take

##### advantage of this feature. Because a single physical channel is shared between

##### multiple streams, you minimize the number of AI Engine - PL interface streams

##### used.

##### This tutorial walks you through the steps to create buffer interface AI Engine kernels

##### that share the same AI Engine - PL interface streams (step 1), to create designs

##### with float and cint16 data types (step 2), and to create packet stream interface AI

##### Engine kernels that share AI Engine - PL interface streams (step 3). The AI Engine

##### kernels are different in these steps. In particular, packet stream interfaces and

##### associated built-in functions are introduced in step 3. The graph construction is

##### covered in detail in step 1 and step 3 (only with differences). All these designs

##### share the same PL kernels that are introduced in step 1. The PS codes for these

##### steps are similar; this is covered in detail in step 1 and step 2 (with differences).

##### There is a limitation in the current version of the AI Engine tools that only integer

##### format values are supported in data files for the AI Engine simulator. Details on the

##### data format and how to convert data from other types to integer format are covered

##### in steps 1 and 2.

##### IMPORTANT : Before beginning the tutorial make sure you have installed

##### the AMD Vitis™ software platform 2024.1. The Vitis release includes all

##### the embedded base platforms including the VCK190 base platform that is

##### used in this tutorial. In addition, ensure that you have downloaded the

##### Common Images for Embedded Vitis Platforms from this link.

##### The ‘common image’ package contains a prebuilt Linux kernel and root file system

##### that can be used with the AMD Versal™ board for embedded design development

##### using the Vitis tools.


###### Displayed in the footer

##### Before starting this tutorial, run the following steps:

##### 1. Go to the directory where you have unzipped the Versal Common Image

##### package.

##### 2. In a Bash shell, run the /**Common Images Dir**/xilinx-versal-common-

##### v2024.1/environment-setup-cortexa72-cortexa53-xilinx-linux

##### script. This script sets up the SDKTARGETSYSROOT and CXX variables. If

##### the script is not present, you must run the /**Common Images

##### Dir**/xilinx-versal-common-v2024.1/sdk.sh.

##### 3. Set up your ROOTFS and IMAGE to point to the rootfs.ext4 and Image files

##### located in the /**Common Images Dir**/xilinx-versal-common-v2024.

##### directory.

##### 4. Set up your PLATFORM_REPO_PATHS environment variable to

##### $XILINX_VITIS/base_platforms.

##### This tutorial targets VCK190 production board for 2024.1 version.

### Objectives

##### After completing this tutorial, you will be able to:

##### Construct a packet switching graph

##### Understand the packet format for packet switching

##### Write an AI Engine kernel that handles packet stream explicitly

##### Design PL kernels for packet switching

##### Write a PS application for hardware and hardware emulation flows

### Steps

##### Step 1 : Construct an explicit packet switching graph with buffer interface AI Engine

##### kernels. PL kernels and PS code for the system design are also introduced. See

##### details in Buffer-Based AI Engine Kernels.

##### Step 2 : Special consideration on float and cint* data types. See details in Buffer-

##### Based AI Engine Kernels with Mixed Data Types.

##### Step 3 : Introduce the packet stream interface and built-in functions for AI Engine

##### kernels, which allows you to fine control how packets are decoded and constructed.

##### See details in Packet Stream-Based AI Engine Kernels.

##### Note: In this tutorial, a Makefile and instructions are provided.


###### Displayed in the footer

##### Hint: In this tutorial, the designs are self-contained in each step, but the

##### steps refer to previous steps. Therefore, it is highly recommended to start

##### from the beginning and progress to completion.

#### Support

##### GitHub issues will be used for tracking requests and bugs. For questions go to

##### forums.

```
Copyright © 2020–2024 Advanced Micro Devices, Inc
Terms and Conditions
```
## Packet Stream Based AI Engine Kernels

##### Packet stream-based AI Engine kernels allow fine-grain control over how packets

##### are generated and consumed in the kernels. This section explains how to code AI

##### Engine kernels with packet stream interfaces (input_pktstream and

##### output_pktstream). The connection in the graph is also described.

##### The PL side and PS side of this example is the same as Buffer Based AI Engine

##### Kernels. Refer to:

##### Packet Format

##### Example PL Kernels for Packet Switching

##### Example PS code for Packet Switching

### Packet Stream Interfaces and Operations

##### Two stream types are provided to denote streaming data, consisting of packetized

##### interleaving of several different streams. These types are listed in the following

##### table.

##### Input Stream Types Output Stream Types

##### input_pktstream output_pktstream

##### A data packet consists of a one word (32-bit) packet header, followed by some

##### number of data words where the last data word has the TLAST field denoting the

##### end-of-packet. The following operations are used to read and advance input packet

##### streams and write and advance output packet streams.


###### Displayed in the footer

int32 readincr(input_pktstream *w);
int32 readincr(input_pktstream *w, bool &tlast);
void writeincr(output_pktstream *w, int32 value);
void writeincr(output_pktstream *w, int32 value, bool tlast);

##### The API with the TLAST argument reads or writes the end-of-packet condition, if the

##### packet size is not fixed.

##### The AI Engine tools provide the built-in function writeHeader to generate a packet

##### header for packets originating from the AI Engine kernel and writes them to the

##### output.

void writeHeader(output_pktstream *str, unsigned int pcktType, unsigned int ID);
void writeHeader(output_pktstream *str, unsigned int pcktType, unsigned int ID, bool tlast);

##### The AI Engine tools also provide the built-in function getPacketid to get the packet

##### ID for the packet stream interface. The index for getPacketid only applies if the

##### packet stream feeds into a pktsplit. In this example, each AI Engine kernel output

##### sees only one logical stream (0 for index).

uint32_t getPacketid(input_pktstream * in, int index);
uint32_t getPacketid(output_pktstream * out, int index);

##### Change working directory to pktstream_aie. Review the AI Engine kernels

##### (aie/aie_core1.cpp, ... , aie/aie_core4.cpp). The code for aie_core

##### (aie/aie_core1.cpp) is as follows.

```
void aie_core1(input_pktstream *in,output_pktstream *out){
readincr(in);//read header and discard
uint32 ID=getPacketid(out, 0 );//for output pktstream
writeHeader(out,pktType,ID); //Generate header for output
```
```
bool tlast;
for(int i= 0 ;i< 8 ;i++){
int32 tmp=readincr(in,tlast);
tmp+= 1 ;
writeincr(out,tmp,i== 7 );//TLAST=1 for last word
}
}
```
##### It can be seen that the input packet header is discarded. The output header is

##### generated by writeHeader, and the packet ID for the header is obtained by

##### getPacketid. TLAST equals 1 for the last word in the packet.

#### Construct Graph for Packet Stream Kernels

##### Review how the graph is constructed in aie/graph.h.

```
using namespace adf;
class mygraph: public adf::graph {
private:
adf:: kernel core[ 4 ];
```
```
adf:: pktsplit< 4 > sp;
adf:: pktmerge< 4 > mg;
```

###### Displayed in the footer

```
public:
adf::input_plio in;
adf::output_plio out;
mygraph() {
core[ 0 ] = adf::kernel::create(aie_core1);
core[ 1 ] = adf::kernel::create(aie_core2);
core[ 2 ] = adf::kernel::create(aie_core3);
core[ 3 ] = adf::kernel::create(aie_core4);
adf::source(core[ 0 ]) = "aie_core1.cpp";
adf::source(core[ 1 ]) = "aie_core2.cpp";
adf::source(core[ 2 ]) = "aie_core3.cpp";
adf::source(core[ 3 ]) = "aie_core4.cpp";
```
```
in=input_plio::create("Datain0", plio_32_bits, "data/input.txt");
out=output_plio::create("Dataout0", plio_32_bits, "data/output.txt");
```
```
sp = adf::pktsplit< 4 >::create();
mg = adf::pktmerge< 4 >::create();
for(int i= 0 ;i< 4 ;i++){
adf::runtime<ratio>(core[i]) = 0.9;
adf::connect<adf::pktstream > (sp.out[i], core[i].in[ 0 ]);
adf::connect<adf::pktstream > (core[i].out[ 0 ], mg.in[i]);
}
```
```
adf::connect<adf::pktstream> (in.out[ 0 ], sp.in[ 0 ]);
adf::connect<adf::pktstream> (mg.out[ 0 ], out.in[ 0 ]);
}
};
```
##### Note that the connection type for the input_pktstream and output_pktstream

##### interfaces are adf::pktstream. So, it uses adf::connect<adf::pktstream> to

##### connect the AI Engine kernel and pktsplit.out / pktmerge.in.

##### Note that input_pktstream is read as integer input. It needs to be

##### reinterpret_cast to other types, if needed.

#### Run the AI Engine Simulator, HW Emulation, and HW Flows

##### 1. Run the AI Engine simulator with the following make command.

```
make aiesim
```
##### 2. Run HW emulation with the following make command (it will build the HW

##### system and host application) :

```
make run_hw_emu
```
##### Tip: If the keyboard is accidentally hit and stops the system booting

##### automatically, type boot at the Versal> prompt to resume the

##### system booting.

##### 3. After Linux has booted, run the following commands at the Linux prompt (this

##### is only for HW cosim).

```
mount /dev/mmcblk0p1 /mnt
cd /mnt
```

###### Displayed in the footer

```
export XILINX_XRT=/usr
export XCL_EMULATION_MODE=hw_emu
./host.exe a.xclbin
```
##### To exit QEMU press Ctrl+A, x.

##### 4. To run in hardware, first build the system and application using the following

##### make command.

```
make package TARGET=hw
```
##### 5. After Linux has booted, run the following commands at the Linux prompt.

```
mount /dev/mmcblk0p1 /mnt
cd /mnt
export XILINX_XRT=/usr
./host.exe a.xclbin
```
##### The host code is self-checking. It checks the correctness of output data. If the

##### output data is correct, after the run has completed, it will print:

TEST PASSED

### Conclusion

##### In this tutorial you learned about:

##### Building the buffer interface or packet stream interface to AI Engine kernels

##### Constructing the packet switching graph

##### Writing PL kernels to perform packet switching

#### Support

##### GitHub issues will be used for tracking requests and bugs. For questions go to

##### forums.

```
Copyright © 2020–2024 Advanced Micro Devices, Inc
Terms and Conditions
```
## Buffer-Based AI Engine Kernels

##### This example shows you how to construct a graph with packet switching capability.

##### In the first section, Construct Graph with Packet Switching Capability, the example

##### graph features:


###### Displayed in the footer

##### Four parallel AI Engine kernels, with all four kernels sharing the same input

##### and output ports to the PL.

##### AI Engine kernels using buffer interfaces, which means they are agnostic

##### about how data is communicated to the PL.

##### This section introduces two new templated node classes, pktsplit<n> and

##### pktmerge<n>, to construct the graph. These classes switch the packet to the

##### correct destination and construct the packet with the corresponding packet IDs,

##### respectively.

##### Then this example:

##### Introduces the Packet Format and how to Prepare Data and Run AI Engine

##### Simulator.

##### Introduces a system design that includes Example PL Kernels for Packet

##### Switching. In the example PL kernels, you can see how a packet is

##### constructed and how the packet ID generated by the AI Engine compiler is

##### used.

##### Shows PS code for the system design in Example PS code for Packet

##### Switching.

##### Shows how to Run Hardware Emulation and Hardware flows.

### Construct Graph with Packet Switching Capability

##### To explicitly control the multiplexing and de-multiplexing of packets, two new

##### templated node classes are added to the ADF graph library: pktsplit<n> and

##### pktmerge<n>. A node instance of class pktmerge<n> is a n:1 multiplexer of n

##### packet streams producing a single packet stream. A node instance of class

##### pktsplit<n> is a 1:n de-multiplexer of a packet stream producing n different

##### packet streams.

##### Note: The maximum number of allowable packet streams is thirty-two on

##### a single physical channel ( n≤32 ).

##### The data from the PLIO is first connected to the pktsplit<n> instance, which splits

##### the packet depending on the packet ID. It automatically discards the packet header

##### and fills the buffer input buffers. It automatically discards the TLAST signal of the

##### packet when the buffer data is fully filled.

##### Each AI Engine kernel works similarly to a non-packet switching kernel. The output

##### data is merged by the pktmerge<n> instance, which automatically inserts the

##### packet headers with packet IDs, and TLAST for the last data of the packet.


###### Displayed in the footer

##### Change the working directory to buffer_aie. The example graph code is in

##### aie/graph.h, shown as follows.

```
class mygraph: public adf::graph {
private:
adf:: kernel core[ 4 ];
```
```
adf:: pktsplit< 4 > sp;
adf:: pktmerge< 4 > mg;
public:
adf::input_plio in;
adf::output_plio out;
mygraph() {
core[ 0 ] = adf::kernel::create(aie_core1);
core[ 1 ] = adf::kernel::create(aie_core2);
core[ 2 ] = adf::kernel::create(aie_core3);
core[ 3 ] = adf::kernel::create(aie_core4);
adf::source(core[ 0 ]) = "aie_core1.cpp";
adf::source(core[ 1 ]) = "aie_core2.cpp";
adf::source(core[ 2 ]) = "aie_core3.cpp";
adf::source(core[ 3 ]) = "aie_core4.cpp";
```
```
in=adf::input_plio::create("Datain0", plio_32_bits, "data/input.txt");
out=adf::output_plio::create("Dataout0", plio_32_bits, "data/output.txt");
```
```
sp = adf::pktsplit< 4 >::create();
mg = adf::pktmerge< 4 >::create();
for(int i= 0 ;i< 4 ;i++){
adf::runtime<ratio>(core[i]) = 0.9;
adf::connect<> (sp.out[i], core[i].in[ 0 ]);
adf::connect<> (core[i].out[ 0 ], mg.in[i]);
}
adf::connect<adf::pktstream> (in.out[ 0 ], sp.in[ 0 ]);
adf::connect<adf::pktstream> (mg.out[ 0 ], out.in[ 0 ]);
}
};
```
##### This is a graph with a 1:4 splitter pktsplit<4> and 1:1 merger pktmerge<4>. Note

##### that the connection type for pktsplit and pktmerge is adf::pktstream. The input

##### port in is first connected to the pktsplit, and pktsplit switches the packets to

##### different AI Engine kernels. The outputs of AI Engine kernels are connected to the

##### pktmerge, and pktmerge generates packet headers for those packets

##### automatically and outputs them through output port, out.

##### Run the make command make aie to compile the graph. Then open the compiled

##### summary with the AMD Vitis™ analyzer using the command vitis_analyzer

##### ./Work/graph.aiecompile_summary. Then click the Graph tab in the Vitis

##### analyzer. The graph of the design is shown as follows.


###### Displayed in the footer

##### It is seen that every sp output has been assigned a unique packet ID. Also, every

##### mg input has been assigned a unique ID. The packet IDs can vary on different

##### implementations. The AI Engine compiler generates a JSON file that contains all

##### the packet ID infomation Work/reports/packet_switching_report.json. It also

##### generates header files that define unique macro variables for the packet IDs. These

##### files are Work/temp/packet_ids_c.h and Work/temp/packet_ids_v.h, which

##### can be directly included in the C or Verilog source code.

##### For example, in this test case, the Work/temp/packet_ids_c.h file is as follows.

#define Datain0_0 0
#define Datain0_1 1
#define Datain0_2 2
#define Datain0_3 3
#define Dataout0_0 3
#define Dataout0_1 2
#define Dataout0_2 1
#define Dataout0_3 0

##### The macro names Datain0_0, ..., Dataout0_3 do not change between different

##### compilations. You can see how these macros are used in the PL kernels in this test

##### case in a later section.

### Packet Format

##### The first 32-bit word of a packet must always be a packet header which encodes

##### several bit fields, as shown in the following table.

##### Bit Note

##### 4-0 Packet ID


###### Displayed in the footer

##### Bit Note

##### 11-5 7'b

##### 14-12 Packet Type

##### 15 1'b

##### 20-16 Source Row

##### 27-21 Source Column

##### 30-28 3'b

##### 31 Odd parity of bits[30:0]

##### The packet ID in the header should match the ID assigned by the compiler. The

##### packet type can be any 3-bit pattern that you want to insert to identify the type of

##### packet. The source row and column denote the AI Engine tile coordinates from

##### where the packet originated. By convention, source row and column for packets

##### originating in the PL is -1,-1.

##### The last 32-bit word should have its TLAST set High. Other words should set their

##### TLAST Low.

##### When the packet originates from the PL, the packet header should be constructed

##### by the PL kernels manually. When the AI Engine receives the packet, it is decoded

##### and routed to the destination corresponding to the packet ID in the header.

##### When the packet originates from the AI Engine, the first 32-bit word should be

##### decoded by the PL kernels manually. By decoding the packet ID from the packet,

##### and reading the packet switching header files (Work/temp/packet_ids_c.h and

##### Work/temp/packet_ids_v.h) by the compiler, the PL kernels should be able to

##### route the packet to the correct destination.

### Prepare Data and Run AI Engine Simulator

##### When constructing the input data file for the AI Engine simulator, the data file should

##### contain the sequence of packets. Each packet contains the packet header, followed

##### by the data. The last data has the TLAST keyword in a separate line just above the

##### data. The data is in 32-bit integer format, including the header. The following is an

##### example of a packet in the data file (data/input.txt).

2415853568
0


###### Displayed in the footer

1 2 3 4 5 6

TLAST
7

##### In this packet, 2415853568 is an integer. The hex value for 2415853568 is

##### 0x8FFF0000, which has packet ID of 0 (the last five bits). It is useful for you to have

##### your own program to convert the original data into the data file with packet headers

##### in the required format.

##### When the input PLIO is not 32 bits wide, it can include multiple 32-bit integers in a

##### line to construct wider bit words, with spaces between them. For example, the

##### following is an example packet for a 64-bit width PLIO.

2415853568 0
1 2
3 4
5 6
TLAST
7

##### Run the AI Engine simulator with following make command. The detailed information

##### for the AI Engine compiler and AI Engine simulator commands can be found in the

##### AI Engine Documentation.

make aiesim

##### The output data is in aiesimulator_output/data/output.txt. The output data

##### is also arranged as successive packets, for example:

T 413 ns
50462720
T 416 ns
4
T 417 ns
8
T 418 ns
12
T 419 ns
16
T 420 ns
20
T 421 ns
24
T 422 ns
28
T 423 ns
TLAST
32

##### The packet header is the first 32-bit word 50462720. Its hex value is 0x3020000.

##### Therefore, the packet ID is 0 (the last 5 bits). You can look at the packet switching

##### header files (Work/temp/packet_ids_c.h and Work/temp/packet_ids_v.h) to


###### Displayed in the footer

##### find out which AI Engine kernel has produced it. The Work/temp/packet_ids_c.h

##### has defined:

#define Dataout0_3 0

##### Here, Dataout0_ 3 denotes that the packet ID 0 comes from pktmerge.in[3]. By

##### looking at the graph code (aie/graph.h) or graph view in Vitis analyzer, you can

##### find which AI Engine kernel actually produced it. In this example result, it is kernel

##### core[3] (aie/aie_core4.cpp).

#### Example PL Kernels for Packet Switching

##### This section describes how the PL kernels can generate and decode packet

##### headers, and how to distribute packets to the corresponding destinations. HLS

##### example code is provided, and hardware emulation and hardware flows can be run.

##### The packet switching feature does not have a dependency on the PL kernel types

##### (HLS, Verilog, etc) and their design structure. It just has requirements around the

##### packet format and how the packet ID works as described in the previous sections.

##### The system design structure of the example is as shown in the following image.

##### The previous section introduced the AI Engine side. It receives packets from one

##### PLIO (AXI4-Stream interface), and distributes the packets to different AI Engine

##### kernels. Then all AI Engine outputs are packed with packet headers automatically

##### and sent to one PLIO.

##### In this example, the PL kernel mm2s1 sends raw data to the HLS packet sender

##### module, and the HLS packet sender module generates packets that match the

##### packet switching requirements. It goes through the AI Engine kernel, core[0]

##### (aie/aie_core1.cpp). Then the HLS packet receiver module decodes the packet

##### header and sends the raw data to the PL kernel, s2mm1. Similarly, PL kernel, mm2s2,


###### Displayed in the footer

##### sends a message to PL kernel, s2mm2. And it is the same for mm2s3 to s2mm3 and

##### mm2s4 to s2mm4.

##### Only the HLS packet sender module and HLS packet receiver module deal with the

##### packet IDs generated by the AI Engine compiler. Other PL kernels focus on the data

##### processing.

##### In this example, the four mm2s kernels are created by the --nk option of Vitis (v++)

##### linker. The same applies for the s2mm kernels. You can look at system.cfg to see

##### how all PL kernels are created and connected:

[connectivity]
nk=s2mm: 4 :s2mm_1.s2mm_2.s2mm_3.s2mm_
nk=mm2s: 4 :mm2s_1.mm2s_2.mm2s_3.mm2s_
nk=hls_packet_sender: 1 :hls_packet_sender_
nk=hls_packet_receiver: 1 :hls_packet_receiver_
stream_connect=hls_packet_sender_1.out:ai_engine_0.Datain
stream_connect=ai_engine_0.Dataout0:hls_packet_receiver_1.in

stream_connect=mm2s_1.s:hls_packet_sender_1.s
stream_connect=mm2s_2.s:hls_packet_sender_1.s
stream_connect=mm2s_3.s:hls_packet_sender_1.s
stream_connect=mm2s_4.s:hls_packet_sender_1.s
stream_connect=hls_packet_receiver_1.out0:s2mm_1.s
stream_connect=hls_packet_receiver_1.out1:s2mm_2.s
stream_connect=hls_packet_receiver_1.out2:s2mm_3.s
stream_connect=hls_packet_receiver_1.out3:s2mm_4.s

##### Next review the HLS packet sender module in

##### pl_kernels/hls_packet_sender.cpp. You can review the packet format in the

##### previous section if necessary. The packet ID is generated by the function,

##### generateHeader. Pay attention to how it sends the packet header and reads data

##### from the corresponding PL kernels:

#include "hls_stream.h"
#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "packet_ids_c.h"

static const unsigned int pktType= 0 ;
static const int PACKET_NUM= 4 ; //How many kernels do packet switching
static const int PACKET_LEN= 8 ; //Length for a packet

static const unsigned int packet_ids[PACKET_NUM]={Datain0_0, Datain0_1, Datain0_2, Datain0_3}; //macro
values are generated in packet_ids_c.h

ap_uint< 32 > generateHeader(unsigned int pktType, unsigned int ID){
#pragma HLS inline
ap_uint< 32 > header= 0 ;
header( 4 , 0 )=ID;
header( 11 , 5 )= 0 ;
header( 14 , 12 )=pktType;
header[ 15 ]= 0 ;
header( 20 , 16 )=-1;//source row
header( 27 , 21 )=-1;//source column
header( 30 , 28 )= 0 ;
header[ 31 ]=header( 30 , 0 ).xor_reduce()?(ap_uint< 1 >) 0 :(ap_uint< 1 >) 1 ;
return header;


###### Displayed in the footer

}

void hls_packet_sender(hls::stream<ap_axiu< 32 , 0 , 0 , 0 >> &s0,hls::stream<ap_axiu< 32 , 0 , 0 , 0 >>
&s1,hls::stream<ap_axiu< 32 , 0 , 0 , 0 >> &s2,hls::stream<ap_axiu< 32 , 0 , 0 , 0 >> &s3,
hls::stream<ap_axiu< 32 , 0 , 0 , 0 >> &out, const unsigned int num){
for(unsigned int iter= 0 ;iter<num;iter++){
for(int i= 0 ;i<PACKET_NUM;i++){//Iterate on PL kernels that do packet switching
unsigned int ID=packet_ids[i];
ap_uint< 32 > header=generateHeader(pktType,ID); //packet header
ap_axiu< 32 , 0 , 0 , 0 > tmp;
tmp.data=header;
tmp.keep=-1;
tmp.last= 0 ;
out.write(tmp);
for(int j= 0 ;j<PACKET_LEN;j++){ //packet data
switch(i){//based on which kernel is sending packet, read the corresponding stream
case 0 :tmp=s0.read();break;
case 1 :tmp=s1.read();break;
case 2 :tmp=s2.read();break;
case 3 :tmp=s3.read();break;
}
if(j==PACKET_LEN-1){
tmp.last= 1 ; //last word in a packet has TLAST=
}else{
tmp.last= 0 ;
}
out.write(tmp);
}
}
}
}

##### Now, review the HLS packet receiver module in

##### pl_kernels/hls_packet_receiver.cpp. The packet ID is retrieved from the

##### packet header by the function, getPacketId. Note how it sends the packet data to

##### the corresponding PL kernels:

#include "hls_stream.h"
#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "packet_ids_c.h"

static const int PACKET_NUM= 4 ;
static const int PACKET_LEN= 8 ;

static const unsigned int packet_ids[PACKET_NUM]={Dataout0_0, Dataout0_1, Dataout0_2, Dataout0_3};

unsigned int getPacketId(ap_uint< 32 > header){
#pragma HLS inline
ap_uint< 32 > ID= 0 ;
ID( 4 , 0 )=header( 4 , 0 );
return ID;
}

void hls_packet_receiver(hls::stream<ap_axiu< 32 , 0 , 0 , 0 >> &in, hls::stream<ap_axiu< 32 , 0 , 0 , 0 >>
&out0,hls::stream<ap_axiu< 32 , 0 , 0 , 0 >> &out1,hls::stream<ap_axiu< 32 , 0 , 0 , 0 >>
&out2,hls::stream<ap_axiu< 32 , 0 , 0 , 0 >> &out3,
const unsigned int total_num_packet){
for(unsigned int iter= 0 ;iter<total_num_packet;iter++){
ap_axiu< 32 , 0 , 0 , 0 > tmp=in.read();//first word is packet header
unsigned int ID=getPacketId(tmp.data);


###### Displayed in the footer

unsigned int channel=packet_ids[ID];
for(int j= 0 ;j<PACKET_LEN;j++){
tmp=in.read();
switch(channel){
case 0 :out0.write(tmp);break;
case 1 :out1.write(tmp);break;
case 2 :out2.write(tmp);break;
case 3 :out3.write(tmp);break;
}
}
}
}

##### Note that for both packet sender and packet receiver, the packet IDs are read from

##### packet_ids_c.h, which is generated by the AI Engine compiler. Therefore, it

##### requires that the AI Engine compilation is completed before the PL kernel

##### compilation. Or, if packet IDs are changed when the AI Engine side has had any

##### change, it requires the PL kernels to be re-compiled.

#### Example PS code for Packet Switching

##### The PS code for hardware emulation and hardware flows is in sw/host.cpp. You

##### can review the code. It opens the XCLBIN using the following code.

// Open xclbin
auto dhdl = xrtDeviceOpen( 0 );//device index=
ret=xrtDeviceLoadXclbinFile(dhdl,xclbinFilename);
xuid_t uuid;
xrtDeviceGetXclbinUUID(dhdl, uuid);

##### It allocates buffers for mm2s kernels and s2mm kernels:

// output memory
xrtBufferHandle out_bo1 = xrtBOAlloc(dhdl, mem_size, 0 , /*BANK=*/ 0 );
...
int *host_out1 = (int*)xrtBOMap(out_bo1);
...
// input memory
xrtBufferHandle in_bo1 = xrtBOAlloc(dhdl, mem_size, 0 , /*BANK=*/ 0 );
...
int *host_in1 = (int*)xrtBOMap(in_bo1);
...

##### It initializes the input memory and then syncs the input memory:

// initialize input memory
for(int i= 0 ;i<mem_size/sizeof(int);i++){
*(host_in1+i)=i;
*(host_in2+i)= 2 *i;
*(host_in3+i)= 3 *i;
*(host_in4+i)= 4 *i;
}
// sync input memory
xrtBOSync(in_bo1, XCL_BO_SYNC_BO_TO_DEVICE , mem_size,/*OFFSET=*/ 0 );
...

##### Then it starts the output kernels and input kernels:


###### Displayed in the footer

// start output kernels
xrtKernelHandle s2mm_k1 = xrtPLKernelOpen(dhdl, uuid, "s2mm:{s2mm_1}");
xrtRunHandle s2mm_r1 = xrtRunOpen(s2mm_k1);
xrtRunSetArg(s2mm_r1, 0 , out_bo1);
xrtRunSetArg(s2mm_r1, 2 , mem_size/sizeof(int));
xrtRunStart(s2mm_r1);
...
xrtKernelHandle hls_packet_receiver_k = xrtPLKernelOpen(dhdl, uuid, "hls_packet_receiver");
xrtRunHandle hls_packet_receiver_r = xrtRunOpen(hls_packet_receiver_k);
xrtRunSetArg(hls_packet_receiver_r, 5 , total_packet_num);
xrtRunStart(hls_packet_receiver_r);

// start input kernels
xrtKernelHandle mm2s_k1 = xrtPLKernelOpen(dhdl, uuid, "mm2s:{mm2s_1}");
xrtRunHandle mm2s_r1 = xrtRunOpen(mm2s_k1);
xrtRunSetArg(mm2s_r1, 0 , in_bo1);
xrtRunSetArg(mm2s_r1, 2 , mem_size/sizeof(int));
xrtRunStart(mm2s_r1);
...
xrtKernelHandle hls_packet_sender_k = xrtPLKernelOpen(dhdl, uuid, "hls_packet_sender");
xrtRunHandle hls_packet_sender_r = xrtRunOpen(hls_packet_sender_k);
xrtRunSetArg(hls_packet_sender_r, 5 , packet_num);
xrtRunStart(hls_packet_sender_r);

##### Then it starts the graph:

// start graph
adf::registerXRT(dhdl, uuid);
gr.run( 2 ); //Iteration number=2. The amount of data matches for PL kernels and graph

##### Then it waits for s2mm kernels to complete, and syncs output memory:

// wait for s2mm to complete
xrtRunWait(s2mm_r1);
...
// sync output memory
xrtBOSync(out_bo1, XCL_BO_SYNC_BO_FROM_DEVICE , mem_size,/*OFFSET=*/ 0 );
...

##### Then, finally, it performs post-processing and releases objects.

##### Note that there is no special packet switching handling in the PS code. It is already

##### done on the AI Engine and PL side.

#### Run Hardware Emulation and Hardware Flows

##### 1. Run HW emulation with the following make command to build the HW system

##### and host application.

```
make run_hw_emu
```
##### Tip: If the keyboard is accidentally hit and stops the system booting

##### automatically, type boot at the Versal> prompt to resume the

##### system booting.

##### 2. After Linux has booted, run the following commands at the Linux prompt (this

##### is only for HW cosim).


###### Displayed in the footer

```
mount /dev/mmcblk0p1 /mnt
cd /mnt
export XILINX_XRT=/usr
export XCL_EMULATION_MODE=hw_emu
./host.exe a.xclbin
```
##### To exit QEMU press Ctrl+A, x.

##### 3. To run in hardware, first build the system and application using the following

##### make command.

```
make package TARGET=hw
```
##### 4. After Linux has booted, run the following commands at the Linux prompt.

```
mount /dev/mmcblk0p1 /mnt
cd /mnt
export XILINX_XRT=/usr
./host.exe a.xclbin
```
##### The host code is self-checking; it checks the correctness of the output data. If the

##### output data is correct, after the run has completed, it will print:

TEST PASSED

#### Conclusion

##### In this step, you learned about the following concepts.

##### Constructing packet switching graph

##### Packet format and preparing data for the AI Engine simulator

##### Designing PL kernels for packet switching

##### PS application and running HW/HW emulation flows

##### Next, review Buffer Based AI Engine Kernels with Mixed Data Types.

```
Copyright © 2020–2024 Advanced Micro Devices, Inc
Terms and Conditions
```
## Buffer-Based AI Engine Kernels with Mixed

## Data Types

##### This example is similar to the previous Buffer-based AI Engine Kernels example,

##### except that one AI Engine kernel has floating point interfaces and one AI Engine

##### kernel has cint16 interfaces. It will focus on the difference with the previous

##### example. One difference is that because the input and output data are in integer

##### format for the AI Engine simulator, it has to convert data between float/cint

##### and integer data types. See Prepare Data for AI Engine Simulator. Another


###### Displayed in the footer

##### difference is that the PS code has to take care of the input and output data types.

##### See PS Application and HW Emulation Flows.

##### The following topics are already covered in Buffer Based AI Engine Kernels.

##### Construct Graph with Packet Switching Capability

##### Packet Format

##### Example PL Kernels for Packet Switching

##### Example PS code for Packet Switching

### Prepare Data for AI Engine Simulator

##### Change the working directory to buffer_aie_mix_int32_float_cint16. The

##### graph code for this example is the same as the Buffer Based AI Engine Kernels

##### example. The AI Engine kernel core[2] (aie/aie_core3.cpp) has floating point

##### interfaces, and the AI Engine kernel core[3] (aie/aie_core4.cpp) has cint

##### interfaces.

##### When preparing the data for the AI Engine simulator, all values should be in 32-bit

##### integer format. The conversion is similar to the reinterpret_cast operation in

##### C++. It is done manually in any language. For example, when you want to feed float

##### data 1.0f, 2.0f,..., into the AI Engine kernel, the integer format can be generated

##### in C as shown in the following code.

//input data for float
for(int i= 0 ;i< 16 ;i++){
float tmp=i;
printf("%d\n",*(int*)&tmp);
}

##### Then the data in the input file (data/input.txt) for float, 1.0f,2.0f,...,16.0f,

##### should be as follows.

0
1065353216
1073741824
1077936128
1082130432
1084227584
1086324736
1088421888
1090519040
1091567616
1092616192
1093664768
1094713344
1095761920
1096810496
1097859072


###### Displayed in the footer

##### Similarly, type cint16 should be converted to integer type. For example, for cint16

##### data, {0,0},{4,4},{8,8},..., the integer format can be generated in C as shown in the

##### following code.

//input data for cint16
for(int i= 0 ;i< 16 ;i++){
int tmp=i* 4 ;
tmp=tmp<< 16 ;
tmp+=i* 4 ;
printf("%d\n",tmp);
}

##### Then the data in the input file (data/input.txt) for cint16 data, {0,0},{4,4},

##### {8,8},...,{60,60}, should be as follows.

0
262148
524296
786444
1048592
1310740
1572888
1835036
2097184
2359332
2621480
2883628
3145776
3407924
3670072
3932220

##### Take a look at the input file data/input.txt to see how input data is organized.

##### Run the following make command to run the AI Engine compiler and simulator.

make aiesim

##### The output data is in aiesimulator_output/data/output.txt. Similarly, the

##### output data can be converted from integer to float or cint16 to be human-

##### readable.

#### PS Application and HW Emulation Flows

##### The difference in the PS application from Buffer Based AI Engine Kernels is that the

##### input buffers and output buffers for different data types should be modified

##### accordingly. Take a look at the code in sw/host.cpp. Note how float and complex

##### type (for cint16) is used in the code.

// output memory
xrtBufferHandle out_bo3 = xrtBOAlloc(dhdl, mem_size, 0 , /*BANK=*/ 0 );
xrtBufferHandle out_bo4 = xrtBOAlloc(dhdl, mem_size, 0 , /*BANK=*/ 0 );
float *host_out3 = (float*)xrtBOMap(out_bo3);
std::complex<short> *host_out4 = (std::complex<short>*)xrtBOMap(out_bo4);

// input memory


###### Displayed in the footer

xrtBufferHandle in_bo3 = xrtBOAlloc(dhdl, mem_size, 0 , /*BANK=*/ 0 );
xrtBufferHandle in_bo4 = xrtBOAlloc(dhdl, mem_size, 0 , /*BANK=*/ 0 );
float *host_in3 = (float*)xrtBOMap(in_bo3);
std::complex<short> *host_in4 = (std::complex<short>*)xrtBOMap(in_bo4);

##### Correspondingly, the pre-processing and post-processing of this data has been

##### changed.

##### 1. Run HW emulation with the following ``make` command to build the HW

##### system and host application.

```
make run_hw_emu
```
##### Tip: If the keyboard is accidentally hit and stops the system booting

##### automatically, type boot at the Versal> prompt to resume the

##### system booting.

##### 2. After Linux has booted, run the following commands at the Linux prompt (this

##### is only for HW cosim).

```
mount /dev/mmcblk0p1 /mnt
cd /mnt
export XILINX_XRT=/usr
export XCL_EMULATION_MODE=hw_emu
./host.exe a.xclbin
```
##### To exit QEMU press Ctrl+A, x.

##### 3. To run in hardware, first build the system and application using the following

##### make command.

```
make package TARGET=hw
```
##### 4. After Linux has booted, run the following commands at the Linux prompt.

```
mount /dev/mmcblk0p1 /mnt
cd /mnt
export XILINX_XRT=/usr
./host.exe a.xclbin
```
##### The host code is self-checking. It will check the correctness of output data. If the

##### output data is correct, after the run has completed, it will print:

TEST PASSED

### Conclusion

##### In this step, you learned about the following concepts.

##### Preparing float and cint16 data types for the AI Engine simulator.

##### PS application for different data types.


###### Displayed in the footer

##### Next, review Packet Stream Based AI Engine Kernels.

#### Support

##### GitHub issues will be used for tracking requests and bugs. For questions go to

##### forums.

```
Copyright © 2020–2024 Advanced Micro Devices, Inc
Terms and Conditions
```

```
Confidential - Copyright © Fluid Topics
```
## AI Engine-ML Kernel and Graph

## Programming Guide (UG1603)


```
Displayed in the footer
```
##### Explicit Packet Switching

```
Packet Switching Graph Constructs
Packet Switching and the aiesimulator
```

```
Displayed in the footer
```
##### ✎

# Explicit Packet Switching

###### Just as multiple AI Engine kernels can share a single processor and execute in a

###### interleaved manner, multiple stream connections can be shared on a single physical

###### channel. This mechanism is known as Packet Switching. The AI Engine-ML

###### architecture and compiler work together to provide a programming model where up

###### to 32 stream connections can share the same physical channel.

###### The Explicit Packet Switching feature allows fine-grain control over how packets are

###### generated, distributed, and consumed in a graph computation. Explicit Packet

###### Switching is typically recommended in cases where many low bandwidth streams

###### from common PL source can be distributed to different AI Engine-ML destinations.

###### Similarly, many low bandwidth streams from different AI Engine-ML sources to a

###### common PL destination can also take advantage of this feature. Because a single

###### physical channel is shared between multiple streams, you minimize the number of

###### AI Engine-ML to PL interface streams used. This section describes graph constructs

###### to create packet-switched streams explicitly in the graph.

## Packet Switching Graph Constructs

###### Packet-switched streams are essentially multiplexed data streams that carry

###### different types of data at different times. Packet-switched streams do not provide

###### deterministic latency due to the potential for resource contention with other packet-

###### switched streams. The multiplexed data flows in units of packets with a 32-bit

###### packet header and a variable number of payload words. A header word needs to be

###### sent before the actual payload data and the TLAST signal is required on the last

###### word of the packet. Two new data types called input_pktstream and

###### output_pktstream are introduced to represent the multiplexed data streams as

###### input to or output from a kernel, respectively. More details on the packet headers

###### and data types can be found in Packet Stream Operations.

###### Note: By convention, packets originating in the programmable logic are

###### initialized with row, column to be -1,-1.

###### To explicitly control the multiplexing and de-multiplexing of packets, two new

###### templated node classes, pktsplit<n> and pktmerge<n>, are added to the ADF

###### graph library. A node instance of class pktmerge<n> is a n:1 multiplexer of n packet

###### streams producing a single packet stream. A node instance of class pktsplit<n>

###### is a 1:n de-multiplexer of a packet stream producing n different packet streams. The


```
Displayed in the footer
```
###### maximum number of allowable packet streams is 32 on a single physical channel (n

###### ≤ 32).

###### A kernel can receive packets of data either as buffers of data or as

###### input_pktstream. And a kernel can send packets of data either as buffers of data

###### or as output_pktstream.

###### To connect from ports, local buffers or packet streams to ports, local buffers or

###### packet streams, use a connect construct, such as:

```
connect (<SOURCE>.out[0], <DEST>.in[0]);
```
###### When a kernel receives packets of data as a buffer of data, the header and TLAST

###### are dropped prior to the kernel receiving the buffer of data. If the kernel writes an

###### output buffer of data, the packet header and TLAST are automatically inserted,

###### when the buffer is transferred by DMA to a packet stream.

###### However, if the kernel receives input_pktstream of data, the kernel needs to

###### process the packet header and TLAST, in addition to the packet data. Similarly, if

###### the kernel sends an output_pktstream of data, the kernel needs to insert the

###### packet header and TLAST, in addition to the packet data into the output stream.

###### These concepts are illustrated in the following example.

```
class ExplicitPacketSwitching: public adf::graph {
private:
adf:: kernel core[4];
adf:: pktsplit<4> sp;
adf:: pktmerge<4> mg;
public:
adf::input_plio in;
adf::output_plio out;
mygraph() {
core[0] = adf::kernel::create(aie_core1);
core[1] = adf::kernel::create(aie_core2);
core[2] = adf::kernel::create(aie_core3);
core[3] = adf::kernel::create(aie_core4);
adf::source(core[0]) = "aie_core1.cpp";
adf::source(core[1]) = "aie_core2.cpp";
adf::source(core[2]) = "aie_core3.cpp";
adf::source(core[3]) = "aie_core4.cpp";
```
```
in=input_plio::create("Datain0", plio_32_bits,
"data/input.txt");
```

```
Displayed in the footer
```
```
out=output_plio::create("Dataout0", plio_32_bits,
"data/output.txt");
```
```
sp = adf::pktsplit<4>::create();
mg = adf::pktmerge<4>::create();
for(int i=0;i<4;i++){
adf::runtime<ratio>(core[i]) = 0.9;
adf::connect(sp.out[i], core[i].in[0]);
adf::connect(core[i].out[0], mg.in[i]);
}
adf::connect(in.out[0], sp.in[0]);
adf::connect(mg.out[0], out.in[0]);
}
};
```
###### The graph has one input PLIO port and one output PLIO port. The input packet

###### stream from the PL is split four ways and input to four different AI Engine kernels.

###### The output streams from the four AI Engine kernels are merged into one packet

###### stream which is output to the PL. The Vitis IDE Graph view of the code is shown as

###### follows.

###### Figure: Graph View

###### One kernel code example is as follows.

```
const uint32 pktType=0;
void aie_core1(input_pktstream *in,output_pktstream *out){
readincr(in);//read header and discard
```

```
Displayed in the footer
```
```
uint32 ID=getPacketid(out,0);//for output pktstream
writeHeader(out,pktType,ID); //Generate header for output
```
```
bool tlast;
for(int i=0;i<8;i++){
int32 tmp=readincr(in,tlast);
tmp+=1;
writeincr(out,tmp,i==7);//TLAST=1 for last word
}
}
```
###### Warning: input_pktstream is read as integer input.

###### Following is an example kernel code that accepts and transfers floating-point data

###### type.

```
const uint32 pktType=0;
void aie_core1_float(input_pktstream *in,output_pktstream *out){
readincr(in);//read header and discard
uint32 ID=getPacketid(out,0);//for output pktstream
writeHeader(out,pktType,ID); //Generate header for output
```
```
bool tlast;
for(int i=0;i<4;i++){
int32 tmp=readincr(in,tlast);//read data as integer type
float tmp_f=reinterpret_cast<float&>(tmp);//Reinterpret memory
as float
tmp_f+=1.0f;
writeincr(out,tmp_f,i==3);//TLAST=1 for last word
}
}
```
###### The input data should also be in integer format. For example, if float values 0.0f,

###### 1.0f, 2.0f and 3.0f are sent to the kernel, they are converted into integer values

###### in the input data file:

```
0
1065353216
1073741824
1077936128
```

```
Displayed in the footer
```
##### ✎

###### After the kernel execution, the float output values are 1.0f, 2.0f, 3.0f and 4.0f.

###### In a simulation output data file, the output values are in integer format, as follows:

```
1065353216
1073741824
1077936128
1082130432
```
#### Related Information

###### Adaptive Data Flow Graph Specification Reference

### Packet Switching and the aiesimulator

###### Explicit packet switching is supported by the aiesimulator. Consider the example

###### of the previous graph that expects packet switched data from the PL; the data is

###### split inside the AI Engine-ML and sent to four AI Engine kernels. On the output side

###### the four kernel outputs are merged into one output stream to the PL.

###### The input data file from the PL contains all the packet switched data from the PL for

###### the four AI Engine kernels in the previous example. It contains the data for different

###### kernels, packet by packet. Each packet of data is for one iteration of an AI Engine

###### kernel. The data format is as follows.

```
2415853568
0 1 2 3 4 5 6
```
```
TLAST
7
```
###### Here, 2415853568 is 0x8fff0000 in hex format. The five least significant bits are

###### the packet ID, 0 in this case. The last data in the packet has the keyword TLAST,

###### which denotes the last data for the input of the kernel.

###### Note: Integer values only are accepted as a packet header ID for the PL packet

###### inputs to the aiesimulator.

###### You can construct the header for each packet manually, or write helper functions to

###### generate the header. The AI Engine-ML compiler generates a packet switching


```
Displayed in the footer
```
###### report file Work/reports/packet_switching_report.json that lists the packet IDs used

###### in the graph. In addition it also generates Work/temp/packet_ids_c.h and

###### Work/temp/packet_ids_v.h header files that can be included in your C or Verilog

###### kernel code.


