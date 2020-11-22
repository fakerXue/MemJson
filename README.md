# DatX
一种紧凑的数据结构，适合网络传输，配置存储等，一定程度上能替代json，读写均优于json，0秒解析。自带demo演示。
通过MemJson可以实现与Json的互转，也支持将原生DatX文本化；
```C++
/*************************************************************************
** Desc		: 一种紧凑的数据结构，适合网络传输，配置存储等，一定程度上能替代json，读写均优于json，0秒解析
**				内存结构如下： {DatX::__len__,[szKey01,nBin01,pBin01],[szKey02,nBin02,pBin02],...}
**              兼容所有json格式，如：
**                  1.单级对象：{"aaa":521,"bbb":13.14,"ccc":"hello"}
**                  2.单级数组：["elem0","elem1","elem2"]
**                  3.对象包数组：{"aaa":521,"bbb":13.14,"ccc":"hello","ddd":["elem0","elem1","elem2"]}
**                  4.数组包对象：[{"aaa0":521,"bbb0":13.14,"ccc0":"hello"},{"aaa1":521,"bbb1":13.14}]
**              另外支持的格式，如：
**                  1.混杂数组1[0~2为键值对元素，3为数组元素]：{"aaa":521,"bbb":13.14,"ccc":"hello",["elem0","elem1","elem2"]}
**                  2.混杂数组2：[0~2为键值对元素，3为数组元素，4~5为二进制]：{"aaa":521,"bbb":13.14,"ccc":"hello",["elem0","elem1","elem2"],"binary0":stream,"binary1":stream}
**            函数用法概览：
**              1.Put：向当前DatX对象添加一个KV，当K已存在时会进行值替换，返回当前DatX的引用；
**              2.Get(s)：从当前DatX对象获取K==s的KV对，仅支持有键名的元素；用XTY包装并返回；
**              3.Get(i)：从当前DatX对象获取第i个KV对，支持所有类型元素；用XTY包装并返回；
**              4.Put：向当前DatX添加一个有键名元素，一般用于添加键值对元素，返回当前DatX的引用；
**              5.Add：向当前DatX添加一个无键名元素，一般用于添加数组元素，返回当前DatX的引用；
**              6.Del(s)：从当前DatX删除一个有键名的元素，一般用于删除键值对元素；
**              7.Del(i)：向当前DatX添加一个无键名的元素，一般用于删除数组元素；
**              8.[s]：将当前DatX键名为s的键值转化为一个DatX，用于操作访问子节点；
**              9.[i]：将当前DatX第i个元素键值转化为一个DatX，用于操作访问子节点；
*************************************************************************/
```

![image](https://github.com/fakerXue/DatX/blob/main/TestCase_DatX.png)
![image](https://github.com/fakerXue/DatX/blob/main/TestCase_MemJson.png)

