# Build & Run

语言规范：C++11

Windows SDK : 10.0.17763.0

平台工具集：Visual Studio 2017 (v141)

Build 选项： Debug x86 / Release x86

# Others

- I/O: 默认使用std::in 与 std::out， 可在源代码MiniSQL.cpp中修改USEFILECOMMAND, USEFILELOG, TESTPATH, LOGFILENAME等宏进行默认文件I/O（若使用文件I/O可以通过命令行参数指定输入文件）
- 测试目录为./test/，其中有预留的测试文件command_0.txt与command_1.txt，分别对应基本功能测试与压力测试
- 数据库资源文件全部在./DataFiles/中
- 可执行文件为./Release/MiniSQL.exe或者./test/MiniSQL.exe

