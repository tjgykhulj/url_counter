# url_counter
* Usage

    make build

	mkdir -p out

	./mock out/url 1024   // mock 1024MB的数据，写入out/url文件

	./url_counter out/url  // 分析指定的out/url内，以换行符分割的url的count top 100


* 简介
    * 功能：读取指定filename中，换行符分割的url，输出数量top 100的url  
    * 数据：100GB文件  
    * 环境：1G内存，可写文件
* 思路
    * 将指定文件，按hash(url) % SEG_FILE_NUM的值，将其offset, len, hash写入指定分割文件。  
    * 迭代每一个文件，解出文件对应map<url, count>，并将按照count数更新到一个全局的小根堆中，小根堆里维护的是count top 100。
    * 打印出小根堆

* 可补充的未完成项
    * 某在解某个文件的map<url, count>时，发现其中的url过多，可以将其重新分割成更多的子文件
    * 由于使用hash+len判断值，有一个较小的错判率。为了保证100%正确，考虑到两种做法：
        1. 当两个字串的hash与len均相同时，读src input文件中指定offset来判断字串是否真的相同
        2. 做一个hash2值，使错判断小到忽略不计
