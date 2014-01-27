Java码农学Linux内核之三-进程与线程（中）
=====

上篇文章算是对进程有了个基本了解，这次我们来点硬的，看看进程调度的源码。关于调度方面的理论，《Linux内核设计与实现》（Linux Kernel Development，简称LKD）已经比较全面了。本文没有那么面面俱到，主要就我在代码里理解的部分来讲。

下面直接看代码，调度相关的代码都在`sched.c`中。这个就是Linux代码核心中的核心，它被运行在亿万台机器上，每台机器每个时钟周期就要执行一次，看到它是不是有点激动？终于知道“高性能的底层代码”长什么样了！

这个类的核心函数是`asmlinkage void __sched schedule(void)`，这就是调度部分的具体代码。当我读完并注释之后才发现已经有很多注释版本了，比如这篇文章：[http://blog.csdn.net/zhoudaxia/article/details/7375836](http://blog.csdn.net/zhoudaxia/article/details/7375836)，所以就不贴代码了。我注释后的代码在[sched.c](https://github.com/code4craft/os-learning/blob/master/linux/kernel/sched.c)里。不过不读源码，不用那些关键词去搜索，估计也找不到一些好文章，这也是一个学习的过程吧。

还有一个核心函数是`void scheduler_tick(void)`，它由时钟中断调用，每个时钟周期会被调用一次，用于重新计算进程的时间。

最后还有一个就是优先队列的实现，优先级和调度算法的实现也有很多种，先暂时囫囵吞枣了。

吐槽一下：内核这个东西，研究的人太多了，自己也没想专业做这个，有点丧失动力啊！理论方面没办法比LKD讲的更好了，我觉得先通读一遍，再继续深入进程部分。

参考资料：

* 《Linux内核设计与实现》 LKD
* Linux 2.6内核中新的锁机制--RCU [http://www.ibm.com/developerworks/cn/linux/l-rcu/](http://www.ibm.com/developerworks/cn/linux/l-rcu/)
* Linux进程调度(3)：进程切换分析 [http://blog.csdn.net/zhoudaxia/article/details/7375836](http://blog.csdn.net/zhoudaxia/article/details/7375836)