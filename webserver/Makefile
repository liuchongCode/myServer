#用于存放中间过程生成的所有文件
BUILD_DIR = build
#src变量为所有 ./src目录下的.cpp文件名
src = $(wildcard ./src/*.cpp)
#objs变量为 build/*.o
#notdir函数是将 例如 ./src/init.cpp 字符串改为 init.cpp
#因此转换完之后，为 build/init.o
objs = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(notdir $(src)))
#指定头文件目录
INC_DIR = include
#DEPS变量为所有头文件名
DEPS = $(wildcard $(INC_DIR)/*.h)
#将include 改为 -Iinclude， 以便之后编译时使用
CFLAGS = $(patsubst %, -I%, $(INC_DIR))

#目标文件
target=main
#build/main:*.o
$(BUILD_DIR)/$(target):$(objs)
#g++ *.o -o build/main
	$(CXX) -g $(objs) -o $(BUILD_DIR)/$(target) -l pthread
#.o文件生成规则
#build/*.o:*.cpp include/*.h
$(BUILD_DIR)/%.o:src/%.cpp $(DEPS)
#创建build文件，命令前带 @ 表示命令执行不会输出在终端
#g++ -c *.cpp -o *.o * -Iinclude
	@mkdir -p $(BUILD_DIR)
	$(CXX) -g -c $< -o $@ $(CFLAGS) $(CFLAGS)/Log

#伪目标, 终端使用 make clean 执行
./PHONY:clean
clean:
#按直接删除整个build文件
	@rm -rf build/*.o build/main