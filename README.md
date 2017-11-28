# tcp-over-udp
## 在UDP的基础上实现TCP build tcp connection from bottom using basic udp

* 实现了TCP三次握手，文件传输，以及通过加载TCP header来控制传输状态等功能。Have implemented tcp 3-way-handshake, file transmission, and status control using tcp header
### 打开方式指南 Instruction
	git clone https://github.com/Universeyi/tcp-over-udp.git
	cd tcp-over-udp
	gcc -lsocket -lm -lnsl myClientTCP.c -o tcp_client
	gcc -lsocket -lm -lnsl myServerTCP.c -o tcp_server
	tcp_server 2333 pp.txt
另开一个窗口 open a new tab of your terminal

	tcp_client 127.0.0.1 2333 people.txt

可由此观察程序运行结果 Then you can view the outcome of our program.