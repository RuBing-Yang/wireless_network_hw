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
set val(stop) 20.3 ;# nam stop time
set val(nn) 7 ;# number of mobilenodes
set val(nnoise) 0 ;# number of noise nodes
set val(nm) 1 ;# number of types of messages
set val(nc) 3 ;# number of channels
set val(ni) 3 ;# number of interfaces, <= number of channels
set pktsize 1000 ;# packet size in bytes
set pktrate 0.01 ;# packet rate in seconds
set filename fxj_circ ;# trace file name
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

$n(0) set X_ 322
$n(0) set Y_ 116
$n(0) set Z_ 0.0
$ns_ initial_node_pos $n(0) 20

$n(1) set X_ 532
$n(1) set Y_ 113
$n(1) set Z_ 0.0
$ns_ initial_node_pos $n(1) 20

$n(2) set X_ 743
$n(2) set Y_ 154
$n(2) set Z_ 0.0
$ns_ initial_node_pos $n(2) 20

$n(3) set X_ 862
$n(3) set Y_ 306
$n(3) set Z_ 0.0
$ns_ initial_node_pos $n(3) 20

$n(4) set X_ 727
$n(4) set Y_ 478
$n(4) set Z_ 0.0
$ns_ initial_node_pos $n(4) 20

$n(5) set X_ 511
$n(5) set Y_ 490
$n(5) set Z_ 0.0
$ns_ initial_node_pos $n(5) 20

$n(6) set X_ 330
$n(6) set Y_ 502
$n(6) set Z_ 0.0
$ns_ initial_node_pos $n(6) 20


# $ns_ at 0.0 "$n(0) setdest 322 116 1000"
# $ns_ at 0.0 "$n(1) setdest 532 113 1000"
# $ns_ at 0.0 "$n(2) setdest 743 154 1000"
# $ns_ at 0.0 "$n(3) setdest 862 306 1000"
# $ns_ at 0.0 "$n(4) setdest 727 478 1000"
# $ns_ at 0.0 "$n(5) setdest 511 490 1000"
# $ns_ at 0.0 "$n(6) setdest 330 502 1000"


# Set links between the nodes


# $ns_ duplex-link $n(0) $n(1) 2Mb 10ms DropTail
# $ns_ duplex-link $n(1) $n(2) 2Mb 10ms DropTail
# $ns_ duplex-link $n(2) $n(3) 2Mb 10ms DropTail
# $ns_ duplex-link $n(3) $n(4) 2Mb 10ms DropTail
# $ns_ duplex-link $n(4) $n(5) 2Mb 10ms DropTail
# $ns_ duplex-link $n(5) $n(6) 2Mb 10ms DropTail
# $ns_ duplex-link $n(2) $n(5) 2Mb 10ms DropTail
# 
# 
# 
# $ns_ duplex-link-op $n(0) $n(1) orient right
# $ns_ duplex-link-op $n(1) $n(2) orient right
# $ns_ duplex-link-op $n(2) $n(3) orient right-up
# $ns_ duplex-link-op $n(3) $n(4) orient left-up
# $ns_ duplex-link-op $n(4) $n(5) orient left
# $ns_ duplex-link-op $n(5) $n(6) orient left
# $ns_ duplex-link-op $n(2) $n(5) orient left-up


#======================================
# add Noise Node
#======================================



#======================================
# UDP Agents Definition
#======================================
set last_node_id [expr $val(nn)-1]

puts "begin to definite UDP Agent"

set udp [new Agent/UDP]
$ns_ attach-agent $n(0) $udp
set null [new Agent/Null]
$ns_ attach-agent $n(6) $null
$ns_ connect $udp $null
set cbr [new Application/Traffic/CBR]
$cbr attach-agent $udp
$cbr set packetSize_ $pktsize
$cbr set interval_ $pktrate
$ns_ at 1.0 "$cbr start"
$ns_ at 20.0 "$cbr stop"



# n(2) move and link(n(2)-n(3)) break
$ns_ at 10.0 "$n(2) setdest 547 269 1000"

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
for {set i 0} {$i < $val(nn)} { incr i } {
    $ns_ at $val(stop) "$n($i) reset"
} 
$ns_ at $val(stop) "$ns_ nam-end-wireless $val(stop)"
$ns_ at $val(stop) "finish"
$ns_ at $val(stop) "puts \"done\" ; $ns_ halt"
$ns_ run

