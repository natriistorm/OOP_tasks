#include <iostream>
#include <vector>
#include <string>
const int MOD = 10000;

std::string reverse_string(std::string& s) {
    int length = s.length();
    std::string reversed(length, '0');
    for (int i = 0; i < length; ++i) {
        reversed[i] = s[length - 1 - i];
    }
    return reversed;
}


inline size_t min(size_t a, size_t b) {
    if (a < b) return a;
    else return b;
}

class BigInteger;

BigInteger operator + (const BigInteger& to_add, const  BigInteger& from_add);
bool operator< (const BigInteger& b, const BigInteger& bi);
bool operator<= (const  BigInteger& b, const  BigInteger& bi);
bool operator== (const  BigInteger& b, const  BigInteger& bi);
BigInteger operator* (const BigInteger& to_mult, const BigInteger& from_mult);
BigInteger operator- (const BigInteger& to_minus, const BigInteger& from_minus);
BigInteger operator/(const BigInteger& to_div, const BigInteger& from_div);


class BigInteger {
private:
    std::vector<int64_t> number;
    bool is_negative;

    bool abs_greater(const BigInteger& bi) const {
        if (number.size() != bi.number.size()) {
            return number.size() > bi.number.size();
        }
        size_t i = 0;
        while (i < number.size() && bi.number[number.size() - i - 1] == number[number.size() - i - 1]) {
            ++i;
        }
        if (i == number.size()) {
            return false;
        }
        return number[number.size() - i - 1] > bi.number[number.size() - i - 1];
    }




    bool is_a_zero() const {
        return (number.size() == 1 && number[0] == 0);
    }

    BigInteger& delete_leading_zeros() {
        size_t i = 0;
        int how_much_dlt = 0;
        while (i < number.size() && number[number.size() - 1 - i] == 0) {
            ++how_much_dlt;
            ++i;
        }
        if (i == number.size()) {
            number.resize(1, 0);
            is_negative = false;
        }
        else if (i != 0) {
            number.resize(number.size() - how_much_dlt);
        }
        return *this;
    }


public:
    BigInteger() {
        number.push_back(0);
        is_negative = false;
    }

    BigInteger(long long input) : is_negative(false) {
        if (input < 0) {
            is_negative = true;
            input = -1 * input;
        }
        while (input >= MOD) {
            number.push_back(input % MOD);
            input = input / MOD;
        }
        number.push_back(input);
    }

    BigInteger(const std::string& t) : is_negative(false) {
        std::string s = t;
        if (s.length() == 0) {
            number.push_back(0);
        }
        if (s[0] == '-' && s[1] != '0') {
            is_negative = true;
            s = s.substr(1);
        }
        if (s == "-0") {
            is_negative = false;
            s = s.substr(1);
        }
        int64_t len = s.length();
        for (int64_t i = len - 1; i >= 0; i -= 4) {
            int current_sum = 0;
            for (int64_t j = 0; j < 4; ++j) {
                if (i - 3 + j >= 0) {
                    current_sum *= 10;
                    current_sum += s[i - 3 + j] - '0';
                }
            }
            number.push_back(current_sum);
        }
    }

    BigInteger(const BigInteger& bi) {
        is_negative = bi.is_negative;
        number = bi.number;
    }

    ~BigInteger() {

    }

    BigInteger& operator=(const BigInteger& equal_bi) {
        if (this == &equal_bi) return *this;
        is_negative = equal_bi.is_negative;
        number = equal_bi.number;
        return *this;

    }

    explicit operator bool() const {
        if (number.size() == 1 && number[0] == 0) {
            return false;
        }
        return true;
    }


    BigInteger& operator+=(const BigInteger& adding) {
        if (adding.is_negative != is_negative) {
            if (!abs_greater(adding)) {
                is_negative = adding.is_negative;
            }
            *this = abs_difference(adding);
        }
        else {
            size_t temp_size = number.size();
            int current_byte = 0;
            for (size_t i = 0; i < temp_size; ++i) {
                if (adding.number.size() <= i) {
                    current_byte += number[i] + 0;
                }
                else {
                    current_byte += number[i] + adding.number[i];
                }
                number[i] = current_byte;
                current_byte = 0;
            }
            while (adding.number.size() > temp_size) {
                number.push_back(adding.number[temp_size]);
                ++temp_size;
            }


            for (size_t i = 0; i < number.size(); ++i) {
                if (number[i] >= MOD) {
                    if (i + 1 == number.size()) {
                        number.push_back(0);
                    }
                    number[i + 1] += number[i] / MOD;
                    number[i] = number[i] % MOD;
                }
            }
        }
        delete_leading_zeros();

        return *this;
    }


