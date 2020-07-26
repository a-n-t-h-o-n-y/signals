#include <functional>
#include <mutex>
#include <optional>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include <boost/function.hpp>

#include <catch2/catch.hpp>

#include <signals/connection.hpp>
#include <signals/detail/signal_impl.hpp>
#include <signals/optional_last_value.hpp>
#include <signals/position.hpp>
#include <signals/signal.hpp>
#include <signals/slot.hpp>

using sig::Connection;
using sig::Optional_last_value;
using sig::Position;
using sig::Signal;
using sig::Signal_impl;
using sig::Slot;

class Signal_impl_fixture {
   protected:
    using type_1 = Signal_impl<void(int),
                               Optional_last_value<void>,
                               int,
                               std::less<int>,
                               std::function<void(int)>,
                               std::mutex>;

    using type_2 = Signal_impl<double(char, int),
                               Optional_last_value<double>,
                               char,
                               std::less<char>,
                               std::function<double(char, int)>,
                               std::mutex>;

    using type_3 = Signal_impl<unsigned(long long),
                               Optional_last_value<unsigned>,
                               double,
                               std::greater<double>,
                               boost::function<unsigned(long long)>,
                               std::mutex>;

    using type_4 = Signal_impl<char(int, double),
                               Optional_last_value<char>,
                               int,
                               std::less<int>,
                               std::function<char(int, double)>,
                               std::mutex>;

    Signal_impl_fixture()
    {
        signal_std.connect(boost_slot_non_empty1);
        signal_std.connect(std_slot_non_empty1);
        std_slot_holds_signal = Slot<void(int)>{signal_std};

        signal_boost.connect(boost_slot_empty2);
        signal_boost.connect(std_slot_empty2);
        boost_slot_holds_signal =
            Slot<std::optional<unsigned>(long long),
                 boost::function<std::optional<unsigned>(long long)>>{
                signal_boost};

        si_type1_2.connect([](int) { return; }, Position::at_back);

        si_type2_2.connect([](char, int) { return 0.3; }, Position::at_back);
        si_type2_2.connect([](char, int) { return 7.3; }, Position::at_back);
        si_type2_2.connect([](char, int) { return 9.1; }, Position::at_back);

        si_type3_2.connect([](long long) { return 4; }, Position::at_back);
        si_type3_2.connect([](long long) { return 5; }, Position::at_back);
        si_type3_2.connect([](long long) { return 6; }, Position::at_back);
        si_type3_2.connect([](long long) { return 7; }, Position::at_back);
        si_type3_2.connect([](long long) { return 8; }, Position::at_back);
        si_type3_2.connect([](long long) { return 9; }, Position::at_back);
    }

    // type_1 - void(int), less<int>, std::function
    type_1 si_type1_1{Optional_last_value<void>(), std::less<int>()};
    type_1 si_type1_2{Optional_last_value<void>(), std::less<int>()};

    // type_2 - double(char, int), less<char>, std::function
    type_2 si_type2_1{Optional_last_value<double>(), std::less<char>()};
    type_2 si_type2_2{Optional_last_value<double>(), std::less<char>()};

    // type_3 - unsigned(long long), greater<double>, boost::function
    type_3 si_type3_1{Optional_last_value<unsigned>(), std::greater<double>()};
    type_3 si_type3_2{Optional_last_value<unsigned>(), std::greater<double>()};

    // type 4 - char(int, double), less<int>, std::function
    type_4 si_type4{Optional_last_value<char>(), std::less<int>()};

    // Slots - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Empty Slot - boost::function
    Slot<void(int), boost::function<void(int)>> boost_slot_empty1{};

    Slot<void(int), boost::function<void(int)>> boost_slot_non_empty1{
        [](int) { return; }};

    // Empty Slot - boost::function
    Slot<unsigned(long long), boost::function<unsigned(long long)>>
        boost_slot_empty2{};

    Slot<unsigned(long long), boost::function<unsigned(long long)>>
        boost_slot_non_empty2{[](long long) { return 3; }};

    // Function Slot - boost::function
    Slot<void(unsigned), boost::function<void(unsigned)>> boost_slot_function1{
        [](unsigned) { return; }};

    // Function Slot - boost::function
    Slot<double(char), boost::function<double(char)>> boost_slot_function2{
        [](char) { return 3.4; }};

    // Extended Slot - boost::function
    Slot<char(const Connection&, int, double),
         boost::function<char(const Connection&, int, double)>>
        boost_extended_slot{[](const Connection&, int, double) { return 'h'; }};

