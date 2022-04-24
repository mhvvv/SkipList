#include <iostream>

#include "skip_list.h"


int main() {

    /*
        key如果为自定义类型, 需要重载比较函数
        注意skipList.load_file函数中类型的定义
     */

    SkipList<int, std::string> skipList(6);

    skipList.load_file();
    std::cout << "skipList size:" << skipList.size() << std::endl;
    
    skipList.search_element(9);
    skipList.search_element(18);


    skipList.display_list();

    skipList.delete_element(3);
    skipList.delete_element(7);

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.display_list();

    skipList.dump_file();
}
