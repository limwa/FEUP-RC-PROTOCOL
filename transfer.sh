echo "Transfering to NetEdu"
echo "torvalds" | sshpass -v scp -r ** netedu@192.168.109.24:~/Desktop/mari_i_lima_protocol
echo "torvalds" | sshpass -v ssh netedu@192.168.109.24 "cd Desktop/mari_i_lima_protocol; rm -rf moodle/ tmp/ media/* bin/*"

echo "Transfering to Root"
sshpass -v -p "alanturing" scp -r -oProxyCommand="sshpass -p torvalds ssh -W %h:%p netedu@192.168.109.24" ** root@172.16.2.43:~/Desktop/mari_i_lima_protocol
sshpass -v -p "alanturing" ssh -oProxyCommand="sshpass -p torvalds ssh -W %h:%p netedu@192.168.109.24" root@172.16.2.43 "cd Desktop/mari_i_lima_protocol; rm -rf moodle/ tmp/ bin/*" 