    // Slot holds empty Signal - boost::function
    Signal<unsigned(long long),
           Optional_last_value<unsigned>,
           int,
           std::less<int>,
           boost::function<unsigned(long long)>>
        empty_signal_boost{};
    Slot<std::optional<unsigned>(long long),
         boost::function<std::optional<unsigned>(long long)>>
        boost_slot_holds_empty_signal{empty_signal_boost};

    // Slot holds non-empty Signal - boost::function
    Signal<unsigned(long long),
           Optional_last_value<unsigned>,
           int,
           std::less<int>,
           boost::function<unsigned(long long)>>
        signal_boost{};
    Slot<std::optional<unsigned>(long long),
         boost::function<std::optional<unsigned>(long long)>>
        boost_slot_holds_signal{};

    // Empty Slot - std::function
    Slot<void(int)> std_slot_empty1{};

    Slot<void(int)> std_slot_non_empty1{[](int) { return; }};

    // Empty Slot - std::function
    Slot<unsigned(long long)> std_slot_empty2{};

    Slot<unsigned(long long)> std_slot_non_empty2{[](long long) { return 7; }};

    // Function Slot - std::function
    Slot<void(unsigned)> std_slot_function1{[](unsigned) { return; }};

    // Function Slot - std::function
    Slot<double(char)> std_slot_function2{[](char) { return 3.4; }};

    // Extended Slot - std::function
    Slot<char(const Connection&, int, double)> std_extended_slot{
        [](const Connection&, int, double) { return 'k'; }};

    // Slot holds empty Signal - std::function
    Signal<void(int)> empty_signal_std{};
    Slot<void(int)> std_slot_holds_empty_signal{empty_signal_std};

    // Slot holds non-empty Signal - std::function
    Signal<void(int)> signal_std{};
    Slot<void(int)> std_slot_holds_signal{};

    // Functors - - - - - - - - - - - - - - - - - - - - -
    class functor {
       public:
        double operator()(char, int) { return 6.5; }
    };
};

