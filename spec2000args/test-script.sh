

#cache size decrease
#c_1="-cache:dl1 dl1:256:32:1:l -cache:dl2 dl2:1024:64:2:l -cache:dl3 dl3:4096:64:4:l"
#c_2="-cache:dl1 dl1:128:32:1:l -cache:dl2 dl2:512:64:2:l -cache:dl3 dl3:2048:64:4:l"
c_1="-cache:dl1 dl1:512:32:1:l -cache:dl2 dl2:2048:64:2:l -cache:dl3 dl3:4096:64:4:l" 
c_2="-cache:dl1 dl1:256:32:1:l -cache:dl2 dl2:1024:64:2:l -cache:dl3 dl3:2048:64:4:l" 
c_3="-cache:dl1 dl1:128:32:1:l -cache:dl2 dl2:512:64:2:l -cache:dl3 dl3:1024:64:4:l" 
c_4="-cache:dl1 dl1:64:32:1:l -cache:dl2 dl2:128:64:2:l -cache:dl3 dl3:512:64:4:l"
c_5="-cache:dl1 dl1:32:32:1:l -cache:dl2 dl2:64:64:2:l -cache:dl3 dl3:128:64:4:l"
#associativity
c_6="-cache:dl1 dl1:256:32:2:l -cache:dl2 dl2:1024:64:4:l -cache:dl3 dl3:4096:64:8:l"
c_7="-cache:dl1 dl1:256:32:4:l -cache:dl2 dl2:1024:64:8:l -cache:dl3 dl3:4096:64:16:l"
#c_10="-cache:dl1 dl1:256:32:4:l -cache:dl2 dl2:1024:64:4:l -cache:dl3 dl3:4096:64:4:l"
#c_11="-cache:dl1 dl1:256:32:8:l -cache:dl2 dl2:1024:64:8:l -cache:dl3 dl3:4096:64:8:l"
#c_12="-cache:dl1 dl1:64:32:4:l -cache:dl2 dl2:256:64:4:l -cache:dl3 dl3:1024:64:4:l"
#block size
c_8="-cache:dl1 dl1:128:16:1:l -cache:dl2 dl2:512:32:2:l -cache:dl3 dl3:1024:32:4:l"
#c_9="-cache:dl1 dl1:128:64:1:l -cache:dl2 dl2:512:64:2:l -cache:dl3 dl3:1024:16:4:l"
echo $c_s_1

#cd vortex

#test 1
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_1} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_1.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_2} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_2.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_3} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_3.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_4} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_4.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_5} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_5.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_6} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_6.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_7} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_7.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_8} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_8.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_9} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_9.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_10} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_10.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_11} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_11.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_12} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_12.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_13} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_13.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_14} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_14.txt

#cd ..

cd lucas

./RUNlucas /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_1} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_1.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_2} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_2.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_3} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_3.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_4} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_4.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_5} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_5.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_6} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_6.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_7} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_7.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_8} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_8.txt
#./RUNlucas /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_9} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_9.txt
#./RUNlucas /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_10} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_10.txt



cd ..
cd swim

./RUNswim /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_1} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_1.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_2} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_2.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_3} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_3.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_4} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_4.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_5} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_5.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_6} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_6.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_7} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_7.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_8} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_8.txt
#./RUNswim /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_9} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_9.txt
#./RUNswim /home/jai/Desktop/simplescalar/simplesim-3.0/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_10} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_Orig_10.txt











