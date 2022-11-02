echo "Transfering to NetEdu"
echo "torvalds" | sshpass -v scp -r ** netedu@192.168.109.24:~/Desktop/mari_i_lima_protocol

echo "Transfering to Root"
sshpass -v -p "alanturing" scp -r -oProxyCommand="sshpass -p torvalds ssh -W %h:%p netedu@192.168.109.24" ** root@172.16.2.43:~/Desktop/mari_i_lima_protocol