使用System V消息队列实现的回射服务器.[原理图](https://images2015.cnblogs.com/blog/1058323/201611/1058323-20161129002148881-757894290.png)(上面1个为代码实现的,会出现死锁,下面1个为改进后解决死锁的回射服务器)
## 1. 问题
使用1个队列来实现回射服务,会出现死锁.比如说大量客户端同时发送数据导致队列满了,则会出现死锁
1. 改进:使用多个队列