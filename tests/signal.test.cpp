#include <functional>
#include <memory>
#include <optional>
#include <typeinfo>

#include <signals/connection.hpp>
#include <signals/expired_slot.hpp>
#include <signals/optional_last_value.hpp>
#include <signals/position.hpp>
#include <signals/signal.hpp>
#include <signals/slot.hpp>

#include <catch2/catch.hpp>

using sig::Connection;
using sig::Expired_slot;
using sig::Optional_last_value;
using sig::Position;
using sig::Signal;
using sig::Slot;

TEST_CASE("Signal::arity", "[signal]")
{
    Signal<int(double, float, char, bool)> sig;
    CHECK(sig.arity == 4);

    Signal<int()> sig2;
    CHECK(sig2.arity == 0);

    Signal<void()> sig3;
    CHECK(sig3.arity == 0);

    CHECK(Signal<void(int, double)>::arity == 2);
}

TEST_CASE("Signal()", "[signal]")
{
    Signal<double(int, char)> sig1;
    CHECK(sig1.empty());
    CHECK_FALSE(bool(sig1(5, 'h')));  // returns an empty std::optional<double>

    Signal<void(int)> sig2;
    CHECK(sig2.empty());
    // CHECK_FALSE(bool(sig2(5))); // returns void

    Signal<void()> sig3;
    CHECK(sig3.empty());
    // CHECK_FALSE(bool(sig3())); // returns void

    Signal<void(char, double), Optional_last_value<void>> sig4;
    CHECK(sig4.empty());
    // CHECK_FALSE(bool(sig4('h', 8.4))); // returns void
    auto comb_void = Optional_last_value<void>();
    CHECK(typeid(comb_void) == typeid(sig4.combiner()));

    const Signal<char(int, double, float), Optional_last_value<char>,
                 std::less<char>>
        sig5;
    CHECK(sig5.empty());
    CHECK_FALSE(bool(sig5(4, 8.4, 6.5)));  // returns std::optional<char>
    auto comb_char = Optional_last_value<char>();
    CHECK(typeid(comb_char) == typeid(sig5.combiner()));
}

TEST_CASE("Signal in Slot connected to Signal", "[signal]")
{
    Signal<int(double, char)> sig_child;
    Slot<int(double, char)> slot_1 = [](double, char) { return 1; };
    sig_child.connect(slot_1);
    Slot<int(double, char)> slot_2 = [](double, char) { return 2; };
    sig_child.connect(slot_2, Position::at_front);
    Slot<int(double, char)> slot_3 = [](double, char) { return 3; };
    sig_child.connect(slot_3);

    Slot<std::optional<int>(double, char)> slot_holding_signal{sig_child};

    Signal<std::optional<int>(double, char)> sig_parent;
    sig_parent.connect(slot_holding_signal);

    // This slot will not be called by the signal containing sig_child
    Slot<int(double, char)> slot_4 = [](double, char) { return 4; };
    sig_child.connect(slot_4);

    auto result1 = sig_parent(7.3, 'h');

    REQUIRE(bool(result1));
    REQUIRE(bool(*result1));
    CHECK(**result1 == 3);

    auto result2 = sig_parent(7.3, 'h');

    REQUIRE(bool(result2));
    REQUIRE(bool(*result2));
    CHECK(**result2 == 3);
    CHECK(sig_parent.num_slots() == 1);

    auto result3 = sig_child(3.2, 'k');

    REQUIRE(bool(result3));
    CHECK(*result3 == 4);
    CHECK(sig_child.num_slots() == 4);
}

TEST_CASE("Signal(Signal&&)", "[signal]")
{
    Signal<char(int, double)> sig;
    Slot<char(int, double)> slot_1 = [](int, double) { return 'h'; };
    auto to_track1                 = std::make_shared<int>(7);
    auto to_track2                 = std::make_shared<double>(8.3);
    slot_1.track(to_track1);
    slot_1.track(to_track2);
    Slot<char(int, double)> slot_2 = [](int, double) { return 'r'; };
    Slot<char(int, double)> slot_3 = [](int, double) { return 'g'; };

    sig.connect(slot_2);
    sig.connect(slot_1, Position::at_front);
    sig.connect(5, slot_3, Position::at_back);

    Signal<char(int, double)> sig_move_to = std::move(sig);

    auto result = sig_move_to(5, 4.3);

    CHECK(bool(result));
    CHECK(*result == 'r');

    to_track1.reset();
    // EXPECT_THROW(sig_move_to(4, 2.1), Expired_slot);
}

