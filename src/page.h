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