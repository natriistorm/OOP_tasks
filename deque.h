#include <iostream>
#include <vector>
#include <iterator>
#include <string>
const size_t chunkSize = 8;
template<typename T>

class Deque {
public:
    template<bool Const>
    class myiterator;
    using iterator = myiterator<false>;
    using const_iterator = myiterator<true>;
    template<bool Const>
    class myiterator {
        using reference = typename std::conditional_t<Const, const T&, T&>;
        using pointer = typename std::conditional_t<Const, const T*, T*>;
    public:
        typedef T value_type;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T difference_type;
        typedef typename std::conditional_t<Const, const_iterator, iterator> self_type;
        myiterator() = default;

        myiterator<Const>(const myiterator<false>& copy) {
            value = copy.curValue();
            curPos = copy.curPosition();
            ptrOnHead = copy.getPtrOnHead();
            ptrOnTail = copy.getPtrOnTail();
        }

        myiterator(const myiterator<true>& copy) {
            value = copy.curValue();
            curPos = copy.curPosition();
            ptrOnHead = copy.getPtrOnHead();
            ptrOnTail = copy.getPtrOnTail();
        }

        int curValue() const {
            return value;
        }
        int curPosition() const {
            return curPos;
        }
        std::vector<std::vector<T>*>* getPtrOnHead() const {
            return ptrOnHead;
        }
        std::vector<std::vector<T>*>* getPtrOnTail() const {
            return ptrOnTail;
        }

        myiterator& operator= (myiterator<Const>& copy) {
            value = copy.curValue();
            curPos = copy.curPosition();
            ptrOnHead = copy.getPtrOnHead();
            ptrOnTail = copy.getPtrOnTail();
            return *this;
        }


        myiterator& operator++() {
            if (value == chunkSize - 1) {
                ++curPos;
                value = 0;
            } else {
                ++value;
            }
            return *this;
        }

        myiterator& operator--() { //prefix
            if (value == 0) {
                --curPos;
                value = chunkSize - 1;
            } else {
                --value;
            }
            return *this;
        }

        myiterator operator--(int a) {
            myiterator<Const> copy(*this);
            --*this;
            return copy;
        }

        myiterator operator++(int a) {
            myiterator<Const> copy(*this);
            ++*this;
            return copy;
        }

        myiterator& operator+=(int a) {
                curPos += (a + value) / chunkSize;
                value = (value + a) % chunkSize;
            return *this;
        }

        myiterator& operator-=(int a) {
            *this += (-a);
            return *this;
        }

        myiterator operator+(int add) {
            myiterator<Const> copy(*this);
            return copy += add;
        }

        myiterator operator-(int add) {
            myiterator<Const> copy(*this);
            return copy -= add;
        }

        template<bool AnotherConst>
        int operator-(const myiterator<AnotherConst>& another) const {
            int mod = 0;
            mod += (this->curPos - another.curPos) * chunkSize;
            mod += this->value - another.value;
            return mod;
        }

        template<bool AnotherConst>
        bool operator<(const myiterator<AnotherConst>& another) const {
            if (this->curPos != another.curPos) {
                return (this->curPos < another.curPos);
            } else {
                return (this->value < another.value);
            }
        }

        template<bool AnotherConst>
        bool operator>(const myiterator<AnotherConst>& another) const {
            return another < *this;
        }

        template<bool AnotherConst>
        bool operator==(const myiterator<AnotherConst>& another) const {
            if (this->curPos == another.curPos) {
                return (this->value == another.value);
            } else {
                return false;
            }
        }

        template<bool AnotherConst>
        bool operator!=(const myiterator<AnotherConst>& another) const {
            return !(*this == another);
        }

        template<bool AnotherConst>
        bool operator<=(const myiterator<AnotherConst>& another) const {
            return (*this == another) or (*this < another);
        }

        template<bool AnotherConst>
        bool operator>=(const myiterator<AnotherConst>& another) const {
            return (*this == another) or (another < *this);
        }

        template< bool _Const = Const >
        std::enable_if_t<_Const, reference >
        operator*() const{
            int tempCurPos = curPos;
            if (curPos < 0) {
                tempCurPos = -curPos;
                return (*(*ptrOnHead)[tempCurPos])[value];
            }
            return (*(*ptrOnTail)[tempCurPos])[value];
        }

        template< bool _Const = Const >
        std::enable_if_t<!_Const, reference >
        operator*() {
            int tempCurPos = curPos;
            if (curPos < 0) {
                tempCurPos = -curPos;
                return (*(*ptrOnHead)[tempCurPos])[value];
            }
            return (*(*ptrOnTail)[tempCurPos])[value];
        }

        template< bool _Const = Const >
        std::enable_if_t<_Const, pointer >
        operator->() const{
            int tempCurPos = curPos;
            if (curPos < 0) {
                tempCurPos = -curPos;
                return &((*(*ptrOnHead)[tempCurPos])[value]);
            }
            return &((*(*ptrOnTail)[tempCurPos])[value]);
        }