TEST_CASE("Signal::operator=(Signal&&)", "[signal]")
{
    Signal<char(int, double)> sig;
    Slot<char(int, double)> slot_1 = [](int, double) { return 'h'; };
    auto to_track1                 = std::make_shared<int>(7);
    auto to_track2                 = std::make_shared<double>(8.3);
    slot_1.track(to_track1);
    slot_1.track(to_track2);
    Slot<char(int, double)> slot_2 = [](int, double) { return 'r'; };
    Slot<char(int, double)> slot_3 = [](int, double) { return 'g'; };

    sig.connect(slot_2);
    sig.connect(slot_1, Position::at_front);
    sig.connect(5, slot_3, Position::at_back);

    Signal<char(int, double)> sig_move_to;
    sig_move_to.connect([](int, double) { return 'x'; });
    sig_move_to = std::move(sig);

    auto result = sig_move_to(5, 4.3);

    CHECK(bool(result));
    CHECK(*result == 'r');

    to_track1.reset();
    // EXPECT_THROW(sig_move_to(4, 2.1), Expired_slot);
}

TEST_CASE("Signal::connect with position", "[signal]")
{
    Signal<float()> sig;
    Slot<float()> slot_1 = []() { return 8.3; };
    auto conn1           = sig.connect(slot_1);

    Slot<float()> slot_2 = []() { return 1.2; };
    auto conn2           = sig.connect(slot_2, Position::at_front);

    Slot<float()> slot_3 = []() { return 3.4; };
    auto conn3           = sig.connect(slot_3, Position::at_back);

    auto result1 = sig();
    REQUIRE(bool(result1));
    CHECK(3.4 == Approx(*result1).margin(1e-12));

    conn3.disconnect();

    auto result2 = sig();
    REQUIRE(bool(result2));
    CHECK(8.3 == Approx(*result2).margin(1e-12));

    conn1.disconnect();

    auto result3 = sig();
    REQUIRE(bool(result3));
    CHECK(1.2 == Approx(*result3).margin(1e-12));

    conn2.disconnect();

    auto result4 = sig();
    CHECK_FALSE(bool(result4));
}

TEST_CASE("Signal::connect() with group", "[signal]")
{
    Signal<char(int)> sig;
    Slot<char(int)> slot_1 = [](int) { return 'a'; };
    auto conn1             = sig.connect(1, slot_1);

    Slot<char(int)> slot_2 = [](int) { return 'b'; };
    auto conn2             = sig.connect(1, slot_2, Position::at_front);

    Slot<char(int)> slot_3 = [](int) { return 'c'; };
    auto conn3             = sig.connect(3, slot_3, Position::at_back);

    Slot<char(int)> slot_4 = [](int) { return 'd'; };
    auto conn4             = sig.connect(3, slot_4);

    Slot<char(int)> slot_5 = [](int) { return 'e'; };
    auto conn5             = sig.connect(100, slot_5);

    Slot<char(int)> slot_6 = [](int) { return 'f'; };
    auto conn6             = sig.connect(1, slot_6, Position::at_front);

    auto result1 = sig(1);
    REQUIRE(bool(result1));
    CHECK('e' == *result1);

    conn5.disconnect();

    auto result2 = sig(2);
    REQUIRE(bool(result2));
    CHECK('d' == *result2);

    conn4.disconnect();

    auto result3 = sig(3);
    REQUIRE(bool(result3));
    CHECK('c' == *result3);

    conn3.disconnect();

    auto result4 = sig(4);
    REQUIRE(bool(result4));
    CHECK('a' == *result4);

    conn1.disconnect();

    auto result5 = sig(5);
    REQUIRE(bool(result5));
    CHECK('b' == *result5);

    conn2.disconnect();

    auto result6 = sig(6);
    REQUIRE(bool(result6));
    CHECK('f' == *result6);

    conn6.disconnect();

    auto result7 = sig(7);
    CHECK_FALSE(bool(result7));
}

