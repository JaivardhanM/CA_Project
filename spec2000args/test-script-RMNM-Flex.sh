

#cache size decrease
#c_1="-cache:dl1 dl1:256:32:1:l -cache:dl2 dl2:1024:64:2:l -cache:dl3 dl3:4096:64:4:l"
#c_2="-cache:dl1 dl1:128:32:1:l -cache:dl2 dl2:512:64:2:l -cache:dl3 dl3:2048:64:4:l"
c_1="-cache:il1 il1:512:32:8:t:1:8192:131072:1 -cache:dl1 dl1:512:32:1:l -cache:dl2 dl2:2048:64:2:l -cache:dl3 dl3:4096:64:4:l" 
c_2="-cache:il1 il1:256:32:8:t:1:8192:131072:1 -cache:dl1 dl1:256:32:1:l -cache:dl2 dl2:1024:64:2:l -cache:dl3 dl3:2048:64:4:l" 
c_3="-cache:il1 il1:128:32:8:t:1:8192:131072:1 -cache:dl1 dl1:128:32:1:l -cache:dl2 dl2:512:64:2:l -cache:dl3 dl3:1024:64:4:l" 
#c_4="-cache:il1 il1:1024:32:8:t:1:8192:131072:1 -cache:dl1 dl1:64:32:1:l -cache:dl2 dl2:128:64:2:l -cache:dl3 dl3:512:64:4:l"
#c_5="-cache:il1 il1:1024:32:8:t:1:8192:131072:1 -cache:dl1 dl1:32:32:1:l -cache:dl2 dl2:64:64:2:l -cache:dl3 dl3:128:64:4:l"
#associativity
c_6="-cache:il1 il1:1024:32:16:t:1:8192:131072:1 -cache:dl1 dl1:512:32:16:l -cache:dl2 dl2:1024:64:4:l -cache:dl3 dl3:4096:64:8:l"
c_7="-cache:il1 il1:1024:32:4:t:1:8192:131072:1 -cache:dl1 dl1:512:32:4:l -cache:dl2 dl2:1024:64:8:l -cache:dl3 dl3:4096:64:16:l"
#c_10="-cache:dl1 dl1:256:32:4:l -cache:dl2 dl2:1024:64:4:l -cache:dl3 dl3:4096:64:4:l"
#c_11="-cache:dl1 dl1:256:32:8:l -cache:dl2 dl2:1024:64:8:l -cache:dl3 dl3:4096:64:8:l"
#c_12="-cache:dl1 dl1:64:32:4:l -cache:dl2 dl2:256:64:4:l -cache:dl3 dl3:1024:64:4:l"
#block size
c_8="-cache:il1 il1:1024:16:8:t:1:8192:131072:1 -cache:dl1 dl1:512:16:1:l -cache:dl2 dl2:1024:32:2:l -cache:dl3 dl3:4096:32:4:l"
#c_9="-cache:il1 il1:1024:8:8:t:1:8192:131072:1 -cache:dl1 dl1:512:8:1:l -cache:dl2 dl2:512:16:2:l -cache:dl3 dl3:1024:16:4:l"
echo $c_s_1


#./RUNcrafty /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 -cache:il1 il1:1024:32:8:t:1:8192:131072:1 -cache:dl1 dl1:1024:32:1:l -cache:dl2 dl2:2048:64:2:l -cache:dl3 dl3:4096:64:4:l -max:inst 50000000 -redir:sim cache_RMNM-Flex_Pwr.txt^C



cd vortex

#test 1
./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_1} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_1.txt
./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_2} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_2.txt
./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_3} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_3.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_4} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_4.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_5} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_5.txt
./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_6} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_6.txt
./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_7} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_7.txt
./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_8} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_8.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_9} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_9.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_10} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_10.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_11} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_11.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_12} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_12.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_13} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_13.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/vortex00.peak.ev6 ${c_14} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_14.txt

cd ..

cd crafty

#test 2
./RUNcrafty /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 ${c_1} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_1.txt
./RUNcrafty /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 ${c_2} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_2.txt
./RUNcrafty /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 ${c_3} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_3.txt
#./RUNcrafty /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 ${c_4} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_4.txt
#./RUNvortex /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 ${c_5} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_5.txt
./RUNcrafty /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 ${c_6} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_6.txt
./RUNcrafty /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 ${c_7} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_7.txt
./RUNcrafty /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 ${c_8} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_8.txt
#./RUNcrafty /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 ${c_9} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_9.txt
#./RUNcrafty /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/crafty00.peak.ev6 ${c_10} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_10.txt


cd ..
cd equake

#test 3
./RUNequake /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/equake00.peak.ev6 ${c_1} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_1.txt
./RUNequake /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/equake00.peak.ev6 ${c_2} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_2.txt
./RUNequake /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/equake00.peak.ev6 ${c_3} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_3.txt
#./RUNequake /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/equake00.peak.ev6 ${c_4} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_4.txt
#./RUNequake /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/equake00.peak.ev6 ${c_5} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_5.txt
./RUNequake /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/equake00.peak.ev6 ${c_6} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_6.txt
./RUNequake /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/equake00.peak.ev6 ${c_7} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_7.txt
./RUNequake /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/equake00.peak.ev6 ${c_8} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_8.txt
#./RUNequake /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/equake00.peak.ev6 ${c_9} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_9.txt
#./RUNequake /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/equake00.peak.ev6 ${c_10} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_10.txt


#cd ..
#cd gzip

#test4equake
#./RUNgzip /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/gzip00.peak.ev6 ${c_1} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_1.txt
#./RUNgzip /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/gzip00.peak.ev6 ${c_2} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_2.txt
#./RUNgzip /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/gzip00.peak.ev6 ${c_3} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_3.txt
#./RUNgzip /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/gzip00.peak.ev6 ${c_4} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_4.txt
#./RUNgzip /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/gzip00.peak.ev6 ${c_5} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_5.txt
#./RUNgzip /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/gzip00.peak.ev6 ${c_6} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_6.txt
#./RUNgzip /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/gzip00.peak.ev6 ${c_7} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_7.txt
#./RUNgzip /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/gzip00.peak.ev6 ${c_8} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_8.txt
#./RUNgzip /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/gzip00.peak.ev6 ${c_9} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_9.txt
#./RUNgzip /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/gzip00.peak.ev6 ${c_10} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_10.txt


cd ..
cd lucas

./RUNlucas /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_1} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_1.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_2} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_2.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_3} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_3.txt
#./RUNlucas /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_4} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_4.txt
#./RUNlucas /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_5} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_5.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_6} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_6.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_7} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_7.txt
./RUNlucas /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_8} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_8.txt
#./RUNlucas /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_9} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_9.txt
#./RUNlucas /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/lucas00.peak.ev6 ${c_10} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_10.txt



cd ..
cd swim

./RUNswim /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_1} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_1.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_2} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_2.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_3} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_3.txt
#./RUNswim /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_4} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_4.txt
#./RUNswim /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_5} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_5.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_6} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_6.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_7} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_7.txt
./RUNswim /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_8} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_8.txt
#./RUNswim /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_9} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_9.txt
#./RUNswim /home/jai/Desktop/simplescalar/simplesim-RMNM-Flex/sim-outorder ../../spec2000binaries/swim00.peak.ev6 ${c_10} -max:inst 50000000 -fastfwd 20000000 -redir:sim cache_RMNM-Flex_10.txt








