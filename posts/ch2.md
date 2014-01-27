Java码农学Linux内核之二-进程与线程（上）
=====

## 一、操作系统的功能

根据维基百科的解释，一个操作系统大概包括以下几个功能：

1. 进程管理（Processing management）
2. 安全机制（Security）
3. 内存管理（Memory management）
4. 用户界面（User interface）
5. 文件系统（File system）
6. 驱动程序（Device drivers）
7. 网络通信（Networking）

可以看到，这些功能彼此之间关系并不大，所以操作系统其实是这么些功能的低内聚复合体。所以学习的时候，采用逐个击破的方式，要比囫囵吞枣，一下全看来的好。进程管理是操作系统最最核心的功能，我们就从这里开始。

## 二、什么是进程

什么是进程？这个问题不太好回答，我们不妨从另一个角度来看看这个问题。

最早的操作系统是没有“进程“这个概念的，比如[DOS](http://zh.wikipedia.org/wiki/DOS)。正如其名“Disk Operating System”，DOS主要的任务就是管理一下磁盘，并把BIOS和硬件做一点抽象。在上面开发的程序，其实是直接跟CPU打交道的，程序最终会编译或者解释成CPU指令并被执行。

这个系统有个最大的问题，就是同时只能执行一个程序。这样对于用户使用无疑太不友好了，我想一边听音乐一边写代码都做不到！怎么办呢？CPU就像一个无脑的工人，在它那里根本没有“程序”的概念，只负责处理“指令”，所以如果我们程序不做点事情，那么好像无论如何都无法实现“多个程序同时执行”吧？

于是，就有了“分时多进程”的操作系统。操作系统把想要执行的程序管理起来，并且按照一定的规则，让它们都得到执行。因为CPU执行很快，所以对用户看起来它们就像同时在执行一样，这就是所谓的“进程“。

在这里我想强调一个观点：**操作系统也是一种程序**，这样子会对于码农距离感更小一点。作为一个喜欢动手的码农，我们不妨想想如何实现分时间片调度？我写了一段最简单的调度算法，大概是这样子（wait这里其实依赖时钟周期相关的东东，但不妨先假设一下）：

```java
 while (true){
     processing = nextProcessing();
     processing.run();
     wait(100); //每100毫秒分片
     processing.interrupt();
 }
 ```
 
 而`nextProcessing()`则可以使用一个FIFO的循环队列来保存，这样子是不是有点意思了？（实际上Linux的调度算法要比这个复杂很多，但是也没有到无法理解的程度，这个我们下篇文章再说。）
 
总结一下这部分：进程就是程序执行的一个实例。它是操作系统为了管理多个程序的执行而产生的机制。
 
## 三、Linux中的进程

废话很多了，来点干货吧！Linux内核中进程相关的代码在`include/linux/sched.h`和`kernel/sched.c`里。（本文针对2.6.39版本的内核）

*插一句，Linux的项目结构大概包括三部分：*

*1. `include`是对外发布的部分，看到代码中有`#include <linux/config.h>`这样的，就是在"include"目录中。*

*2. `kernel`则是内核部分的实现，`arch`是对不同平台的适配。*

*3. 其他目录多是功能模块，例如初始化`init`，文件系统`fs`，内存管理`mm`等。*

记得有句名言叫做“程序=算法+数据结构”。我在写一般业务逻辑的时候，总觉得不完全符合，但是看Linux代码的时候，确确实实感觉到了这句话的道理。

进程相关的最重要的数据结构，我觉得有两个。一个是`task_struct`（在`sched.h`中），就是我们常说的“进程描述符”，用于标识一个进程，以及保存上下文信息。弄懂这个可以算是成功了一半了！

task_struct是个巨大的结构体，光定义就有好几百行，有很多`#ifdef`括起来的可选功能。有些功能需要到后面才能看懂，这里主要说几个部分：

```c
struct task_struct {

    /* 执行状态，具体的状态见TASK_RUNNING等一系列常量定义 */
	volatile long state;

	/* 执行中的标志位，具体内容见PF_* 系列常量定义 */
	unsigned int flags;

    /* 优先级，用于调度 */
    int prio, static_prio, normal_prio;

    /* 进程使用的内存 */
    struct mm_struct *mm, *active_mm;
}
```

感觉细节仍然不明白？其实我也不明白，不过好歹算是懂了个大概！

## 四、进程与线程

说完了进程，我们来理一理线程的概念。其实线程可以理解为特殊的进程，它没有独立的资源（对，就是上面的*mm），它依赖于进程，某个进程的子线程之间可以共享资源，除此之外没有什么区别。

在c程序里，我们使用fork来创建进程。例如：

```c
#include <stdio.h>
#include <unistd.h>
int main(int argc, const char* argv[]) {
  pid_t pid;
  printf("Hello, World!%d\n",pid);
  for (int i=0;i<2;i++){
      pid = fork();
      if (pid == 0){
        printf("I am child");
      } else {
        printf("I am parent, my child is %d",pid);
      }
  }
  return 0;
}
```

这里fork会创建一份新的进程。这个进程会复制当前进程的所有上下文，包括寄存器内容、堆栈和内存（现在内存一般使用Copy-On-Write机制，不过我觉得对于用户来说觉得它是复制了一份也没什么问题）。因为程序计数器也一起复制了，所以执行到哪一步也会被复制下来。

fork的实现在`kernal/fork.c`里。

```c
long do_fork(unsigned long clone_flags,
	      unsigned long stack_start,
	      struct pt_regs *regs,
	      unsigned long stack_size,
	      int __user *parent_tidptr,
	      int __user *child_tidptr)
```

到底是创建线程还是进程，取决于`clone_flags`传入的参数。

下篇文章分析一下进程的生命周期及调度。

参考资料：

* [http://blog.csdn.net/hongchangfirst/article/details/7075026](http://blog.csdn.net/hongchangfirst/article/details/7075026)
* [http://zh.wikipedia.org/wiki/DOS](http://zh.wikipedia.org/wiki/DOS)
* 《Linux内核设计与实现》
* 《深入理解Linux内核》