        template< bool _Const = Const >
        std::enable_if_t<!_Const, pointer >
        operator->(){
            int tempCurPos = curPos;
            if (curPos < 0) {
                tempCurPos = -curPos;
                return &((*(*ptrOnHead)[tempCurPos])[value]);
            }
            return &((*(*ptrOnTail)[tempCurPos])[value]);
        }


        void create(int x, int y, std::vector<std::vector<T>*>& ptrTail, std::vector<std::vector<T>*>& ptrHead) {
            value = x;
            curPos = y;
            ptrOnHead = &ptrHead;
            ptrOnTail = &ptrTail;
        }
        void create(const myiterator<Const>& iter, std::vector<std::vector<T>*>& ptrTail, std::vector<std::vector<T>*>& ptrHead) {
            value = iter.value;
            curPos = iter.curPos;
            ptrOnHead = &ptrHead;
            ptrOnTail = &ptrTail;
        }
    private:
        int value = 0;
        int curPos = 0;
        std::vector<std::vector<T>*>* ptrOnTail;
        std::vector<std::vector<T>*>* ptrOnHead;
    };

private:
    size_t sz = 0;
    std::vector<std::vector<T>*> head;
    std::vector<std::vector<T>*> tail;
    myiterator<false> beginDeque;
    myiterator<false> endDeque;
    
    
    void createVectors(const T& elem) {
        for (int i = 0; i < 2; ++i) {
            std::vector<T>* ptrVec = new std::vector<T>(chunkSize, elem);
            std::vector<T>* ptrTail = new std::vector<T>(chunkSize, elem);
            head.push_back(ptrVec);
            tail.push_back(ptrTail);
        }
    }

public:
    Deque():sz(0) {
        beginDeque.create(1, 0, tail, head);
        endDeque.create(0, 0, tail, head);
    }
    Deque(size_t a) : sz(a){
        try {
            int tailSize = a / chunkSize + 1;
            for (int i = 0; i < tailSize; ++i) {
                tail.push_back(new std::vector<T>(chunkSize));
            }
            for (int i = 0; i < 2; ++i) {
                head.push_back(new std::vector<T>(chunkSize));
            }
            beginDeque.create(0, 0, tail, head);
            endDeque.create(a % chunkSize - 1, a / chunkSize, tail, head);
        } catch(...) {
            for (size_t i = 0; i < tail.size(); ++i) {
                delete tail[i];
            }
            for (size_t i = 0; i < head.size(); ++i) {
                delete head[i];
            }
            tail.clear();
            sz = 0;
        }
    };

    Deque(size_t a, const T& value)  {
        try {
            sz = a;
            int tailSize = a / chunkSize + 1;
            for (int i = 0; i < tailSize; ++i) {
                tail.push_back(new std::vector<T>(chunkSize, value));
            }
            for (int i = 0; i < 2; ++i) {
                head.push_back(new std::vector<T>(chunkSize));
            }
            beginDeque.create(0, 0, tail, head);
            endDeque.create(a % chunkSize - 1, tailSize - 1, tail, head);
        } catch(...) {
            for (size_t i = 0; i < tail.size(); ++i) {
                delete tail[i];
            }
            for (size_t i = 0; i < head.size(); ++i) {
                delete head[i];
            }
            head.clear();
            tail.clear();
            sz = 0;
        }
    };

    Deque(const Deque& copied) {
        try {
            sz = copied.sz;
            for (size_t i = 0; i < copied.tail.size(); ++i) {
                tail.push_back(new std::vector<T>(*copied.tail[i]));
            }
            for (size_t i = 0; i < copied.head.size(); ++i) {
                head.push_back(new std::vector<T>(*copied.head[i]));
            }
            beginDeque.create(copied.beginDeque, tail, head);
            endDeque.create(copied.endDeque, tail, head);
        } catch(...) {
            for (size_t i = 0; i < tail.size(); ++i) {
                delete tail[i];
            }
            for (size_t i = 0; i < head.size(); ++i) {
                delete head[i];
            }
            sz = 0;
        }
    }

