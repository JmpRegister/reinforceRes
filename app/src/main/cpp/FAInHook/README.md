该项目核心的hook代码来自ele7enxxh，想获取最新的InLineHook方法，请移步https://github.com/ele7enxxh/Android-Inline-Hook


Android So Inline hook.

You can add FAInHook in your project by adding follow
cmake config in your CMakeLists.txt

set(FA_STATIC on)       # set this if you want to build FAGotHook as a static library

add_directory(path/to/FAInHook)

target_link_libraries( ${ProjectName} FAGotHook)

see main.cpp to find how to use

This project is still in developed.

这个上面说使用add_directory，反正我android上使用的cmake显示没有这个命令，只能使用add_subdirectory，还要求在android工程的目录树中，这样的话我只有使用双subdirectory的方式来使用这个库了，在我的笔记中有记载。即在cpp目录下建立两个子目录来分别编译。