# Sylar


## 1.日志模块

###        一直有点搞不定log模块各个类之间的调度关系,今天来梳理一下
        

       1.

                        LogEventWrap(首先创建一个LogEvenWrap对象,接收一个LogEven对象)
                                              |
                                              |
                                        LogEvent(logger,level,...)
                                        
                                        
                                        
        2.然后调用LogEvenWrap的getSS(),其实就是返回他所接收的LogEvent的m_ss,然后接收日志信息,自此,我们自己输入的日志信息保存在m_ss
        
        
        3.LogEventWrap对象析构,调用 m_event->getLogger()->log(m_event->getLevel(), m_event);
            
            (1)m_event调用logger的log()方法
            (2)logger在log方法中调用m_appenders的log()方法
            
~~~cpp
            void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        auto self = shared_from_this();
        MutexType::Lock lock(m_mutex);
        if(!m_appenders.empty()) {
            for(auto& i : m_appenders) {
                i->log(self, level, event);
            }
        } else if(m_root) {
            m_root->log(level, event);
        }
    }
}

~~~
            
            
            (3)m_appenders调用m_formatter的format()方法
~~~cpp
void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        MutexType::Lock lock(m_mutex);
        m_formatter->format(std::cout, logger, level, event);
    }
}
~~~
            重点:
            (4)format()将一个std::cout作为参数传入子类的format方法来收集日志所需信息,其中,m_ss的内容已经
            通过LogEvent的getPartten()方法输入到std::cout,这是最难以理解的地方,稍不小心就会错过一些细节,导致一直看不懂
~~~cpp
std::ostream& LogFormatter::format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    for(auto& i : m_items) {
        i->format(ofs, logger, level, event);
    }
    return ofs;
}
~~~
           
###   解决完调度关系,还有一个关键问题就是LogManager类的初始化过程

        大概有几个类
        1.首先是LogManager类:
            (1)构造方法中,首先会重置m_root,并添加到m_loggers,然后执行init方法
        ---其中init方法好像没完善,我猜大概作用就是从配置文件读取配置
            此处是我没理解好Config.h类,源码中:
            
```cpp
sylar::ConfigVar<std::set<LogDefine> >::ptr g_log_defines =
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");
```
            其实就已经将logs注册到s_datas中,后面要通过LoadFromYaml函数,将配置文件的内容加载到g_log_defines,还剩一个问题就是怎么从g_log_defines读取配置
            (2)getLogger()方法,接收一个日志器的名称,然后去m_loggers里面查找,找不到会创建一个并加入到m_loggers
            (3)toYamlString()方法:其实就是将类的属性转换成配置文件格式
        2.LogDefine类:
            (1)是将Log类转换成配置文件的中间件
        3.LogAppenderDefine类:
            (1)是将LogAppenderDefine类转换成配置文件的中间件
        4.LogFormatterDefine类:
            (1)是将LogFormatterDefine类转换成配置文件的中间件
        5.还有两个LexicalCast的全特化,用于转换LogDefine的类型
        6.结构体 LogIniter:
            用于回调操作,在Log的配置文件发生变化时,修改对应Logger对象的属性值
            

                                        

## 2.配置模块
---





由于对yaml格式不了解,所以准备写成json格式的解析配置系统且json格式有json.hpp库十分方便
        
        
        
        遇到的问题:
        1.一开始不理解为什么作者只遍历ismap,yaml格式的序列不遍历,想着listallmenber函数将json的数组也遍历存
        入ConfigVarMap,后面编写vector的偏特化模板函数发现,map容器不允许重复键,所以对于配置文件中的数组类型,
        map只会存下最后一位元素,解决办法就是不遍历数组,只遍历键值对即json的对象类型.
        2.set,map等要使用insert,还有map记得make_pair()
                          --------------截止3.8完成的任务,周五下班,周六3.9再战,希望两天之内完成和日志模块的整合,进军线程模块
                          
        3.今天看视频,作者遇到了s_data这个变量初始化的问题,问题在于这是个静态变量,存在还未初始化就要使用到的情况
        优化成一个静态方法返回静态变量:
~~~cpp
        static ConfigVarMap& GetDatas() {
            static ConfigVarMap s_datas;
            return s_datas;
        }
~~~


- [x] 3.11 --- 还是觉得得单独梳理总结一下配置模块:
        
        1.ConfigVarBase类:封装配置数据的抽象类,主要用于存储配置数据
        2.ConfigVar类:继承自ConfigVarBase
        3.Config类:用于提供各种处理配置数据的方法

                            首先调用lookup方法,自定义配置数据存入s_datas
                                            |
                                            |
                                    然后调用LoadFromJson()
                            加载配置文件并查询修改s_datas的内容
                                            |
                                            |
                            通过一系列类型转换,并在setValue过程中调用数据改变
                            的回调函数

## 3.日志模块和配置模块的总结
---
        1.日志模块主要难在各个类之间的调度关系,我自己感觉,源码里面很多操作是不必要的,作者的一些操作增加了理解难度
        2.配置模块难在模版的偏特化,还有对yaml格式的不熟悉,要是真看不懂yaml,建议自己照着写一个json格式的,并不难,甚至好像更简单
                            -------------------截止3.9
         
         问题:
            1.遇到了很多次,往logger里面新添加appender没有给他设置formatter,要记得调用logger的setFormatter()方法,保证appender->hasformatter()==true
                 ----由此,我实在感觉源码中有很多部分是冗余的,所以致使许多问题,可能是框架还待完善,很多模块还没到使用的环节
        
### 24.3.10 --日志和配置模块完结:

        期间遇到很多问题,我个人也觉得这只是匆匆结束,临近结尾,最大的问题就是源码没有封装Log模块的自动    加载配置文件,我觉得作者原来的每次加载要自己调用LoadFromYaml太过于麻烦,所以准备自己封装.
        
        刚开始,我把加载的过程写在了LogManager类的init()函数,并通过LogManager的构造函数调用init()方法,
        希望实现在SYLAR_LOG_NAME()返回logger对象的同时完成配置文件的加载,但是发现一个问题,init函数的调用似乎优先于
                static Sylar::ConfigVar<std::set<LogDefine>>::ptr g_log_defines =
                    Sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");
        此变量的初始化,LoadFormJson函数加载配置文件并修改s_datas的时候无法找到logs对应的配置,因此最后  
        会出现没法读取配置文件的配置,仍使用默认配置的情况,我还尝试过将此变量写进init方法,但是似乎以为  此变量依赖Lookup方法和GetData()等静态方法,定义在init()中会报错,这似乎和变量的初始化顺序有关,  我没有想到很好的解决办法,所以只能尝试封装了一个ManagerLog类来进行初始化,所幸这种方法可行.
        
        无论如何,能正常运行就是胜利,其他的后面要是有时间再回来改进,明天进军线程模块


## 4.线程模块 --- 3.11启
            
~~~cpp
static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";
~~~
重写muduo库的项目中遇到过,目的是使每一个线程都存储一份变量,这样子每一个线程访问到的都是不同的实例
        


~~~cpp
  Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = sylar::GetThreadId();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

    std::function<void()> cb;
    cb.swap(thread->m_cb);//防止函数对象内有智能指针,通过swap减少引用,防止资源无法释放

    thread->m_semaphore.notify();

    cb();
~~~
