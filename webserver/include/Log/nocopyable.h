#ifndef NOCOPYABLE_H
#define NOCOPYABLE_H

/*
    不可拷贝类
    私有拷贝构造函数和复制构造函数
*/
class nocopyable
{
private:
    nocopyable(const nocopyable&);
    const nocopyable& operator=(const nocopyable);
protected:
    nocopyable() {}
    ~nocopyable() {}
};

#endif
