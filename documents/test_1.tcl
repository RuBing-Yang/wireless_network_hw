Mac/802_11 set dataRate_ 5.4e6 ;# rate for data frames in Mbps
#======================================
# Simulation Parameters Setup
#======================================
set val(chan) Channel/WirelessChannel ;# channel type
set val(prop) Propagation/TwoRayGround ;# radio-propagation model
set val(netif) Phy/WirelessPhy ;# network interface type
set val(mac) Mac/802_11 ;# MAC type
set val(ifq) Queue/DropTail/PriQueue ;# interface queue type
set val(ll) LL ;# link layer type
set val(ant) Antenna/OmniAntenna ;# antenna model
set val(ifqlen) 50 ;# max packet in ifq
set val(rp) AODVUU ;# routing protocol
set val(x) 1000 ;# X dimension of topography
set val(y) 1000 ;# Y dimension of topography
set val(stop) 42 ;# nam stop time
set val(nn) 5 ;# number of mobilenodes
set val(nc) 3 ;# number of channels
set val(ni) 3 ;# number of interfaces, <= number of channels
set pktsize 1000 ;# packet size in bytes
set pktrate 0.01 ;# packet rate in seconds
set filename test
puts "Ad-Hoc Wireless Network in Chain Topologies - $val(nn) Nodes, $val(nc) Channels, $val(ni) Interfaces"
#======================================
# Initialization
#======================================
# Create a ns simulator
set ns_ [new Simulator]
# Setup topography object
set topo [new Topography]
$topo load_flatgrid $val(x) $val(y)
set god_ [create-god [expr $val(nn)*$val(nc)]]
# Open the NS trace file
set tracefd [open $filename.tr w]
$ns_ trace-all $tracefd
$ns_ use-newtrace
# Open the NAM trace file
set namfile [open $filename.nam w]
$ns_ namtrace-all $namfile
$ns_ namtrace-all-wireless $namfile $val(x) $val(y)
# Create wireless channels
for {set i 0} {$i < $val(nc)} {incr i} {
    set chan($i) [new $val(chan)]
}
#======================================
# Mobile Node Parameter Setup
#======================================
$ns_ node-config -adhocRouting $val(rp) \
                -llType $val(ll) \
                -macType $val(mac) \
                -ifqType $val(ifq) \
                -ifqLen $val(ifqlen) \
                -antType $val(ant) \
                -propType $val(prop) \
                -phyType $val(netif) \
                -channel $chan(0) \
                -topoInstance $topo \
                -agentTrace ON \
                -routerTrace ON \
                -macTrace OFF \
                -movementTrace OFF \
                -ifNum $val(ni)    \
                -workMode 0 \
                -noiseChannel 0
#======================================
# Nodes Definition
#======================================
puts "begin to add channel"
$ns_ change-numifs $val(nc)
for {set i 0} {$i < $val(nc)} {incr i} {
    $ns_ add-channel $i $chan($i)
}
# Create nodes
puts "begin to create nodes"
for {set i 0} {$i < $val(nn)} {incr i} {
    set n($i) [$ns_ node]
    $god_ new_node $n($i)
}
puts "created nodes"
# Set node positions in horizontal chain topology
set nodedist 250

$n(0) set X_ 100
$n(0) set Y_ 300
$n(0) set Z_ 0
$ns_ initial_node_pos $n(0) 50
$n(0) random-motion 0

$n(1) set X_ 300
$n(1) set Y_ 300
$n(1) set Z_ 0
$ns_ initial_node_pos $n(1) 50
$n(1) random-motion 0

$n(2) set X_ 500
$n(2) set Y_ 300
$n(2) set Z_ 0
$ns_ initial_node_pos $n(2) 50
$n(2) random-motion 0

$n(3) set X_ 750
$n(3) set Y_ 300
$n(3) set Z_ 0
$ns_ initial_node_pos $n(3) 50
$n(3) random-motion 0

$n(4) set X_ 450
$n(4) set Y_ 600
$n(4) set Z_ 0
$ns_ initial_node_pos $n(4) 50
$n(4) random-motion 0


#======================================
# Agents Definition
#======================================
set udp0 [new Agent/UDP]
$ns_ attach-agent $n(0) $udp0
set sink0 [new Agent/Null]
set last_node_id [expr $val(nn)-1]
$ns_ attach-agent $n(3) $sink0
$ns_ connect $udp0 $sink0

#======================================
# Applications Definition
#======================================
# Setup a CBR Application over UDP connection
set cbr0 [new Application/Traffic/CBR]
$cbr0 attach-agent $udp0
$cbr0 set packetSize_ $pktsize
$cbr0 set interval_ $pktrate
$ns_ at 1.0 "$cbr0 start"
$ns_ at 20.0 "$n(4) setdest 450 300 800"
$ns_ at 20.0 "$n(2) setdest 600 300 400"

$ns_ at 41.0 "$cbr0 stop"


#======================================
# Simulation Termination
#======================================
# Define a finish procedure
proc finish {} {
global ns_ tracefd filename pktsize last_node_id
    global namfile
    $ns_ flush-trace
    close $tracefd
    close $namfile
    exec nam $filename.nam &
    # Call throughput analyzer (AWK scripts written by Marco Fiore, marco.fiore@polito.it)
    # exec awk -f avgStats.awk src=0 dst=$last_node_id flow=0 pkt=$pktsize $filename.tr &
    exit 0
}
for {set i 0} {$i < $val(nn) } { incr i } {
    $ns_ at $val(stop) "$n($i) reset"
} 
$ns_ at $val(stop) "$ns_ nam-end-wireless $val(stop)"
$ns_ at $val(stop) "finish"
$ns_ at $val(stop) "puts \"done\" ; $ns_ halt"
$ns_ run











--

祝工作、学习顺利！
                        冯鹏