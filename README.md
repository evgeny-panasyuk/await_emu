`await`_emu
=========

Full emulation of `await` feature from C# language in C++ based on Stackful Coroutines from `Boost.Coroutine` library.

This proof-of-concept shows that exact syntax of `await` feature can be emulated with help of Stackful Coroutines, demonstrating that it is superior mechanism.

Main aim of this proof-of-concept is to draw attention to Stackful Coroutines, but not emulation of `await` itself, because stackful coroutines is superior mechanism and allows to reach higher levels of asynchronous encapsulation.

Code example
============

```C++
int bar(int i)
{
    // await is not limited by "one level" as in C#
    auto result = await async([i]{ return reschedule(), i*100; });
    return result + i*10;
}

int foo(int i)
{
    cout << i << ":\tbegin" << endl;
    cout << await async([i]{ return reschedule(), i*10; }) << ":\tbody" << endl;
    cout << bar(i) << ":\tend" << endl;
    return i*1000;
}

void async_user_handler()
{
    vector<future<int>> fs;

    // instead of `async` at function signature, `asynchronous` should be
    // used at the call place:
    for(auto i=0; i!=5; ++i)
        fs.push_back( asynchronous([i]{ return foo(i+1); }) );

    for(auto &&f : fs)
        cout << await f << ":\tafter end" << endl;
}
```
`await` takes `std::future`-like object with support of continuation attachment (e.g. [`.then`](http://www.boost.org/doc/libs/1_54_0/doc/html/thread/synchronization.html#thread.synchronization.futures.then)), and "returns result" of future.

At this example `boost::async` is used. But that approach can be extended to work with other futures, like PPL's [`task::then`](http://msdn.microsoft.com/en-us/library/hh750044.aspx), etc.

Output example:
```
1:      begin
2:      begin
3:      begin
4:      begin
5:      begin
20:     body
10:     body
30:     body
110:    end
1000:   after end
50:     body
220:    end
2000:   after end
550:    end
40:     body
330:    end
3000:   after end
440:    end
4000:   after end
5000:   after end
```
