Linux内核学习之四-内存管理
====

## 一、虚拟存储器

内存管理最基础的概念，恐怕是虚拟存储器（Virtual Memory，简称VM）了，它是计算机系统（注意我没写操作系统，因为其中还有部分硬件功能）在物理存储之上的一套机制。它将物理地址（Physical Address，简称PA）转换为虚拟地址（Virtual Address，简称VA），来访问存储。正是因为有了虚拟存储器，才有了后面的内存转换、页表等机制。

看到这里我就有疑问了，有了物理地址，程序已经能定位到内存块并且使用了，为什么需要有虚拟地址？

实际上，如果只有一个程序在内存中运行，没有虚拟存储问题也不大，比如DOS（又提到它了！），而且据说DOS确实是没有虚拟存储机制的。但是当有了多进程之后，问题就出现了：多个进程共同使用一整个物理内存，既不安全也不方便，比如A用了`0xb7001008`，结果B没法知道，然后也用了它，岂不是乱套了？

为了解决这个问题，就有了虚拟存储机制。对于每个进程来说，它的虚拟地址空间总是一样的，但是实际使用的物理内存是分开的，而且它也不知道到底是在使用虚拟地址，还是物理地址，反正用就对了！这样子既简化了程序开发，又增加了安全性，真是非常巧妙的设计！

总结一下，虚拟存储的最大作用就是**隔离与抽象**。

## 二、地址转换的实现

为了了解地址的转换，我们必须引入“页”（page）的概念。其实页就是一块连续的内存，这也是操作系统利用内存的最小单位。更直观一点的说，在Linux里，它是这么实现的：

```c
struct page{
    //保存状态
    unsigned long flags;
    //引用计数
    atomic_t _count;
    atomic_t _mapcount;
    unsigned long private;
    struct address_space *mapping;
    pgoff_t index;
    struct list_head lru;
    //指向虚拟地址
    void *virtual;
}
```

`page`结构体保存对应页的引用数、虚拟地址等信息。因为这个结构本身也是消耗内存的，所以一页的大小太小，那么还要存`page`结构，就有很多内存浪费了，很不划算。如果页大小太大，经常会出现一个页装不满的情况，也是我们不愿看到的。在32位CPU里，一页是4KB。

有了页的知识，地址转换就可以进行了。在看这部分之前，不妨先想想，如果让我们实现一套地址转换，会怎么做？

似乎这没有什么难度啊？可以分两部分，你看我伪代码都写好了：

1. 保存虚拟地址到物理地址的映射关系

		page_table={virtual_address:physical_address}

2. 在程序中使用指针访问物理地址的时候，对其进行转换

		physical_address=page_table[virtual_address]	
是不是非常简单？

实际上目前的计算机系统做的方式也差不多。但是区别是，因为这两个操作非常频繁，光靠软件实现性能未必有那么好，所以这两部分有了一些硬件上的优化。

把VA转换为PA的事情，由CPU里一个专门的部件来完成，它叫做“内存管理单元”（Memory Management Unit，简称MMU）。

保存地址映射关系的部件，叫做页表（Page Table），它是保存在内存中的，由操作系统维护。但是访问一次内存还要查一次内存，这事感觉不太科学，所以MMU还会维护一份用过的页表索引，这就是传说中的TLB（Translation Lookaside Buffer，也叫转换备用缓冲区，我们学校的孙钟秀院士在他的教材中将其翻译为“快表”）。

所以最后的流程是：

1. 操作系统新建进程时，为进程分配内存空间，并更新页表；
2. 该进程的指令到CPU之前，其中的虚拟地址，会触发MMU转换流程；
3. MMU先到TLB中找页表，找不到再去物理内存中找页表，最后转换为物理地址，递给CPU执行。

这其实也是一个操作系统反过来影响CPU设计的案例，这也说明，其实硬件跟系统分界并不是死的，比如有些CPU的指令集也会包括一些高等的操作，理论上越底层越快，所以到底放在哪一层，主要取决于这个机制的价值和通用性。

至此地址转换算是差不多了，操作系统和MMU握了个手，合作愉快！

## 三、Linux中的内存管理

Linux中内存分配相关的代码在[`kernal/page_alloc.c`](https://github.com/code4craft/os-learning/blob/master/linux/mm/page_alloc.c)中，其中核心的函数是`struct page * __alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order, struct zonelist *zonelist, nodemask_t *nodemask)`。我对能看懂的代码做了注释，大部分都没看懂，等全部都研究一遍之后再来细细研究吧…
			
Linux内存管理中还有很多细节，罗列几个，做个备忘。

1. 分区（zone）

	Linux将内存分为几个区：ZONE_DMA、ZONE_NORMAL和ZONE_HIGHEN。DMA（Direct Memory Access）是一种IO直接操作内存的技术，有些硬件只能用特定地址的内存来进行DMA，对这种内存需要标记一下。
	
2. NUMA

	NUMA（Non-Uniform Memory Access Architecture）是相对于UMA（Uniform Memory Access Architecture）来说的。UMA就是指多处理器共享一片内存，而NUMA则反其道行之，将CPU绑定到一些内存中，从而加快速度。据说在多于八核的处理器中效果明显。
	
3. slab

	slab这部分LKD讲的并不好，有些绕弯（也可能是翻译不好吧），然后我搜到很多资料，大致连描述都照样复制，我也不知道是不是我智商太低，弄不懂，还是作者只是做了个摘抄，反正对于这些技术文章只能呵呵了。
	
	其实slab解决了什么问题呢？我们知道在内核里有些数据结构是很常用的，例如inode，这些数据结构会频繁初始化和销毁。但是初始化数据结构是有开销的啊，更好的办法是把它存下来，然后下次创建的时候，直接拿一个现成的，改改内容，就可以用了！slab又译作“板坯”，这样子是不是好理解一点呢？
	
	在实现上，slab会为一类对象开辟一段空间，存储多个这样的对象，然后创建和销毁，其实只是在这片空间里指针移动一下的事情了！我们其实可以叫它“对象池”或者“结构池”吧！slab的代码实现在LKD中有非常详细的描述，不再赘述了。

参考资料：

* 文中讲到的LKD指《Linux内核设计与实现》（Linux Kernel Development）
* 《深入理解计算机系统》
* [http://learn.akae.cn/media/ch17s04.html](http://learn.akae.cn/media/ch17s04.html)
* [http://www.cnblogs.com/shanyou/archive/2009/12/26/1633052.html](http://www.cnblogs.com/shanyou/archive/2009/12/26/1633052.html)
* The Slab Allocator:An Object-Caching Kernel Memory Allocator [http://www.usenix.org/publications/library/proceedings/bos94/full_papers/bonwick.ps](http://www.usenix.org/publications/library/proceedings/bos94/full_papers/bonwick.ps)