TEST_CASE_METHOD(Signal_impl_fixture, "Signal_impl()", "[Signal_impl]")
{
    CHECK(0 == si_type1_1.num_slots());
    CHECK(0 == si_type2_1.num_slots());
    CHECK(0 == si_type3_1.num_slots());

    CHECK(si_type1_1.empty());
    CHECK(si_type2_1.empty());
    CHECK(si_type3_1.empty());

    si_type1_1(7);  // returns void
    CHECK_FALSE(bool(si_type2_1('l', 5)));
    CHECK_FALSE(bool(si_type3_1(99)));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl(Signal_impl const&)",
                 "[Signal_impl]")
{
    type_1 copied = si_type1_1;
    CHECK(0 == copied.num_slots());
    CHECK(copied.empty());
    copied(7);  // returns void

    type_2 copied2 = si_type2_2;
    CHECK(3 == copied2.num_slots());
    CHECK_FALSE(copied2.empty());
    REQUIRE(bool(copied2('f', 7)));
    CHECK(9.1 == Approx(*(copied2('4', 6))).margin(1e-12));

    type_3 copied3 = si_type3_2;
    CHECK(6 == copied3.num_slots());
    CHECK_FALSE(copied3.empty());
    REQUIRE(bool(copied3(9999)));
    CHECK(9 == *(copied3(9997876)));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl(Signal_impl&&)",
                 "[Signal_impl]")
{
    type_1 sig_impl_mv1{std::move(si_type1_1)};
    CHECK(si_type1_1.empty());

    type_1 sig_impl_mv2{std::move(si_type1_2)};
    CHECK(si_type1_2.empty());
    CHECK(1 == sig_impl_mv2.num_slots());

    type_3 sig_impl_mv3{std::move(si_type3_2)};
    CHECK(si_type3_2.empty());
    CHECK(6 == sig_impl_mv3.num_slots());
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::operator=(Signal_impl const&)",
                 "[Signal_impl]")
{
    CHECK(si_type1_1.empty());
    si_type1_1 = si_type1_2;
    CHECK(1 == si_type1_1.num_slots());
    CHECK(1 == si_type1_2.num_slots());

    CHECK(6 == si_type3_2.num_slots());
    si_type3_1 = si_type3_2;
    CHECK(6 == si_type3_1.num_slots());
    CHECK(6 == si_type3_2.num_slots());
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::operator=(Signal_impl&&)",
                 "[Signal_impl]")
{
    CHECK(si_type1_1.empty());
    si_type1_1 = std::move(si_type1_2);
    CHECK(1 == si_type1_1.num_slots());
    CHECK(si_type1_2.empty());

    CHECK(6 == si_type3_2.num_slots());
    si_type3_1 = std::move(si_type3_2);
    CHECK(6 == si_type3_1.num_slots());
    CHECK(si_type3_2.empty());
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::connect() by position",
                 "[Signal_impl]")
{
    // void(int)
    // this slot calls a signal which calls both a std slot and a boost slot.
    Connection conn1 =
        si_type1_1.connect(std_slot_holds_signal, Position::at_back);
    Connection conn2 =
        si_type1_1.connect(std_slot_non_empty1, Position::at_front);
    CHECK(2 == si_type1_1.num_slots());
    si_type1_1(7);  // returns void

    // double(char, int)
    Connection conn3 =
        si_type2_1.connect([](char, int) { return 8.3; }, Position::at_back);
    Connection conn4 =
        si_type2_1.connect([](char, int) { return 2.8; }, Position::at_front);
    CHECK(2 == si_type2_1.num_slots());
    CHECK(8.3 == Approx(*si_type2_1('f', 5)).margin(1e-12));
    conn3.disconnect();
    CHECK(1 == si_type2_1.num_slots());
    CHECK(2.8 == Approx(*si_type2_1('k', 8)).margin(1e-12));

    // unsigned(long long)
    Connection conn5 = si_type3_1.connect(std_slot_non_empty2,
                                          Position::at_back);  // returns 7
    Connection conn6 = si_type3_1.connect(boost_slot_non_empty2,
                                          Position::at_back);  // returns 3
    Connection conn7 =
        si_type3_1.connect([](long long) { return 2; }, Position::at_front);

    CHECK(3 == si_type3_1.num_slots());
    CHECK(3 == *si_type3_1(9999999));
    conn6.disconnect();
    CHECK(7 == *si_type3_1(7654321));
    conn5.disconnect();
    CHECK(2 == *si_type3_1(1234567));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::connect() by group and position",
                 "[Signal_impl]")
{
    // void(int), less<int>
    Connection conn1 =
        si_type1_1.connect(3, std_slot_holds_signal, Position::at_back);
    Connection conn2 =
        si_type1_1.connect(1, std_slot_non_empty1, Position::at_front);
    CHECK(2 == si_type1_1.num_slots());
    si_type1_1(7);  // returns void

    // double(char, int), less<char>
    Connection conn3 = si_type2_1.connect(
        'a', [](char, int) { return 8.3; }, Position::at_back);
    Connection conn4 = si_type2_1.connect(
        'z', [](char, int) { return 2.8; }, Position::at_front);
    Connection conn5 = si_type2_1.connect(
        'z', [](char, int) { return 7.3; }, Position::at_back);
    CHECK(3 == si_type2_1.num_slots());
    CHECK(7.3 == Approx(*si_type2_1('f', 5)).margin(1e-12));
    conn5.disconnect();
    CHECK(2 == si_type2_1.num_slots());
    CHECK(2.8 == Approx(*si_type2_1('k', 8)).margin(1e-12));
    conn4.disconnect();
    CHECK(1 == si_type2_1.num_slots());
    CHECK(8.3 == Approx(*si_type2_1('k', 8)).margin(1e-12));

    // unsigned(long long), greater<double>
    Connection conn6 = si_type3_1.connect(5.4, std_slot_non_empty2,
                                          Position::at_back);  // returns 7
    Connection conn7 = si_type3_1.connect(5.3, boost_slot_non_empty2,
                                          Position::at_back);  // returns 3
    Connection conn8 = si_type3_1.connect(
        5.3, [](long long) { return 8; }, Position::at_front);
    Connection conn9 = si_type3_1.connect(
        0.77, [](long long) { return 2; }, Position::at_front);

    CHECK(4 == si_type3_1.num_slots());
    CHECK(2 == *si_type3_1(9999999));
    conn9.disconnect();
    CHECK(3 == *si_type3_1(7654321));
    conn7.disconnect();
    CHECK(8 == *si_type3_1(1234567));
    conn8.disconnect();
    CHECK(7 == *si_type3_1(1234567));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::connect() both overloads",
                 "[Signal_impl]")
{
    // double(char, int), less<char>
    Connection conn1 = si_type2_1.connect(
        'a', [](char, int) { return 8.3; }, Position::at_back);
    Connection conn2 =
        si_type2_1.connect([](char, int) { return 2.8; }, Position::at_front);
    Connection conn3 =
        si_type2_1.connect([](char, int) { return 9.1; }, Position::at_back);
    Connection conn4 = si_type2_1.connect(
        'z', [](char, int) { return 7.3; }, Position::at_front);

    CHECK(9.1 == Approx(*si_type2_1('f', 5)).margin(1e-12));
    conn3.disconnect();

    CHECK(7.3 == Approx(*si_type2_1('f', 5)).margin(1e-12));
    conn4.disconnect();

    CHECK(8.3 == Approx(*si_type2_1('k', 8)).margin(1e-12));
    conn1.disconnect();

    CHECK(2.8 == Approx(*si_type2_1('k', 8)).margin(1e-12));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::connect_extended() by position",
                 "[Signal_impl]")
{
    // char(int, double)
    Connection conn1 =
        si_type4.connect_extended(boost_extended_slot, Position::at_front);
    CHECK_FALSE(conn1 == Connection());
    CHECK(1 == si_type4.num_slots());
    REQUIRE(bool(si_type4(6, 3.42)));
    CHECK('h' == *si_type4(6, 3.42));

    Connection conn2 =
        si_type4.connect_extended(std_extended_slot, Position::at_back);
    CHECK(2 == si_type4.num_slots());
    REQUIRE(bool(si_type4(8, 0.43)));
    CHECK('k' == *si_type4(8, 0.43));

    conn1.disconnect();

    CHECK(1 == si_type4.num_slots());
    REQUIRE(bool(si_type4(8, 0.43)));
    CHECK('k' == *si_type4(8, 0.43));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::connect_extended() by group and position",
                 "[Signal_impl]")
{
    // char(int, double)
    Connection conn1 =
        si_type4.connect_extended(5, boost_extended_slot, Position::at_front);
    CHECK_FALSE(conn1 == Connection());
    CHECK(1 == si_type4.num_slots());
    REQUIRE(bool(si_type4(6, 3.42)));
    CHECK('h' == *si_type4(6, 3.42));

    Connection conn2 =
        si_type4.connect_extended(3, std_extended_slot, Position::at_back);
    CHECK(2 == si_type4.num_slots());
    REQUIRE(bool(si_type4(8, 0.43)));
    CHECK('h' == *si_type4(8, 0.43));

    conn1.disconnect();

    CHECK(1 == si_type4.num_slots());
    REQUIRE(bool(si_type4(8, 0.43)));
    CHECK('k' == *si_type4(8, 0.43));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::connect_extended() both overloads",
                 "[Signal_impl]")
{
    // char(int, double)
    Connection conn1 =
        si_type4.connect_extended(5, boost_extended_slot, Position::at_front);
    CHECK_FALSE(conn1 == Connection());
    CHECK(1 == si_type4.num_slots());
    REQUIRE(bool(si_type4(6, 3.42)));
    CHECK('h' == *si_type4(6, 3.42));

    Connection conn2 = si_type4.connect_extended(
        [](const Connection&, int, double) { return 'l'; }, Position::at_front);
    CHECK(2 == si_type4.num_slots());
    REQUIRE(bool(si_type4(8, 0.43)));
    CHECK('h' == *si_type4(8, 0.43));

    Connection conn3 = si_type4.connect_extended(
        [](const Connection&, int, double) { return 'p'; }, Position::at_back);
    CHECK(3 == si_type4.num_slots());
    REQUIRE(bool(si_type4(8, 0.43)));
    CHECK('p' == *si_type4(8, 0.43));

    Connection conn4 =
        si_type4.connect_extended(3, std_extended_slot, Position::at_back);
    CHECK(4 == si_type4.num_slots());
    REQUIRE(bool(si_type4(8, 0.43)));
    CHECK('p' == *si_type4(8, 0.43));

    conn3.disconnect();
    CHECK(3 == si_type4.num_slots());
    REQUIRE(bool(si_type4(8, 0.43)));
    CHECK('h' == *si_type4(8, 0.43));

    conn1.disconnect();
    CHECK(2 == si_type4.num_slots());
    REQUIRE(bool(si_type4(8, 0.43)));
    CHECK('k' == *si_type4(8, 0.43));

    conn4.disconnect();
    CHECK(1 == si_type4.num_slots());
    REQUIRE(bool(si_type4(8, 0.43)));
    CHECK('l' == *si_type4(8, 0.43));

    conn2.disconnect();
    CHECK(si_type4.empty());
    CHECK_FALSE(bool(si_type4(8, 0.43)));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::disconnect() by group",
                 "[Signal_impl]")
{
    // double(char, int), less<char>
    si_type2_1.connect(
        'a', [](char, int) { return 8.3; }, Position::at_back);
    si_type2_1.connect(
        'a', [](char, int) { return 1.3; }, Position::at_back);
    si_type2_1.connect([](char, int) { return 2.8; }, Position::at_front);
    si_type2_1.connect([](char, int) { return 9.1; }, Position::at_front);
    si_type2_1.connect(
        'z', [](char, int) { return 7.3; }, Position::at_front);

    CHECK(7.3 == Approx(*si_type2_1('f', 5)).margin(1e-12));
    si_type2_1.disconnect('z');

    CHECK(1.3 == Approx(*si_type2_1('f', 5)).margin(1e-12));
    si_type2_1.disconnect('a');

    CHECK(2.8 == Approx(*si_type2_1('k', 8)).margin(1e-12));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::disconnect_all_slots()",
                 "[Signal_impl]")
{
    CHECK(6 == si_type3_2.num_slots());

    si_type3_2.disconnect_all_slots();

    CHECK(0 == si_type3_2.num_slots());
    CHECK(si_type3_2.empty());
}

