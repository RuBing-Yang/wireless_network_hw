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
set val(x) 700 ;# X dimension of topography
set val(y) 700 ;# Y dimension of topography
set val(stop) 42 ;# nam stop time
set val(nn) 12 ;# number of mobilenodes
set val(nnoise) 1 ;# number of noise nodes
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
set ns [new Simulator]
# Setup topography object
set topo [new Topography]
$topo load_flatgrid $val(x) $val(y)
set god_ [create-god [expr $val(nn)*$val(nc)]]
# Open the NS trace file
set tracefd [open $filename.tr w]
$ns trace-all $tracefd
$ns use-newtrace
# Open the NAM trace file
set namfile [open $filename.nam w]
$ns namtrace-all $namfile
$ns namtrace-all-wireless $namfile $val(x) $val(y)
# Create wireless channels
for {set i 0} {$i < $val(nc)} {incr i} {
    set chan($i) [new $val(chan)]
}


#======================================
# Mobile Node Parameter Setup
#======================================
$ns node-config -adhocRouting $val(rp) \
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
$ns change-numifs $val(nc)
for {set i 0} {$i < $val(nc)} {incr i} {
    $ns add-channel $i $chan($i)
}
# Create nodes
puts "begin to create nodes"
for {set i 0} {$i < $val(nn)} {incr i} {
    set n($i) [$ns node]
    $god_ new_node $n($i)
}
puts "created nodes"

# Set node positions in horizontal chain topology
 
  
$n(0) set X_ 467
$n(0) set Y_ 386
$n(0) set Z_ 0.0
$ns initial_node_pos $n(0) 20
  
$n(1) set X_ 558
$n(1) set Y_ 496
$n(1) set Z_ 0.0
$ns initial_node_pos $n(1) 20
  
$n(2) set X_ 646
$n(2) set Y_ 305
$n(2) set Z_ 0.0
$ns initial_node_pos $n(2) 20
  
$n(3) set X_ 719
$n(3) set Y_ 475
$n(3) set Z_ 0.0
$ns initial_node_pos $n(3) 20
  
$n(4) set X_ 861
$n(4) set Y_ 403
$n(4) set Z_ 0.0
$ns initial_node_pos $n(4) 20
  
$n(5) set X_ 852
$n(5) set Y_ 262
$n(5) set Z_ 0.0
$ns initial_node_pos $n(5) 20
  
$n(6) set X_ 1024
$n(6) set Y_ 351
$n(6) set Z_ 0.0
$ns initial_node_pos $n(6) 20
  
$n(7) set X_ 698
$n(7) set Y_ 654
$n(7) set Z_ 0.0
$ns initial_node_pos $n(7) 20
  
$n(8) set X_ 847
$n(8) set Y_ 554
$n(8) set Z_ 0.0
$ns initial_node_pos $n(8) 20
  
$n(9) set X_ 990
$n(9) set Y_ 488
$n(9) set Z_ 0.0
$ns initial_node_pos $n(9) 20
  
$n(10) set X_ 1023
$n(10) set Y_ 621
$n(10) set Z_ 0.0
$ns initial_node_pos $n(10) 20
  
$n(11) set X_ 889
$n(11) set Y_ 721
$n(11) set Z_ 0.0
$ns initial_node_pos $n(11) 20


#======================================
# UDP Agents Definition
#======================================
set last_node_id [expr $val(nn)-1]

puts "begin to definite UDP Agent"
for {set i 0} {$i < $val(nm)} {incr i} {
    set udp($i) [new Agent/UDP]
    $ns attach-agent $n(0) $udp($i)
    set null($i) [new Agent/Null]
    $ns attach-agent $n(11) $null($i)
    $ns connect $udp($i) $null($i)
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
    $ns at 1.0 "$cbr($i) start"
    $ns at 40.0 "$cbr($i) stop"
}
puts "Setup CBR Application over UDP connection"

# n(3) move and link(n(3)-n(4)) break
#$ns at 10.0 "$n(5) setdest 280 100 200"
$ns at 20.0 "$n(3) setdest 400 600 200"
# link 3->4 should change to 3->6->4


#======================================
# Simulation Termination
#======================================
# Define a finish procedure
proc finish {} {
    global ns tracefd filename pktsize last_node_id
    global namfile
    $ns flush-trace
    close $tracefd
    close $namfile
    exec nam $filename.nam &
    # Call throughput analyzer (AWK scripts written by Marco Fiore, marco.fiore@polito.it)
    # exec awk -f avgStats.awk src=0 dst=$last_node_id flow=0 pkt=$pktsize $filename.tr &
    exit 0
}
for {set i 0} {$i < $val(nn)} { incr i } {
    $ns at $val(stop) "$n($i) reset"
} 
$ns at $val(stop) "$ns nam-end-wireless $val(stop)"
$ns at $val(stop) "finish"
$ns at $val(stop) "puts \"done\" ; $ns halt"
$ns run