TEST_CASE("Signal::connect both overloads", "[signal]")
{
    Signal<char(int)> sig;
    Slot<char(int)> slot_1 = [](int) { return 'a'; };
    auto conn1             = sig.connect(1, slot_1);

    Slot<char(int)> slot_2 = [](int) { return 'b'; };
    auto conn2             = sig.connect(1, slot_2, Position::at_front);

    Slot<char(int)> slot_3 = [](int) { return 'c'; };
    auto conn3             = sig.connect(3, slot_3, Position::at_back);

    Slot<char(int)> slot_4 = [](int) { return 'd'; };
    auto conn4             = sig.connect(3, slot_4);

    Slot<char(int)> slot_5 = [](int) { return 'e'; };
    auto conn5             = sig.connect(100, slot_5);

    Slot<char(int)> slot_6 = [](int) { return 'f'; };
    auto conn6             = sig.connect(1, slot_6, Position::at_front);

    Slot<char(int)> slot_7 = [](int) { return 'g'; };
    auto conn7             = sig.connect(slot_7, Position::at_front);

    Slot<char(int)> slot_8 = [](int) { return 'h'; };
    auto conn8             = sig.connect(slot_8, Position::at_front);

    Slot<char(int)> slot_9 = [](int) { return 'i'; };
    auto conn9             = sig.connect(slot_9, Position::at_back);

    auto result0 = sig(0);
    REQUIRE(bool(result0));
    CHECK('i' == *result0);

    conn9.disconnect();

    auto result1 = sig(1);
    REQUIRE(bool(result1));
    CHECK('e' == *result1);

    conn5.disconnect();

    auto result2 = sig(2);
    REQUIRE(bool(result2));
    CHECK('d' == *result2);

    conn4.disconnect();

    auto result3 = sig(3);
    REQUIRE(bool(result3));
    CHECK('c' == *result3);

    conn3.disconnect();

    auto result4 = sig(4);
    REQUIRE(bool(result4));
    CHECK('a' == *result4);

    conn1.disconnect();

    auto result5 = sig(5);
    REQUIRE(bool(result5));
    CHECK('b' == *result5);

    conn2.disconnect();

    auto result6 = sig(6);
    REQUIRE(bool(result6));
    CHECK('f' == *result6);

    conn6.disconnect();

    auto result7 = sig(7);
    REQUIRE(bool(result7));
    CHECK('g' == *result7);

    conn7.disconnect();

    auto result8 = sig(8);
    REQUIRE(bool(result8));
    CHECK('h' == *result8);

    conn8.disconnect();

    auto result9 = sig(9);
    CHECK_FALSE(bool(result9));
}

TEST_CASE("Signal::connect() with tracked object", "[signal]")
{
    Signal<int(char, char)> sig;
    Slot<int(char, char)> slot = [](char, char) { return 3; };
    auto track_me              = std::make_shared<int>(9);
    slot.track(track_me);

    auto conn = sig.connect(slot);

    auto result = sig('l', 'k');
    REQUIRE(bool(result));
    CHECK(3 == *result);

    track_me.reset();
    // EXPECT_THROW(sig('l', 'k'), Expired_slot);  // slot has expired
}

TEST_CASE("Signal::connect with extended position", "[signal]")
{
    Signal<char(int, int)> sig;

    Slot<char(const Connection&, int, int)> slot_1 = [](const Connection&, int,
                                                        int) { return 'a'; };
    auto conn1 = sig.connect_extended(slot_1);

    Slot<char(const Connection&, int, int)> slot_2 = [](const Connection&, int,
                                                        int) { return 'b'; };
    auto conn2 = sig.connect_extended(slot_2, Position::at_front);

    Slot<char(const Connection&, int, int)> slot_3 = [](const Connection&, int,
                                                        int) { return 'c'; };
    auto conn3 = sig.connect_extended(slot_3, Position::at_front);

    Slot<char(const Connection&, int, int)> slot_4 = [](const Connection&, int,
                                                        int) { return 'd'; };
    auto conn4 = sig.connect_extended(slot_4, Position::at_back);

    Slot<char(const Connection&, int, int)> slot_5 = [](const Connection&, int,
                                                        int) { return 'e'; };
    auto conn5 = sig.connect_extended(slot_5);

    auto result1 = sig(5, 4);
    REQUIRE(bool(result1));
    CHECK('e' == *result1);
    conn5.disconnect();

    auto result2 = sig(5, 4);
    REQUIRE(bool(result2));
    CHECK('d' == *result2);
    conn4.disconnect();

    auto result3 = sig(5, 4);
    REQUIRE(bool(result3));
    CHECK('a' == *result3);
    conn1.disconnect();

    auto result4 = sig(5, 4);
    REQUIRE(bool(result4));
    CHECK('b' == *result4);
    conn2.disconnect();

    auto result5 = sig(5, 4);
    REQUIRE(bool(result5));
    CHECK('c' == *result5);
    conn3.disconnect();

    auto result6 = sig(5, 4);
    CHECK_FALSE(bool(result6));
}