    BigInteger& operator-= (const BigInteger& adding) {
        if (adding.is_negative && !is_negative) {
            BigInteger copy_of_adding(adding);
            copy_of_adding.is_negative = false;
            *this += copy_of_adding;
        }
        else if (!adding.is_negative && is_negative) {
            BigInteger copy_of_adding(adding);
            copy_of_adding.is_negative = true;
            *this += copy_of_adding;
        }
        else {
            is_negative = this->lesser(adding);
            *this = abs_difference(adding);
        }
        delete_leading_zeros();
        return *this;
    }

    BigInteger& operator *=(const BigInteger& mult) {
        is_negative = mult.is_negative ^ is_negative;
        size_t n = number.size();
        size_t m = mult.number.size();
        size_t t = n + m;
        number.resize(n + m, 0);
        for (size_t i = t; i > 0; --i) {
            int a = number[i - 1];
            int b = mult.number[0];
            number[i - 1] = 0;
            for (size_t j = min(i - 1, n - 1) + 1; j > 0; --j) {
                if (i - j < m) {
                    number[i - 1] += number[j - 1] * mult.number[i - j];
                }
            }
            number[i - 1] += a * b;
        }
        for (size_t i = 0; i < number.size(); ++i) {
            if (number[i] >= MOD) {
                int temp = number[i] % MOD;
                int overflow = number[i] / MOD;
                number[i] = temp;
                number[i + 1] += overflow;
            }
        }
        delete_leading_zeros();
        return *this;
    }

    BigInteger operator /=(const  BigInteger& div) {
        BigInteger left = 0;
        BigInteger right = *this;
        right.is_negative = false;
        BigInteger middle;
        while (left < right) {
            middle = division_by_two(left, right) + 1;
            if (!(middle * div).abs_greater(*this)) {
                left = middle;
            }
            else {
                right = middle - 1;
            }
        }
        left.is_negative = is_negative ^ div.is_negative;
        left.delete_leading_zeros();
        *this = left;
        return *this;
    }

    BigInteger& operator %= (const BigInteger& mod) {
        *this = *this - (*this / mod) * mod;
        return *this;
    }

    BigInteger operator-() {
        if (!this->is_a_zero()) {
            BigInteger copy(*this);
            copy.is_negative = !(this->is_negative);
            return copy;
        }
        else return *this;
    }

    BigInteger& operator++() {
        *this += 1;
        return *this;
    }

    BigInteger operator++(int) {
        BigInteger copy = *this;
        ++* this;
        return copy;
    }

    BigInteger& operator--() {
        *this -= 1;
        return *this;
    }

    BigInteger operator--(int) {
        BigInteger copy = *this;
        --* this;
        return copy;
    }

    std::string toString() const {
        std::string bi_str = "";
        std::string bi_small_str = "";
        if (is_negative && !this->is_a_zero()) {
            bi_str.push_back('-');
        }
        int len = 0;
        int sz = number.size();
        for (size_t i = 0; i < number.size(); ++i) {
            len = 0;
            int cur_num = number[sz - 1 - i];
            while (cur_num > 0 || len == 0) {
                bi_small_str.push_back(cur_num % 10 + '0');
                cur_num /= 10;
                ++len;
            }
            if (sz - 1 - i != number.size() - 1 && len < 4) {
                while (len < 4) {
                    bi_small_str.push_back('0');
                    ++len;
                }
            }
            bi_small_str = reverse_string(bi_small_str);
            bi_str += bi_small_str;
            bi_small_str = "";
        }
        return bi_str;
    }

private:
    BigInteger abs_difference(const BigInteger& second_arg) {
        int taken = 0;
        int dif = 0;
        int first_num;
        int second_num;
        size_t temp_size = number.size();
        bool comparsion = !abs_greater(second_arg);
        for (size_t i = 0; i < min(number.size(), second_arg.number.size()); ++i) {
            first_num = number[i];
            second_num = second_arg.number[i];
            if (taken == 1) {
                if (comparsion) {
                    --second_num;
                }
                else {
                    --first_num;
                }
                taken = 0;
            }
            if (comparsion) {
                dif = second_num - first_num;
            }
            else {
                dif = first_num - second_num;
            }
            if (dif < 0) {
                dif = MOD + dif;
                taken = 1;
            }
            number[i] = dif;
        }
        while (second_arg.number.size() > temp_size) {
            if (taken == 1) {
                number.push_back(second_arg.number[temp_size] - 1);
                taken = 0;
            }
            else {
                number.push_back(second_arg.number[temp_size]);
            }
            ++temp_size;
        }
        if (number.size() > second_arg.number.size() && taken == 1) {
            int t = 0;
            while (number[second_arg.number.size() + t] == 0) {
                number[second_arg.number.size() + t] = 9999;
                ++t;
            }
            --number[second_arg.number.size() + t];
        }
        this->delete_leading_zeros();
        return *this;
    }


public:
    bool lesser(const BigInteger& bi) const {
        if (is_negative != bi.is_negative) {
            return is_negative;
        }
        else {
            if (is_negative) { //both negative -10 -8 or -5 -3
                return abs_greater(bi);
            }
            else { //both positive 10 8 or  5 3
                return bi.abs_greater(*this);
            }
        }
    }

