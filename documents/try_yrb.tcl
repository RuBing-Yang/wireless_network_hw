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
set val(nn) 6 ;# number of mobilenodes
set val(nnoise) 1 ;# number of noise nodes
set val(nm) 1 ;# number of types of messages
set val(nc) 3 ;# number of channels
set val(ni) 3 ;# number of interfaces, <= number of channels
set pktsize 1000 ;# packet size in bytes
set pktrate 0.01 ;# packet rate in seconds
set filename try_yrb ;# trace file name
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

$n(1) set X_ 83
$n(1) set Y_ 274
$n(1) set Z_ 0.0
$ns_ initial_node_pos $n(1) 20

$n(2) set X_ 165
$n(2) set Y_ 445
$n(2) set Z_ 0.0
$ns_ initial_node_pos $n(2) 20

$n(3) set X_ 394
$n(3) set Y_ 445
$n(3) set Z_ 0.0
$ns_ initial_node_pos $n(3) 20

$n(4) set X_ 457
$n(4) set Y_ 268
$n(4) set Z_ 0.0
$ns_ initial_node_pos $n(4) 20

$n(5) set X_ 280
$n(5) set Y_ 184
$n(5) set Z_ 0.0
$ns_ initial_node_pos $n(5) 20

$n(0) set X_ 554
$n(0) set Y_ 431
$n(0) set Z_ 0.0
$ns_ initial_node_pos $n(0) 20

# Set links between the nodes

$ns_ duplex-link $n(1) $n(2) 2Mb 10ms DropTail
$ns_ duplex-link $n(2) $n(3) 2Mb 10ms DropTail
$ns_ duplex-link $n(3) $n(4) 2Mb 10ms DropTail
$ns_ duplex-link $n(1) $n(5) 2Mb 10ms DropTail
$ns_ duplex-link $n(5) $n(4) 2Mb 10ms DropTail
$ns_ duplex-link $n(3) $n(0) 2Mb 10ms DropTail
$ns_ duplex-link $n(4) $n(0) 2Mb 10ms DropTail

$ns_ duplex-link-op $n(1) $n(2) orient up
$ns_ duplex-link-op $n(2) $n(3) orient right
$ns_ duplex-link-op $n(3) $n(4) orient down
$ns_ duplex-link-op $n(1) $n(5) orient right-down
$ns_ duplex-link-op $n(5) $n(4) orient right-up
$ns_ duplex-link-op $n(3) $n(0) orient right-down
$ns_ duplex-link-op $n(4) $n(0) orient right-up


#======================================
# add Noise Node
#======================================
$ns_ node-config -workMode -1  -noiseChannel 0

# Create nodes
puts "begin to create noise nodes"
for {set i 0} {$i < $val(nnoise)} {incr i} {
    set nnoise($i) [$ns_ node]
    $god_ new_node $nnoise($i)
}
puts "created noise nodes"

$nnoise(0) set X_ 280
$nnoise(0) set Y_ 160
$nnoise(0) set Z_ 0.0
$ns_ initial_node_pos $nnoise(0) 20


#======================================
# UDP Agents Definition
#======================================
set last_node_id [expr $val(nn)-1]

puts "begin to definite UDP Agent"
for {set i 0} {$i < $val(nm)} {incr i} {
    set udp($i) [new Agent/UDP]
    $ns_ attach-agent $n(1) $udp($i)
    set null($i) [new Agent/Null]
    $ns_ attach-agent $n(4) $null($i)
    $ns_ connect $udp($i) $null($i)
}
puts "definite UDP Agent"

	
#======================================
# Applications Definition
#======================================
# Setup CBR Application over UDP connection
puts "begin to Setup CBR Application"
for {set i 0} {$i < $val(nm)} {incr i} {
    set cbr($i) [new Application/Traffic/CBR]
    $cbr($i) attach-agent $udp($i)
    $cbr($i) set packetSize_ $pktsize
    $cbr($i) set interval_ $pktrate
    $ns_ at 1.0 "$cbr($i) start"
    $ns_ at 40.0 "$cbr($i) stop"
}
puts "Setup CBR Application over UDP connection"

# n(3) move and link(n(3)-n(4)) break
$ns_ at 20.0 "$n(3) setdest 394 600 200"
# link 3->4 should change to 3->6->4


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

