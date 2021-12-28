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
set val(x) 1200 ;# X dimension of topography
set val(y) 1200 ;# Y dimension of topography
set val(stop) 42 ;# nam stop time
set val(nn) 11 ;# number of mobilenodes
set val(nm) 1 ;# number of types of messages
set val(nc) 3 ;# number of channels
set val(ni) 3 ;# number of interfaces, <= number of channels
set pktsize 1000 ;# packet size in bytes
set pktrate 0.1 ;# packet rate in seconds
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

$n(0) set X_ 731
$n(0) set Y_ 563
$n(0) set Z_ 0.0
$ns_ initial_node_pos $n(0) 20

$n(1) set X_ 877
$n(1) set Y_ 681
$n(1) set Z_ 0.0
$ns_ initial_node_pos $n(1) 20

$n(2) set X_ 827
$n(2) set Y_ 431
$n(2) set Z_ 0.0
$ns_ initial_node_pos $n(2) 20

$n(3) set X_ 885
$n(3) set Y_ 536
$n(3) set Z_ 0.0
$ns_ initial_node_pos $n(3) 20

$n(4) set X_ 924
$n(4) set Y_ 318
$n(4) set Z_ 0.0
$ns_ initial_node_pos $n(4) 20

$n(5) set X_ 1060
$n(5) set Y_ 631
$n(5) set Z_ 0.0
$ns_ initial_node_pos $n(5) 20

$n(6) set X_ 1058
$n(6) set Y_ 447
$n(6) set Z_ 0.0
$ns_ initial_node_pos $n(6) 20

$n(7) set X_ 1067
$n(7) set Y_ 347
$n(7) set Z_ 0.0
$ns_ initial_node_pos $n(7) 20

$n(8) set X_ 1254
$n(8) set Y_ 617
$n(8) set Z_ 0.0
$ns_ initial_node_pos $n(8) 20

$n(9) set X_ 1198
$n(9) set Y_ 453
$n(9) set Z_ 0.0
$ns_ initial_node_pos $n(9) 20

$n(10) set X_ 1064
$n(10) set Y_ 755
$n(10) set Z_ 0.0
$ns_ initial_node_pos $n(10) 20

# set n[0-9]+ \[\$ns_ node\]
# (\$n)([0-9]+)
# $1($2)
# (\$ns)([^_)])
# $1_$2

#======================================
# add Noise Node
#======================================
# 全信道干扰： noiseChannel = -1


#======================================
# UDP Agents Definition
#======================================
set last_node_id [expr $val(nn)-1]

puts "begin to definite UDP Agent"
set i 0
set udp($i) [new Agent/UDP]
$ns_ attach-agent $n(0) $udp($i)
set null($i) [new Agent/Null]
$ns_ attach-agent $n(8) $null($i)
$ns_ connect $udp($i) $null($i)

set i 1
set udp($i) [new Agent/UDP]
$ns_ attach-agent $n(4) $udp($i)
set null($i) [new Agent/Null]
$ns_ attach-agent $n(10) $null($i)
$ns_ connect $udp($i) $null($i)
puts "definite UDP Agent"
	
#======================================
# Applications Definition
#======================================
# Setup CBR Application over UDP connection
puts "begin to Setup CBR Application"
set i 0
set cbr($i) [new Application/Traffic/CBR]
$cbr($i) attach-agent $udp($i)
$cbr($i) set packetSize_ $pktsize
$cbr($i) set interval_ $pktrate
$ns_ at 1.0 "$cbr($i) start"
$ns_ at 40.0 "$cbr($i) stop"
set i 1
set cbr($i) [new Application/Traffic/CBR]
$cbr($i) attach-agent $udp($i)
$cbr($i) set packetSize_ $pktsize
$cbr($i) set interval_ $pktrate
$ns_ at 1.0 "$cbr($i) start"
$ns_ at 40.0 "$cbr($i) stop"
puts "Setup CBR Application over UDP connection"

$ns_ at 7.0 "$n(3) setdest 1155 549 800"
#$ns_ at 10.0 "$n(6) setdest 1185 735 800"
#$ns_ at 13.0 "$n(8) setdest 1027 480 800"


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