TEST_CASE("Signal::connect_extended() with group", "[signal]")
{
    Signal<char(int)> sig;
    Slot<char(const Connection&, int)> slot_1 = [](const Connection&, int) {
        return 'a';
    };
    auto conn1 = sig.connect_extended(1, slot_1);

    Slot<char(const Connection&, int)> slot_2 = [](const Connection&, int) {
        return 'b';
    };
    auto conn2 = sig.connect_extended(1, slot_2, Position::at_front);

    Slot<char(const Connection&, int)> slot_3 = [](const Connection&, int) {
        return 'c';
    };
    auto conn3 = sig.connect_extended(3, slot_3, Position::at_back);

    Slot<char(const Connection&, int)> slot_4 = [](const Connection&, int) {
        return 'd';
    };
    auto conn4 = sig.connect_extended(3, slot_4);

    Slot<char(const Connection&, int)> slot_5 = [](const Connection&, int) {
        return 'e';
    };
    auto conn5 = sig.connect_extended(100, slot_5);

    Slot<char(const Connection&, int)> slot_6 = [](const Connection&, int) {
        return 'f';
    };
    auto conn6 = sig.connect_extended(1, slot_6, Position::at_front);

    auto result1 = sig(1);
    REQUIRE(bool(result1));
    CHECK('e' == *result1);

    conn5.disconnect();

    auto result2 = sig(2);
    REQUIRE(bool(result2));
    CHECK('d' == *result2);

    conn4.disconnect();

    auto result3 = sig(3);
    REQUIRE(bool(result3));
    CHECK('c' == *result3);

    conn3.disconnect();

    auto result4 = sig(4);
    REQUIRE(bool(result4));
    CHECK('a' == *result4);

    conn1.disconnect();

    auto result5 = sig(5);
    REQUIRE(bool(result5));
    CHECK('b' == *result5);

    conn2.disconnect();

    auto result6 = sig(6);
    REQUIRE(bool(result6));
    CHECK('f' == *result6);

    conn6.disconnect();

    auto result7 = sig(7);
    CHECK_FALSE(bool(result7));
}

TEST_CASE("Signal::connect_extended with both overloads", "[signal]")
{
    Signal<char(int)> sig;
    Slot<char(const Connection&, int)> slot_1 = [](const Connection&, int) {
        return 'a';
    };
    auto conn1 = sig.connect_extended(1, slot_1);

    Slot<char(const Connection&, int)> slot_2 = [](const Connection&, int) {
        return 'b';
    };
    auto conn2 = sig.connect_extended(1, slot_2, Position::at_front);

    Slot<char(const Connection&, int)> slot_3 = [](const Connection&, int) {
        return 'c';
    };
    auto conn3 = sig.connect_extended(3, slot_3, Position::at_back);

    Slot<char(const Connection&, int)> slot_4 = [](const Connection&, int) {
        return 'd';
    };
    auto conn4 = sig.connect_extended(3, slot_4);

    Slot<char(const Connection&, int)> slot_5 = [](const Connection&, int) {
        return 'e';
    };
    auto conn5 = sig.connect_extended(100, slot_5);

    Slot<char(const Connection&, int)> slot_6 = [](const Connection&, int) {
        return 'f';
    };
    auto conn6 = sig.connect_extended(1, slot_6, Position::at_front);

    Slot<char(const Connection&, int)> slot_7 = [](const Connection&, int) {
        return 'g';
    };
    auto conn7 = sig.connect_extended(slot_7, Position::at_front);

    Slot<char(const Connection&, int)> slot_8 = [](const Connection&, int) {
        return 'h';
    };
    auto conn8 = sig.connect_extended(slot_8, Position::at_front);

    Slot<char(const Connection&, int)> slot_9 = [](const Connection&, int) {
        return 'i';
    };
    auto conn9 = sig.connect_extended(slot_9, Position::at_back);

    auto result0 = sig(0);
    REQUIRE(bool(result0));
    CHECK('i' == *result0);

    conn9.disconnect();

    auto result1 = sig(1);
    REQUIRE(bool(result1));
    CHECK('e' == *result1);

    conn5.disconnect();

    auto result2 = sig(2);
    REQUIRE(bool(result2));
    CHECK('d' == *result2);

    conn4.disconnect();

    auto result3 = sig(3);
    REQUIRE(bool(result3));
    CHECK('c' == *result3);

    conn3.disconnect();

    auto result4 = sig(4);
    REQUIRE(bool(result4));
    CHECK('a' == *result4);

    conn1.disconnect();

    auto result5 = sig(5);
    REQUIRE(bool(result5));
    CHECK('b' == *result5);

    conn2.disconnect();

    auto result6 = sig(6);
    REQUIRE(bool(result6));
    CHECK('f' == *result6);

    conn6.disconnect();

    auto result7 = sig(7);
    REQUIRE(bool(result7));
    CHECK('g' == *result7);

    conn7.disconnect();

    auto result8 = sig(8);
    REQUIRE(bool(result8));
    CHECK('h' == *result8);

    conn8.disconnect();

    auto result9 = sig(9);
    CHECK_FALSE(bool(result9));
}

