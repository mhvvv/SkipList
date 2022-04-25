# Skip List

基于跳表实现的轻量级键值型KV存储引擎，使用C++实现。

## 接口

* 插入记录 - insert_element(const K&, const V&)
* 删除记录 - delete_element(const K&)
* 查找记录 - search_element(const K&);
* 展示记录 - display_list()
* 数据加载 - load_file();
* 数据存盘 - dump_file()
* 记录数量 - size()

插入数据时会使用此方法决定最高插入到第几级索引(层)
```C++
template<typename K, typename V>
int SkipList<K, V>::get_random_level() {
/*
    该 randomLevel 方法会随机生成 1~MAX_LEVEL 之间的数，且 ：
       1/2 的概率返回 0   => No
       1/4 的概率返回 1   => 一级索引
       1/8 的概率返回 2 以此类推
 */
    int level = 0;
    while(rand() % 2 && level < max_level) 
        ++level;

    return level;
}
```

## 致谢

https://www.jianshu.com/p/9d8296562806

https://github.com/youngyangyang04/Skiplist-CPP