    bool eq(const BigInteger& bi) const {
        if (is_negative != bi.is_negative) {
            return false;
        }
        if (number.size() != bi.number.size()) {
            return false;
        }
        for (size_t i = 0; i < number.size(); ++i) {
            if (number[i] != bi.number[i]) {
                return false;
            }
        }
        return true;
    }

    static BigInteger division_by_two(const BigInteger& left, const  BigInteger& right) {
        BigInteger copy(left);
        copy += right;
        long long t = 0;
        for (int i = copy.number.size() - 1; i >= 0; --i) {
            t *= MOD;
            t += copy.number[i];
            copy.number[i] = t / 2;
            t &= 1;
        }
        copy.delete_leading_zeros();
        return copy;

    }


};

bool operator< (const BigInteger& b, const BigInteger& bi) {
    return b.lesser(bi);
}

bool operator< (BigInteger& b, BigInteger& bi) {
    return b.lesser(bi);
}

bool operator> (const BigInteger& b, const  BigInteger& bi) {
    return bi < b;
}

bool operator== (const  BigInteger& b, const  BigInteger& bi) {
    return b.eq(bi);
}

bool operator!= (const  BigInteger& b, const  BigInteger& bi) {
    return !(b == bi);
}

bool operator<= (const  BigInteger& b, const  BigInteger& bi) {
    return !(b > bi);
}

bool operator>= (const  BigInteger& b, const  BigInteger& bi) {
    return !(b < bi);
}

BigInteger operator+ (const BigInteger& to_add, const BigInteger& from_add) {
    BigInteger copy(to_add);
    copy += from_add;
    return copy;
}

BigInteger operator- (const BigInteger& to_minus, const BigInteger& from_minus) {
    BigInteger copy(to_minus);
    copy -= from_minus;
    return copy;
}

BigInteger operator* (const BigInteger& to_mult, const BigInteger& from_mult) {
    BigInteger copy(to_mult);
    copy *= from_mult;
    return copy;
}

BigInteger operator/(const BigInteger& to_div, const BigInteger& from_div) {
    BigInteger copy(to_div);
    copy /= from_div;
    return copy;
}

BigInteger operator%(const BigInteger& to_mod, const BigInteger& from_mod) {
    BigInteger copy(to_mod);
    copy %= from_mod;
    return copy;
}


class Rational {
private:
    BigInteger numerator;
    BigInteger denumerator;

    BigInteger gcd(BigInteger a, BigInteger b) {
        if (a < 0) a = -a;
        if (b < 0) b = -b;
        while (a > 0) {
            b %= a;
            std::swap(a, b);
        }
        return b;
    }

    void balancing() {
        BigInteger gcd_rational = gcd(numerator, denumerator);
        if (gcd_rational != 1) {
            numerator /= gcd_rational;
            denumerator /= gcd_rational;
        }
        if (denumerator < 0) {
            denumerator = -denumerator;
            numerator = -numerator;
        }
    }


public:
    Rational() {
        numerator = 0;
        denumerator = 0;
        balancing();
    }

    Rational(long long a) {
        numerator = a;
        denumerator = 1;
        balancing();
    }

    Rational(const BigInteger& a) {
        numerator = a;
        denumerator = 1;
        balancing();
    }
    Rational(const BigInteger& a, const BigInteger& b) {
        numerator = a;
        denumerator = b;
        balancing();
    }

