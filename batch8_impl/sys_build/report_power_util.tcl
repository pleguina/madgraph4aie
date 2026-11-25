# ============================================================================
# report_power_util.tcl
#
# Run on the routed checkpoint produced by the v++ hw link. Writes:
#   - PL resource utilisation
#   - power, split by domain, including the AI Engine domain
#
# Usage: vivado -mode batch -source report_power_util.tcl -tclargs <outdir>
# ============================================================================

set outdir [lindex $argv 0]
file mkdir $outdir

# The v++ hw link leaves its Vivado project under _x/link/vivado/vpl/prj.
# Checkpoint naming has moved between releases, so search rather than assume.
set candidates [list]
foreach pat {
    "_x/link/vivado/vpl/prj/prj.runs/impl_1/*routed.dcp"
    "_x/link/vivado/vpl/prj/prj.runs/impl_1/*.dcp"
    "_x/link/vivado/vpl/*routed.dcp"
    "_x/link/vivado/vpl/prj/*.runs/impl_1/*routed.dcp"
    "./*routed.dcp"
} {
    foreach f [glob -nocomplain $pat] { lappend candidates $f }
}

if {[llength $candidates] == 0} {
    puts "ERROR: no routed checkpoint found. Searched:"
    foreach pat {_x/link/vivado/vpl/prj/prj.runs/impl_1 _x/link/vivado/vpl} {
        puts "  $pat"
    }
    exit 1
}

# Prefer an explicitly "routed" checkpoint if one is present.
set dcp [lindex $candidates 0]
foreach f $candidates { if {[string match "*routed*" $f]} { set dcp $f ; break } }
puts "INFO: opening checkpoint $dcp"
open_checkpoint $dcp

# ---- Resource utilisation ---------------------------------------------------
report_utilization                 -file $outdir/utilization.rpt
report_utilization -hierarchical   -file $outdir/utilization_hier.rpt

# ---- Power ------------------------------------------------------------------
# Vectorless estimation: no SAIF is available for the AIE array, so this is a
# tool estimate under default toggle rates.
report_power                       -file $outdir/power.rpt
# -hierarchical was removed from report_power in 2024.2; use a depth instead.
catch {report_power -hierarchical_depth 4 -file $outdir/power_hier.rpt}

# Machine-readable summary of the domain split, so the AIE-domain number can be
# quoted without hand-reading the text report.
set fh [open $outdir/power_summary.txt w]
foreach prop {TOTAL_POWER DYNAMIC_POWER DEVICE_STATIC_POWER CONFIDENCE_LEVEL} {
    if {![catch {set v [get_property STATS.$prop [current_design]]}]} {
        puts $fh "$prop = $v"
    }
}
close $fh

report_timing_summary -file $outdir/timing_summary.rpt

puts "INFO: reports written to $outdir"
exit 0
