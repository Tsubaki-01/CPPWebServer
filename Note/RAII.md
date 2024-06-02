在原项目中`sqlConnRAII`的使用借助了匿名对象，但是在这边有使用逻辑上的错误。

`sqlConnRAII(sql, SqlConnPool::instance());`

这么一条语句，匿名对象在构造之后，句子结束了就会马上调用析构函数，导致sql连接马上返回连接队列。

因此，应该定义一个局部变量来使得匿名变量成为一个命名变量。

`sqlConnRAII tempRAII = sqlConnRAII(sql, SqlConnPool::instance());`

这样就能在`tempRAII`生命周期结束时才调用析构函数