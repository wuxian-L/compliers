#include <iostream>
void test(){
    if(3 == 3 < 4 == 3){
        std::cout << "ok" << std::endl;
    }
}
int main(){
    test();
    return 0;
}