TEST_CASE("Signal::connect_extended slot with tracked object", "[signal]")
{
    Signal<int(char, char)> sig;
    Slot<int(const Connection&, char, char)> ext_slot =
        [](const Connection&, char, char) { return 5; };
    auto track_me = std::make_shared<int>(2);
    ext_slot.track(track_me);

    auto conn = sig.connect_extended(ext_slot);

    CHECK(conn.connected());
    auto result = sig('h', 'k');
    REQUIRE(bool(result));
    CHECK(5 == *result);

    track_me.reset();
    CHECK(0 == track_me.use_count());

    CHECK_THROWS_AS(ext_slot.call(conn, 'j', 'o'), Expired_slot);
    // EXPECT_THROW(sig('l', 'k'), Expired_slot);
}

TEST_CASE("Signal::disconnted by group", "[signal]")
{
    Signal<char(int)> sig;

    Slot<char(int)> slot_1 = [](int) { return 'a'; };
    sig.connect(1, slot_1, Position::at_front);

    Slot<char(int)> slot_2 = [](int) { return 'b'; };
    sig.connect(2, slot_2);

    Slot<char(int)> slot_3 = [](int) { return 'c'; };
    sig.connect(2, slot_3, Position::at_front);

    Slot<char(int)> slot_4 = [](int) { return 'd'; };
    sig.connect(3, slot_4);

    auto result1 = sig(4);
    REQUIRE(bool(result1));
    CHECK('d' == *result1);

    sig.disconnect(3);

    auto result2 = sig(4);
    REQUIRE(bool(result2));
    CHECK('b' == *result2);

    sig.disconnect(2);

    auto result3 = sig(4);
    REQUIRE(bool(result3));
    CHECK('a' == *result3);
}

TEST_CASE("Signal::disconnect_all_slots", "[signal]")
{
    Signal<char(char, int)> sig;

    Slot<char(char, int)> slot_1 = [](char, int) { return 'a'; };
    Slot<char(const Connection&, char, int)> slot_2 =
        [](const Connection&, char, int) { return 'b'; };
    Slot<char(char, int)> slot_3 = [](char, int) { return 'c'; };

    sig.connect(slot_1);
    sig.connect_extended(4, slot_2);
    sig.connect(slot_3, Position::at_front);

    auto result1 = sig('g', 6);
    REQUIRE(bool(result1));

    sig.disconnect_all_slots();

    auto result2 = sig('d', 3);
    CHECK_FALSE(bool(result2));
}

TEST_CASE("Signal::empty", "[signal]")
{
    Signal<int(char)> sig;

    Slot<int(char)> slot_1 = [](char) { return 1; };
    sig.connect(slot_1);

    CHECK_FALSE(sig.empty());

    sig.disconnect_all_slots();
    CHECK(sig.empty());

    Slot<int(const Connection&, char)> slot_2 = [](const Connection&, char) {
        return 2;
    };
    sig.connect_extended(slot_2);

    CHECK_FALSE(sig.empty());

    sig.disconnect_all_slots();

    CHECK(sig.empty());
}

TEST_CASE("Signal::num_slots()", "[signal]")
{
    Signal<int()> sig;

    Slot<int()> slot_1 = []() { return 1; };
    sig.connect(slot_1);
    Slot<int()> slot_2 = []() { return 2; };
    sig.connect(slot_2);
    Slot<int()> slot_3 = []() { return 3; };
    sig.connect(2, slot_3);
    Slot<int()> slot_4 = []() { return 4; };
    sig.connect(2, slot_4);

    CHECK(4 == sig.num_slots());

    Slot<int()> slot_5 = []() { return 5; };
    sig.connect(3, slot_5, Position::at_back);
    Slot<int(const Connection&)> slot_6 = [](const Connection&) { return 6; };
    sig.connect_extended(slot_6);
    Slot<int()> slot_7 = []() { return 7; };
    sig.connect(slot_7);

    CHECK(7 == sig.num_slots());

    sig.disconnect(2);

    CHECK(5 == sig.num_slots());
}

