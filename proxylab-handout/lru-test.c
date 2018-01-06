/*常见的缓存算法---lru(c工具箱/c++ stl)
*LRU(Least Recently Used),
*LFU(Least frequently Used)
*FIFO
*浏览器的缓存策略、memcached的缓存策略都是使用LRU这个算法
*LRU算法的三大特点(双向链表+哈希表):
*1.new data 插在表头
*2.当缓存命中,将数据移到表头
*3.当表满时,删除表尾(此时是按时间顺序排列的)
*/
#include <iostream>
#include <list>
#include <map>
#include <vector>
#include <queue>
#include <set>
#include <stack>
#include <array>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
/* #include <uthash.h>   UThash 的数据结构为c语言的hash表实现*/

struct CacheNode
{
    int key;
    int value;
    CacheNode *pre, *next;
    //CacheNode(int k,int v) : key(k),value(v),pre(null),next(null);
};

//create LRU
int LRUCacheCreate(int capacity,);
//destroy LRU
int LRUCacheDelet();
//把key对应的val值存入cache中.
int LRUCacheSet();
//从cache中得到val
char LRUCacheGet();
//show 缓存的内容
void LRUCacheShow();

