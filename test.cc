#include <functional>
#include <iostream>
using namespace std;
class A;



class A {
public:
    A(function<void()> cb) { cout << "A cb()"; }
};
template <class T> void test2(T a) {
    cout << "test2" << endl;
    A b(a);
}







template <class T> void test1(T a) {
    cout <<"test1" << endl;
    test2(a);
}










void test() { cout << "test" << endl; }




int main() {
  function<void()> cb = test;
    test1(cb);

  return 0;
}
