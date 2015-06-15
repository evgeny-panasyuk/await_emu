// Copyright Evgeny Panasyuk 2015.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// e-mail: E?????[dot]P???????[at]gmail.???

// Example of await emultation based on stackless coroutine macros from Boost.Asio
// LIVE DEMO at http://coliru.stacked-crooked.com/a/dce4cedb3f2d2498

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/pop_front.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/cat.hpp>

#include <boost/asio/coroutine.hpp>

#include <iostream>
#include <utility>
#include <future>
#include <thread>

// inefficient "then" emulation
template<typename F, typename T>
void operator<<=(F c, std::future<T> &&f)
{
    std::thread([f=std::move(f), c=std::move(c)]() mutable
    {
        f.wait();
        c(std::move(f));
    }).detach();
}

/************************************************************************************************/
// refer http://www.boost.org/doc/libs/1_56_0/boost/fusion/adapted/struct/define_struct.hpp
// rerquired for sequence of tuples syntax ((a,b)(c,d)...)
#define COROUTINE_TYPE_0(X, Y) ((X, Y)) COROUTINE_TYPE_1
#define COROUTINE_TYPE_1(X, Y) ((X, Y)) COROUTINE_TYPE_0
#define COROUTINE_TYPE_0_END
#define COROUTINE_TYPE_1_END

#define VAR_TYPE(elem) BOOST_PP_TUPLE_ELEM(2, 0, elem)
#define VAR_NAME(elem) BOOST_PP_TUPLE_ELEM(2, 1, elem)
#define VARS(r, data, i, elem) BOOST_PP_COMMA_IF(i) VAR_TYPE(elem) VAR_NAME(elem)

#define MEMBER_DECLARE(r, data, i, elem) VAR_TYPE(elem) VAR_NAME(elem);
#define MEMBER_INIT(r, data, i, elem) VAR_NAME(elem)( std::move(VAR_NAME(elem)) )

#define FOR_EACH_DECL(action, seq) \
    BOOST_PP_SEQ_FOR_EACH_I(action, _, BOOST_PP_SEQ_POP_FRONT(BOOST_PP_CAT(COROUTINE_TYPE_0(0,0)seq,_END))) \
/**/

#define COROUTINE(result_type, name, params, locals) \
struct name : boost::asio::coroutine \
{ \
    FOR_EACH_DECL(MEMBER_DECLARE, locals) \
    FOR_EACH_DECL(MEMBER_DECLARE, params) \
    std::promise<result_type> result; \
    name(FOR_EACH_DECL(VARS, params)) \
        : FOR_EACH_DECL(MEMBER_INIT, params) \
    {} \
    auto operator()() \
    { \
        auto f = result.get_future(); \
        this->call(); \
        return f; \
    } \
    void call() \
    { \
        BOOST_ASIO_CORO_REENTER(*this) \
/**/

#define COROUTINE_END }; }

#define AWAIT(wanted) \
BOOST_ASIO_CORO_YIELD [self = std::move(*this)](auto ready_future) mutable \
{ \
    self.wanted ready_future.get(); \
    self.call(); \
} <<= \
/**/

#define RETURN(x) \
    this->result.set_value(x); \
    return; \
/**/
/************************************************************************************************/
using namespace std;

COROUTINE(int, coroutine, (int, param),
    (int, local_x)
    (int, local_y)
    (int, local_i))
{
    AWAIT(local_x =) async([]{ return 1; });

    local_y = 0;
    for(local_i = 0; local_i!=4; ++local_i)
        AWAIT(local_y +=) async([]{ return 2; });

    RETURN(local_x + local_y + param);
}
COROUTINE_END;

int main()
{
    auto f = coroutine{3}();
    cout << f.get() << endl;
}