    Rational(const Rational& a) {
        numerator = a.numerator;
        denumerator = a.denumerator;
        balancing();
    }

    ~Rational() {

    }

    Rational& operator=(const  Rational& a) {
        numerator = a.numerator;
        denumerator = a.denumerator;
        balancing();
        return *this;
    }

    Rational operator-() const {
        Rational a(*this);
        a.numerator = -a.numerator;
        a.balancing();
        return a;
    }

    Rational operator+() const {
        return *this;
    }

    Rational& operator +=(const Rational& b) {
        numerator = numerator * b.denumerator + b.numerator * denumerator;
        denumerator = denumerator * b.denumerator;
        balancing();
        return *this;
    }

    Rational& operator -=(const Rational& b) {
        *this += (-b);
        balancing();
        return *this;
    }

    Rational& operator *=(const  Rational& b) {
        numerator *= b.numerator;
        denumerator *= b.denumerator;
        balancing();
        return *this;
    }

    Rational& operator /=(const  Rational& b) {
        numerator *= b.denumerator;
        denumerator *= b.numerator;
        balancing();
        return *this;
    }

    BigInteger num() const {
        return numerator;
    }

    BigInteger denum() const {
        return denumerator;
    }

    std::string toString() const {
        std::string str = "";
        if (numerator == 0) {
            str = "0";
            return str;
        }
        str += numerator.toString();
        if (denumerator != 1) {
            str += '/';
            str += denumerator.toString();
        }

        return str;
    }

    std::string asDecimal(size_t precision = 0) const {
        std::cerr << numerator.toString() << "/" << "\n";
        std::cerr << denumerator.toString() << "=" << "\n";
        BigInteger numer = numerator;
        ++precision;
        for (size_t i = 0; i < precision; ++i) {
            numer *= 10;
        }

        numer = numer / denumerator;
        --precision;

        bool is_negative_temp = false;
        if (numer < 0) {
            is_negative_temp = true;
            numer = -numer;
        }

        if (numer % 10 >= 5) {
            numer += 10;
        }
        numer /= 10;
        std::string number = numer.toString();
        number = reverse_string(number);

        while (precision > number.size()) {
            number += '0';
        }
        number = reverse_string(number);
        if (precision != 0 && precision <= number.size()) {
            number.insert(number.begin() + number.size() - precision, '.');
        }

        if (number[0] == '.') {
            number = '0' + number;
        }
        if (is_negative_temp && number != "0") {
            number = '-' + number;
        }
        std::cerr << number << "\n";
        std::cerr << "next" << "\n";
        return number;

    }

    explicit operator double() {
        std::string s = asDecimal(15);
        double answer = 0;
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] >= '0' && s[i] <= '9') {
                answer *= 10;
                answer += s[i] - 48;
            }
            else {
                continue;
            }
        }
        if (numerator < 0) {
            answer *= -1;
        }
        answer *= 1e15;
        return answer;
    }
};

bool operator< (const Rational& a, const Rational& b) {
    return a.num() * b.denum() < a.denum() * b.num();
}

bool operator> (const Rational& a, const Rational& b) {
    return b < a;
}

bool operator== (const Rational& a, const Rational& b) {
    return a.num() * b.denum() == a.denum() * b.num();
}

bool operator!= (const Rational& a, const Rational& b) {
    return !(a == b);
}

bool operator>= (const Rational& a, const Rational& b) {
    return !(a < b);
}

bool operator<= (const Rational& a, const Rational& b) {
    return !(a > b);
}


std::istream& operator>> (std::istream& in, BigInteger& input) {
    std::string str;
    in >> str;
    input = BigInteger(str);
    return in;
}

std::ostream& operator<< (std::ostream& out, const BigInteger& output) {
    out << output.toString();
    return out;
}


Rational operator+(const Rational& to_add, const Rational& from_add) {
    Rational  copy(to_add);
    copy += from_add;
    return copy;
}

Rational operator-(const Rational& to_minus, const Rational& from_minus) {
    Rational copy(to_minus);
    copy -= from_minus;
    return copy;
}


Rational operator*(const Rational& to_mult, const Rational& from_mult) {
    Rational  copy(to_mult);
    copy *= from_mult;
    return copy;
}

Rational operator/(const Rational& to_div, const Rational& from_div) {
    Rational  copy(to_div);
    copy /= from_div;
    return copy;
}