TEST_CASE_METHOD(Signal_impl_fixture, "Signal_impl::empty()", "[Signal_impl]")
{
    CHECK(si_type1_1.empty());

    si_type1_1.disconnect_all_slots();

    CHECK(si_type1_1.empty());

    si_type3_2.disconnect_all_slots();
    CHECK(si_type3_2.empty());
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::num_slots()",
                 "[Signal_impl]")
{
    CHECK(0 == si_type1_1.num_slots());
    CHECK(1 == si_type1_2.num_slots());
    CHECK(3 == si_type2_2.num_slots());
    CHECK(6 == si_type3_2.num_slots());
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::operator()",
                 "[Signal_impl]")
{
    // call on empty signal that returns void
    si_type1_1(1);

    // call on empty signal that does not return void
    CHECK_FALSE(bool(si_type2_1('j', 5)));

    // call on void return type signal that contains slots
    si_type1_2(8);

    // call on non-void return type signal and check return value
    REQUIRE(bool(si_type2_2('r', 8)));
    CHECK(9.1 == Approx(*si_type2_2('r', 8)).margin(1e-12));

    // Signal_impl with extended slots
    si_type4.connect_extended(boost_extended_slot, Position::at_back);
    REQUIRE(bool(si_type4(6, 3.2)));
    CHECK('h' == *si_type4(6, 3.2));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::operator() const",
                 "[Signal_impl]")
{
    // call on empty signal that returns void
    const type_1 si_1 = si_type1_1;
    si_1(3);

    // call on empty signal that does not return void
    const type_2 si_2 = si_type2_1;
    CHECK_FALSE(bool(si_2('j', 5)));

    // call on void return type signal that contains slots
    const type_1 si_3 = si_type1_2;
    si_3(8);

    // call on non-void return type signal and check return value
    const type_2 si_4 = si_type2_2;
    REQUIRE(bool(si_4('r', 8)));
    CHECK(9.1 == Approx(*si_4('r', 8)).margin(1e-12));

    // Signal_impl with extended slots
    si_type4.connect_extended(boost_extended_slot, Position::at_back);
    const type_4 si_5 = si_type4;
    REQUIRE(bool(si_5(6, 3.2)));
    CHECK('h' == *si_5(6, 3.2));
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::combiner()",
                 "[Signal_impl]")
{
    CHECK(typeid(si_type2_2.combiner()) ==
          typeid(Optional_last_value<double>{}));

    auto comb = si_type3_1.combiner();
    std::vector<int> vec{1, 2, 3, 4, 5};
    auto i = comb(std::begin(vec), std::end(vec));
    REQUIRE(bool(i));
    CHECK(5 == *i);
}

TEST_CASE_METHOD(Signal_impl_fixture,
                 "Signal_impl::set_combiner()",
                 "[Signal_impl]")
{
    si_type1_2.set_combiner(Optional_last_value<void>{});
    CHECK(typeid(si_type1_2.combiner()) == typeid(Optional_last_value<void>{}));

    si_type2_2.set_combiner(Optional_last_value<double>{});
    CHECK(typeid(si_type2_2.combiner()) ==
          typeid(Optional_last_value<double>{}));
}