Linux内核学习之三-进程的调度
=====
## 一、调度的总体流程

进程的调度是进程部分的核心-很显然，如果没有调度，我们也不需要进程了！我们在上一篇文章的第二部分实现了一个最简单的按照时间片的调度算法，每个进程都平均执行100毫秒。

```java
 while (true){
     processing = nextProcessing();
     processing.run();
     wait(100); //每100毫秒分片
     processing.interrupt();
 }
 ```

那么Linux中如何实现的呢？我们先来看流程。调度相关的代码都在`sched.c`中。这个就是Linux代码核心中的核心，它被运行在亿万台机器上，每台机器每个时钟周期就要执行一次，看到它是不是有点激动？终于知道“高性能的底层代码”长什么样了！

这个文件的核心函数是`asmlinkage void __sched schedule(void)`，这就是调度部分的具体代码。当我读完并注释之后才发现已经有很多注释版本了，比如这篇文章：[http://blog.csdn.net/zhoudaxia/article/details/7375836](http://blog.csdn.net/zhoudaxia/article/details/7375836)，所以就不贴代码了。我注释后的代码在[sched.c](https://github.com/code4craft/os-learning/blob/master/linux/kernel/sched.c)(4079行开始)里。不过不读源码，不用那些关键词去搜索，估计也找不到一些好文章，这也是一个学习的过程吧。

这个方法有两个重要的点，一个是`pick_next_task(rq);`，获取下一个可执行进程，它涉及到调度算法；一个是`context_switch(rq, prev, next);`，这就是所谓的“上下文切换”了。

## 二、调度算法

我们前面的“100毫秒算法”算法当然是非常粗糙的。在了解货真价实的Linux调度算法时，不妨看看，调度系统需要考虑什么问题（非官方不权威总结）：

1. 最大限度利用CPU，只要有进程能执行，就不让要CPU空等。尽量最大化CPU利用率。
2. 保证进程（特别是交互式进程）的响应时间尽可能短。
3. 能由系统管理员指定优先级，让重要的任务先执行。
4. 因为调度执行非常频繁，所以必须考虑它的性能。
5. 支持多核平均调度，也就是所谓的对称多处理器（Symmetric Multi-Processor，SMP）。

我们的“100毫秒算法”不满足1和3，对于2来说其实也不太好（可能有些进程都不会执行那么久）。如果我们把时间缩短，换成1毫秒怎么样呢？我们知道，“进程切换”本身也有开销，这样子频繁切换，岂不是得不偿失了？

实际上，因为其核心地位，Linux的调度算法一旦提升一点点性能，对整个工业界的提升也是巨大的。对于算法高手来说，这里成了大显身手的好地方。所以Linux调度算法的变化那是相当的快，从[O(n)调度器](http://en.wikipedia.org/wiki/O(n)_scheduler)到[O(1)调度器](http://en.wikipedia.org/wiki/O\(1\)_scheduler)，再到2.6.23中的"[CFS（completely fair schedule）](http://zh.wikipedia.org/wiki/%E5%AE%8C%E5%85%A8%E5%85%AC%E5%B9%B3%E6%8E%92%E7%A8%8B%E5%99%A8)"，让人看得都晕了！

了解了要解决的问题，或许会更容易理解一点。O(n)和O(1)算法都是基于时间片的，基本思路就是：给进程指定优先级，IO高、交互强的进程给予更高的优先级，CPU占用高的则降低优先级，每次选优先级最高的执行；同时为每个进程分配时间片（每个进程的时间片都是动态调整的），每个进程每次执行的时间就是这个时间片的时间。O(n)和O(1)的区别在于从优先级队列里取进程的时候的时间复杂度而已。具体细节就不多说了。

而"CFS"则是使用了一个"vruntime"的概念来保存执行时间。同时它用一颗红黑树来对进程做排序，vruntime越小的进程会被越先执行，所以它的时间复杂度是O(logn)。它的代码在`kernel/sched_fair.c`中。

另外还有个“实时调度算法”的概念。这些就是“加塞的”的进程，它们优先于CFS的所有进程。对应的类型是`SCHED_FIFO`和`SCHED_RR`，在`sched.h`中可以看到。

调度部分就这么多，还有些细节，例如CFG具体实现，书里已经很详细了，就不重复记录，免得写晕了！

## 参考资料：

* 《Linux内核设计与实现》 LKD
* Linux 2.6内核中新的锁机制--RCU [http://www.ibm.com/developerworks/cn/linux/l-rcu/](http://www.ibm.com/developerworks/cn/linux/l-rcu/)
* Linux进程调度(3)：进程切换分析 [http://blog.csdn.net/zhoudaxia/article/details/7375836](http://blog.csdn.net/zhoudaxia/article/details/7375836)
* [http://blog.csdn.net/yunsongice/article/details/8547107](http://blog.csdn.net/yunsongice/article/details/8547107)