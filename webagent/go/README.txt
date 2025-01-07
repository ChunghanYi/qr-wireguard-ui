<How to build>
==============
go mod init webagent
go mod tidy
go build or make

<How to run>
============
./web-agentd -f

<How to check logs>
===================
tail -f /var/log/webagent.log
