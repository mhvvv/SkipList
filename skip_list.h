#ifndef SKIP_LIST_H
#define SKIP_LIST_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <vector>

#define FILE_PATH "./dumpFile"

template<typename K, typename V>
class Node {
private:
    K key;
    V value;
    int level;

public:
    Node() = default;
    Node(const K&, const V&, int);

    K get_key() const;
    V get_value() const;

    void set_value(const V&);

    /*
     * 表示当前节点位置的下一个节点所有层的数据，从上层切换到下层，就是数组下标-1，
     * forwards[3]表示当前节点在第三层的下一个节点。
     */
    std::vector<Node<K, V>* > forward;
};

template<typename K, typename V>
class SkipList {
private:
    int max_level;          // 最大索引级别 
    int skip_list_level;    // 当前所处索引级别
    int element_count;      // 结点数

    Node<K, V>* header;     // 头节点
    
    // 文件操作
    std::ofstream file_writer;     
    std::ifstream file_reader;  

public:
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<K, V>* create_node(const K&, const V&, int);
    void display_list();
    bool search_element(const K&);
    bool delete_element(const K&);
    /* 添加元素, 成功返回true, 失败给出错误信息并返回false */
    bool insert_element(const K&, const V&);
    void dump_file();
    void load_file();
    int size();

private:
    void get_key_value_from_string(const std::string&, std::string*, std::string*);
    bool is_valid_string(const std::string&);

public:
    std::mutex mtx;
    std::string delimiter = ":";
};

void error_handing(const std::string&);

template<typename K, typename V>
Node<K, V>::Node(const K& k, const V& v, int _level) : key(k), value(v), level(_level) {
    this->forward.resize(level + 1, nullptr);
}

template<typename K, typename V>
K Node<K, V>::get_key() const {
    return key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const {
    return value;
}

template<typename K, typename V>
void Node<K, V>::set_value(const V& val) {
    this->value = val;
}

template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
    this->max_level = max_level;
    skip_list_level = 0;
    element_count = 0;

    K k;
    V v;
    header = new Node<K, V>(k, v, max_level);
};

template<typename K, typename V>
SkipList<K, V>::~SkipList() {
    if(file_writer.is_open())
        file_writer.close();
    if(file_reader.is_open())
        file_reader.close();
    delete header;
}

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K& k, const V& v, int level) {
    Node<K, V>* t = new Node<K, V>(k, v, level);
    return t;
}

template<typename K, typename V>
bool SkipList<K, V>::insert_element(const K& key, const V& val) {
    mtx.lock();
    Node<K, V>* current = header;

    std::vector<Node<K, V>* > update(max_level + 1, nullptr);

    // 找到插入位置
    for(int i = skip_list_level; i >= 0; --i) {
        while(current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;  // 可能在update后面插入
    }  
    current = current->forward[0];

    // key 已存在
    if(current && current->get_key() == key) {
        error_handing("Key exists.");
        mtx.unlock();
        return false;
    }

    // current == nullptr 意味着到达链表末尾
    // current != nullptr && current->get_value() != (>) key 意味着要插入到 update[0] 和 current 之间
    if(current == nullptr || current->get_key() != key) {
        int random_level = get_random_level();
        if(random_level > skip_list_level) {
            for(int i = skip_list_level + 1; i <= random_level; ++i) {
                update[i] = header;  // 新的一层索引
            }
            skip_list_level = random_level;
        }
        
        // 要插入的节点, level为存在于的最高层
        Node<K, V>* newNode = create_node(key, val, random_level);
        // insert node
        for(int i = 0; i <= random_level; ++i) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
        std::cout << "Successfully inserted." << std::endl;
        ++element_count;
    }
    mtx.unlock();
    return true;
}

template<typename K, typename V>
bool SkipList<K, V>::search_element(const K& key) {
    Node<K, V>* current = header;

    // 从最大层开始查找，找到前一节点，通过--i，移动到下层再开始查找
    for(int i = skip_list_level; i >= 0; --i) {
        while(current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }
    current = current->forward[0];
    if(current && current->get_key() == key) {
        std::cout << "Found: (Key = " << key << ", Value = " << current->get_value() << ").";
        return true;
    } else {
        error_handing("Not Found Key.");
        return false;
    }
}

// Dislay skip list
template<typename K, typename V>
void SkipList<K, V>::display_list() {
    std::cout << "\n***** Skip List: *****\n";
    for(int i = 0; i <= skip_list_level; ++i) {
        Node<K, V>* t = header->forward[i];
        std::cout << "Level " << i << ": ";
        while(t) {
            std::cout << "(" << t->get_key() << ", " << t->get_value() << ")";
            t = t->forward[i];
        }
        std::cout << std::endl;
    }
}

// Delete element from skip list 
template<typename K, typename V> 
bool SkipList<K, V>::delete_element(const K& key) {
    mtx.lock();
    Node<K, V>* current = header;
    std::vector<Node<K, V>* > update(max_level + 1, nullptr);

    // 从最高级索引开始删
    for(int i = skip_list_level; i >= 0; --i) {
        while (current->forward[i] !=NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];
    
    if(current && current->get_key() == key) {
        // 从下往上删
        for(int i = 0; i <= skip_list_level; ++i) {
            if(update[i]->forward[i] != current) break;
            update[i]->forward[i] = current->forward[i];  // update[i]->forward[i]->forward[i]
        }
        // 可能会降低索引层数
        // Remove levels which have no elements
        while(skip_list_level > 0 && header->forward[skip_list_level] == nullptr) {
            --skip_list_level;
        }
        std::cout << "Successfully deleted (" << key << ", " << current->get_value() << ")" << std::endl;
        --element_count;
            
        mtx.unlock();
        return true;
    } else {
        error_handing("Not Found Key.");
        return false;
    }
}

// Dump data in memory to file
template<typename K, typename V>
void SkipList<K, V>::dump_file() {
    std::cout << "Dumping File ...\n";
    file_writer.open(FILE_PATH);
    Node<K, V>* t = header->forward[0];
    while(t) {
        file_writer << t->get_key() << ":" << t->get_value() << "\n";
        t = t->forward[0];
    }
    file_writer.flush();
    file_writer.close();
}

// Load data from disk
template<typename K, typename V>
void SkipList<K, V>::load_file() {
    file_reader.open(FILE_PATH);
    std::cout << "Loading File ...\n";
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    std::cout << "open_right.\n";
    while(getline(file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if(key->empty() || value->empty()) 
            continue;
        
        std::cout << "load (" << *key << ", " << *value << ")" << std::endl;
        insert_element(std::stoi(*key), *value);
        std::cout << "load (" << *key << ", " << *value << ")" << std::endl; 
    }
    file_reader.close();
}

template<typename K, typename V>
int SkipList<K, V>::size() {
    return element_count;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {
    if(!is_valid_string(str)) return ;
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {
    if(str.empty()) return false;
    if(str.find(delimiter) == std::string::npos) return false;
    return true;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level() {
/*
    该 randomLevel 方法会随机生成 1~MAX_LEVEL 之间的数，且 ：
       1/2 的概率返回 1   => No
       1/4 的概率返回 2   => 一级索引
       1/8 的概率返回 3 以此类推
 */
    int level = 1;
    while(rand() % 2 && level < max_level) 
        ++level;

    return level;
}

void error_handing(const std::string& arg) {
    int len = arg.size();
    std::string cutoffLine('*', len + 5);
    std::cout << cutoffLine << std::endl;
    std::cout << arg << std::endl;
    std::cout << cutoffLine << std::endl;
}


#endif  // SKIP_LSKIP_LIST_H
