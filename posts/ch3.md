Java码农学Linux内核之二-进程与线程（中）
=====
上篇文章算是对进程有了个基本了解，这次我们来看看进程的生命周期和调度。调度相关的代码都在`sched.c`中。这次来啃个硬骨头，直接从`sched.c`看起。



参考资料：

* Linux 2.6内核中新的锁机制--RCU [http://www.ibm.com/developerworks/cn/linux/l-rcu/](http://www.ibm.com/developerworks/cn/linux/l-rcu/)