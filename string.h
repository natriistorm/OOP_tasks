#include <iostream>
#include <cstring>

inline size_t max(size_t first, size_t second) {
    if (first > second) return first;
    else return second;
}

class String{
private:
    char* str;
    size_t size;
    size_t capacity;
    char* temp;
public:
    String(): size(0),  capacity(4) {
        str = new char[capacity];
    }

    String(size_t cur_size, char s = '\0') : size(cur_size),  capacity(4){
        while (capacity < size) {
            capacity <<= 1;
        }
        str = new char[capacity];
        memset(str, s, size);
    }

    String(const char c): size(1),  capacity(4) {
        str = new char[capacity];
        str[0] = c;
    }

    String(const char* c_style_str): capacity(4) {
        size = strlen(c_style_str);
        while (capacity < size) {
            capacity <<= 1;
        }
        str = new char[capacity];
        memcpy(str, c_style_str, size);
    }

    String(const String& copy_string): size(copy_string.size),  capacity(4) {
        while (capacity < size) {
            capacity <<= 1;
        }
        str = new char[capacity];
        memcpy(str, copy_string.str, size);
    }

    ~String() {
        delete[] str;
    }

    String& operator= (const String& equal_string) {
        if (this == &equal_string) return *this;
        size = equal_string.size;
        capacity = equal_string.capacity;
        temp = new char [capacity];
        memcpy(temp, equal_string.str, size);
        std::swap(str, temp);
        delete []temp;
        return *this;
    }

    bool operator== (const String& is_equal_string) const {
        if (size != is_equal_string.size) {
            return false;
        }
        return (!strcmp(str, is_equal_string.str));
    }

    const char& operator[] (size_t ind) const {
        return *(str + ind);
    }

    char& operator[] (size_t ind) {
        return *(str + ind);
    }


    const char& front() const {
        return str[0];
    }
    char& front() {
        return str[0];
    }

    const char& back() const {
        return str[size - 1];
    }
    char& back() {
        return str[size - 1];
    }

    String substr(size_t start_ind, size_t count) const {
        String substring(count);
        memcpy(substring.str, str + start_ind, count);
        return substring;

    }

    size_t find(const String& substring) const {
        for (size_t i = 0; i < size; ++i) {
            if (str[i] == substring[0]) {
                size_t tempid = 1;
                while ((tempid < substring.size) && (i + tempid < size)
                                && (str[i + tempid] == substring[tempid])) {
                    ++tempid;
                }
                if (tempid == substring.size) {
                    return i;
                }
            }
        }
        return size;
    }

    size_t rfind(const String& substring) const {
        size_t finded_ind = size;
        for (size_t i = 0; i < size; ++i) {
            if (str[i] == substring[0]) {
                size_t temp = 1;

                while ((temp < substring.size) && (i + temp < size)
                       && (str[i + temp] == substring[temp])) {
                    ++temp;
                }
                if (temp == substring.size) {
                    finded_ind = i;
                }
            }
        }
        return finded_ind;
    }


    size_t length() const {
        return size;
    }

    bool empty() {
        return size == 0;
    }

    void clear() {
        capacity = 4;
        size = 0;
        delete[] str;
        str = new char[capacity];
    }

    void push_back(const char c) {
        if (size >= capacity) {
            capacity <<= 1;
            temp = new char[capacity];
            memcpy(temp, str, size);
            std::swap(str, temp);
            delete[] temp;
        }
        str[size] = c;
        ++size;
    }
    void pop_back() {
        --size;
    }


    String& operator+= (const String& added_string) {
        size_t temp_size = size + added_string.size;
        if (temp_size <= capacity) {
            memcpy(str + size, added_string.str, added_string.size);
        } else {
            while (capacity < temp_size) {
                capacity <<= 1;
            }
            temp = new char [capacity];
            memcpy(temp, str, size);
            memcpy(temp + size, added_string.str, added_string.size);
            std::swap(str, temp);
            delete[] temp;
        }
        size += added_string.size;
        return *this;
    }

};

String operator+ (const String& str, const String& add) {
    String tempstr(str);
    tempstr += add;
    return tempstr;
}


std::istream& operator>> (std::istream& in, String& s) {
    char input;
    s.clear();
    while (in.get(input) && !isspace(input)) {
        s.push_back(input);
    }
    return in;
}

std::ostream& operator<< (std::ostream& out, const String& s) {
    size_t len = s.length();
    for (size_t i = 0; i < len; ++i) {
        out << (s[i]);
    }
    return out;
}