TEST_CASE("Signal::operator()", "[signal]")
{
    Signal<void(char)> sig1;
    sig1.connect([](char) { return; });

    sig1('d');  // returns void

    Signal<int(double)> sig2;
    sig2.connect([](double) { return 4; });
    CHECK(bool(sig2(4.3)));
    CHECK(4 == *sig2(4.5));
}

TEST_CASE("Signal::operator() const", "[signal]")
{
    Signal<void(char)> sig1;
    sig1.connect([](char) { return; });

    const Signal<void(char)> sig1_const = std::move(sig1);

    sig1_const('d');  // returns void

    Signal<int(double)> sig2;
    sig2.connect([](double) { return 4; });
    const Signal<int(double)> sig2_const = std::move(sig2);

    CHECK(bool(sig2_const(4.3)));
    CHECK(4 == *sig2_const(4.5));
}

template <typename return_type>
class New_combiner {
   public:
    New_combiner() = default;
    explicit New_combiner(int val) : value_{val} {}
    using Result_type = return_type;
    template <typename InputIterator>
    Result_type operator()(InputIterator first, InputIterator last)
    {
        return Result_type();
    }
    int get_value() { return value_; }

   private:
    int value_ = 0;
};

TEST_CASE("Signal::combiner()", "[signal]")
{
    Signal<void(int)> sig1;
    CHECK(typeid(Optional_last_value<void>{}) == typeid(sig1.combiner()));

    Signal<int(double), Optional_last_value<int>> sig2;
    CHECK(typeid(Optional_last_value<int>{}) == typeid(sig2.combiner()));

    Signal<int(double), New_combiner<int>> sig3;
    CHECK(typeid(New_combiner<int>{}) == typeid(sig3.combiner()));
}

TEST_CASE("Signal::set_combiner()", "[signal]")
{
    Signal<int(double, char)> sig1;
    sig1.set_combiner(Optional_last_value<int>{});
    CHECK(typeid(Optional_last_value<int>{}) == typeid(sig1.combiner()));

    Signal<int(double, char), New_combiner<int>> sig2;
    sig2.set_combiner(New_combiner<int>{});
    CHECK(typeid(New_combiner<int>{}) == typeid(sig2.combiner()));

    Signal<int(double, char), New_combiner<int>> sig3;
    sig3.set_combiner(New_combiner<int>{7});
    CHECK(7 == sig3.combiner().get_value());
    sig3.set_combiner(New_combiner<int>{3});
    CHECK(3 == sig3.combiner().get_value());
}

TEST_CASE("Signal::swap()", "[signal]")
{
    using std::swap;
    Signal<int(char)> sig1;
    Signal<int(char)> sig2;

    Slot<int(char)> slot_1 = [](char) { return 1; };
    sig1.connect(slot_1);
    sig2.connect(slot_1);
    Slot<int(char)> slot_2 = [](char) { return 2; };
    sig1.connect(slot_2);
    sig2.connect(slot_2);
    Slot<int(char)> slot_3 = [](char) { return 3; };
    sig1.connect(slot_3);
    sig2.connect(slot_3);
    Slot<int(char)> slot_4 = [](char) { return 4; };
    sig2.connect(slot_4);
    Slot<int(char)> slot_5 = [](char) { return 5; };
    sig2.connect(slot_5);
    Slot<int(char)> slot_6 = [](char) { return 6; };
    sig2.connect(slot_6);

    auto result1 = sig1('d');
    auto result2 = sig2('g');

    REQUIRE(bool(result1));
    CHECK(3 == *result1);

    REQUIRE(bool(result2));
    CHECK(6 == *result2);

    swap(sig1, sig2);

    result1 = sig1('d');
    result2 = sig2('g');

    REQUIRE(bool(result1));
    CHECK(6 == *result1);

    REQUIRE(bool(result2));
    CHECK(3 == *result2);

    swap(sig2, sig1);

    result1 = sig1('d');
    result2 = sig2('g');

    REQUIRE(bool(result1));
    CHECK(3 == *result1);

    REQUIRE(bool(result2));
    CHECK(6 == *result2);
}

TEST_CASE("Signal::get_tracker()", "[signal]")
{
    Signal<int(char)> sig;
    Slot<int(char)> slot_1 = [](char) { return 6; };
    sig.connect(slot_1);

    CHECK(2 == sig.get_tracker().use_count());
}
