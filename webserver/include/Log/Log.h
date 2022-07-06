// #ifndef LOGH
// #define LOGH

// /**
// 关于源码这部分，能看到的版本也挺多的，主要区别都是做了很多平台处理，套了很多层，就找一个比较好理解的，原理都是一样的。

//     #include <stdarg.h>

// #define _INTSIZEOF(n) ((sizeof(n)+sizeof(int)-1)&~(sizeof(int) - 1) )
// 这个宏就理解成4字节对齐，换句话说也就是保证1字节对齐。
 
// #define va_start(ap,v) ( ap = (va_list)&v + _INTSIZEOF(v) ) 
// 这个宏，用于初始化参数列表ap，指定第一个参数的地址，完成初始化。其中ap初始化后的起始地址也就是v以后的地址。
 
// #define va_arg(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
// 这个宏，用于先改变ap的指向地址，即指向下一个地址，然后在返回当前地址，并将当前地址做t类型强转、
 
// #define va_end(ap) ( ap = (va_list)0 )
// 这个宏，用于释放ap指针使用，防止野指针。
// **/
// #include <fstream>
// #include <stdarg.h>
// #include <string>
// /*
//     使用单例模式：程序中该类只能有一个实例
//         - 将构造函数私有化，因此不能调用构造函数创建对象
//         - 由于无法创建对象，因此创建对象的成员函数需要声明为静态成员函数
//         - 由于返回指针存在线程安全问题，用户可能使用delete释放内存，所以改为返回引用
//         - 由于复制构造函数也可能导致多个实例，所以禁用拷贝构造函数
// */
// class Log
// {
// private:
//     Log() {
//         m_file.open("../WebServer.log", std::ios::out | std::ios::app);
//         printf("创建log\n");
//     }
    
//     std::ofstream m_file; 
//     // static Log * m_object;
// public: 
//     Log(Log & other) = delete;    // 禁用拷贝构造
//     ~Log() {
//         m_file.close();
//     }
//     // 创建
//     static Log& CreateLog() {
//         // if (m_object == nullptr) {
//         //     m_object = new Log();
//         // }
//         static Log m_object;
//         return m_object;
//     }

//     void fprintf(int argv, ...) {
//         va_list va;
//         va_start(va, argv);

//     }

//     Log & operator<<(int a) {
//         this->m_file << a;
//         return (*this);
//     }

//     Log & operator<<(unsigned int a) {
//         this->m_file << a;
//         return (*this);
//     }

//     Log & operator<<(long a) {
//         this->m_file << a;
//         return (*this);
//     }

//     Log & operator<<(unsigned long a) {
//         this->m_file << a;
//         return (*this);
//     }

//     Log & operator<<(char a) {
//         this->m_file << a;
//         return (*this);
//     }

//     Log & operator<<(char * a) {
//         this->m_file << a;
//         return (*this);
//     }

//     Log & operator<<(const char * a) {
//         this->m_file << a;
//         return (*this);
//     }

//     Log & operator<<(std::string a) {
//         this->m_file << a;
//         return (*this);
//     }
// };

// // std::ofstream & operator<<(std::ofstream & cout, Log & arg) {
    
// // }
// // Log Log::m_object = nullptr;
// static Log& LOG = Log::CreateLog(); // 创建log日志
// #endif