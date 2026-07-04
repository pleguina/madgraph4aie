# report_power.tcl - headless Versal AI-Engine power from the console.
#
# Opens the implemented (routed) checkpoint of the full system_project design, which
# contains the PL + NoC + PS + the AI-Engine array, and runs report_power. The AIE
# domain power is reported directly (no XPE spreadsheet / PDM GUI, no board).
#
#   source /tools/Xilinx/Vitis/2024.1/settings64.sh
#   vivado -mode batch -source report_power.tcl -tclargs <routed.dcp> <out_prefix>
#
# Measured 2026-07-04 on xcvc1902-vsva2197-2MP-e-S:
#   AIE / ai_engine_0 = 54.79 W (400 tiles), total on-chip = 82.71 W
#   (vectorless/flat AIE activity; refine dynamic term with measured duty cycles).

set dcp    [lindex $argv 0]
set prefix [lindex $argv 1]
if {$dcp eq ""}    { set dcp "system_project/.../impl_1/vitis_design_wrapper_routed.dcp" }
if {$prefix eq ""} { set prefix "power" }

puts "==> opening routed checkpoint (Versal + AIE): $dcp"
open_checkpoint $dcp
puts "==> part: [get_property PART [current_design]]"
report_power -file ${prefix}_full.rpt
report_power -hierarchical_depth 3 -file ${prefix}_hier.rpt
puts "==> DONE: ${prefix}_full.rpt (see the 'AIE' on-chip row and 'ai_engine_0' hierarchy row)"