    Deque& operator=(const Deque& copy) {
        Deque<T> reservedCopy(*this);
        try {
            sz = copy.sz;
            tail.clear();
            head.clear();
            for (size_t i = 0; i < copy.tail.size(); ++i) {
                tail.push_back(new std::vector<T>(*copy.tail[i]));
            }
            for (size_t i = 0; i < head.size(); ++i) {
                head.push_back(new std::vector<T>(*copy.head[i]));
            }
            beginDeque.create(copy.beginDeque, tail, head);
            endDeque.create(copy.endDeque, tail, head);
            return *this;
        } catch(...) {
            for (size_t i = 0; i < tail.size(); ++i) {
                delete head[i];
            }
            head.resize(copy.head.size());
            for (size_t i = 0; i < copy.head.size(); ++i) {
                *(head[i]) = *reservedCopy.head[i];
            }
            for (size_t i = 0; i < tail.size(); ++i) {
                delete tail[i];
            }
            tail.resize(copy.tail.size());
            for (size_t i = 0; i < copy.tail.size(); ++i) {
                *(tail[i]) = *reservedCopy.tail[i];
            }
            sz = reservedCopy.sz;
            beginDeque = reservedCopy.beginDeque;
            endDeque = reservedCopy.endDeque;
            return *this;
        }

    }

    ~Deque() {
    }

    size_t size() const {
        return sz;
    }

    T& operator[](size_t i) {
        int curIndValue= beginDeque.curValue();
        int curIndPos= beginDeque.curPosition();
        int tempPos = curIndPos + (i + curIndValue) / chunkSize;
        int tempValue = (curIndValue + i) % chunkSize;
        if (tempPos < 0) {
            return (*head[-tempPos])[tempValue];
        }
        return (*tail[tempPos])[tempValue];
    }

    const T& operator[](size_t i) const {
        int curIndValue= beginDeque.curValue();
        int curIndPos= beginDeque.curPosition();
        int tempPos = curIndPos + (i + curIndValue) / chunkSize;
        int tempValue = (curIndValue + i) % chunkSize;
        if (tempPos < 0) {
            return (*head[-tempPos])[tempValue];
        }
        return (*tail[tempPos])[tempValue];
    }

    T& at(size_t i) {
        if (i >= sz) {
            throw std::out_of_range("Out of range");

        }
        return (*this)[i];
    }

    const T& at(size_t i) const {
        if (i >= sz) {
            throw std::out_of_range("Out of range");
        }
        return (*this)[i];
    }

    iterator begin() {
        return beginDeque;
    }

    const_iterator begin() const {
        return const_iterator(beginDeque);
    }

    const_iterator cbegin() const {
        return const_iterator(beginDeque);
    }

    iterator end() {
        myiterator<false> copyOfEnd = endDeque;
        return ++copyOfEnd;
    }

    const_iterator end() const {
        myiterator<true> copyOfEnd = endDeque;
        ++copyOfEnd;
        return copyOfEnd;
    }

    const_iterator cend() const {
        myiterator<false> copyOfEnd = endDeque;
        return ++copyOfEnd;
    }

    void push_back(const T& element){
        if (sz == 0) {
            createVectors(element);
        }
        int tailSize = tail.size();
        if (endDeque.curValue() == chunkSize - 1 && endDeque.curPosition() == tailSize - 1) {
            std::vector<T>* newChunk = new std::vector<T>(chunkSize, *beginDeque);
            tail.push_back(newChunk);
        }
        ++endDeque;
        *endDeque = element;
        ++sz;
    }

    void pop_back() {
        if (sz == 0) {
            return;
        }
        --endDeque;
        --sz;
    }

    void push_front(const T& element) {
        if (sz == 0) {
            createVectors(element);
        }
        int headSize = head.size();
        if (beginDeque.curValue() == 0 && -beginDeque.curPosition() == headSize - 1) {
            std::vector<T>* newChunk = new std::vector<T>(chunkSize, *beginDeque);
            head.push_back(newChunk);
        }
        if (-beginDeque.curPosition() == 0 and beginDeque.curValue() == 0) {
            beginDeque.create(chunkSize - 1, -1, tail, head);
        } else {
            --beginDeque;
        }
        *beginDeque = element;
        ++sz;
    }

    void pop_front() {
        if (sz == 0) {
            return;
        }
        ++beginDeque;
        --sz;
    }

    template<bool AnotherConst>
    void insert(const myiterator<AnotherConst>& ind, const T& elem) {
        int pos = ind - beginDeque;
        push_back(*endDeque);
        for (int i = sz; i > pos; --i) {
            (*this)[i] = (*this)[i - 1];
        }
        (*this)[pos] = elem;
    }

    template<bool AnotherConst>
    void erase(const myiterator<AnotherConst>& ind) {
        int pos = ind - beginDeque;
        for (size_t i = pos; i < sz - 1; ++i) {
            (*this)[i] = (*this)[i + 1];
        }
        pop_back();
    }

    using const_reverse_iterator = std::reverse_iterator<T>;

    const_reverse_iterator crbegin() {
        return const_reverse_myiterator(cend());
    }

    const_reverse_iterator crend() {
        return const_reverse_myiterator(cbegin());
    }

    using reverse_myiterator = std::reverse_iterator<T>;

    reverse_myiterator rend() {
        return reverse_myiterator(begin());
    }

    reverse_myiterator rbegin() {
        return reverse_myiterator(end());
    }
};


