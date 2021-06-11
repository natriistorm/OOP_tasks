#include <iostream>
#include <vector>
#include <iterator>
#include <cassert>
template<size_t ChunkSize>
class FixedAllocator{
private:
    struct Alloc_Node{
        char weightChunk[ChunkSize]; //for right work of operator sizeof
        Alloc_Node* next = nullptr;
    };
    int bufferSize = 1;
    int maxBufferSize = 1024;
    void* create_new_pool(){
        if (bufferSize < maxBufferSize) {
            bufferSize *= 2;
        }

        Alloc_Node* ptr = new Alloc_Node[bufferSize];

        for (int i = 0; i < bufferSize - 1; ++i) {
            ptr[i].next = &ptr[i + 1];
        }
        ptr[bufferSize - 1].next = top;
        top = &ptr[0];
        return ptr;
    }
    FixedAllocator(){
        create_new_pool();
    }
    ~FixedAllocator(){
        Alloc_Node* current = top;
        while (!top) {
            Alloc_Node* copy = current->next;
            delete current;
            current = copy;
        }
    }

public:
    Alloc_Node* top = nullptr;
    static FixedAllocator& instance () {
        static FixedAllocator instance;
        return instance;
    }
    void* allocate(){
        if (!top) {
            create_new_pool();
        }
        Alloc_Node* copy = top;
        top = top->next;
        return reinterpret_cast<void*>(copy);
    }
    void deallocate(void* ptr) {
        Alloc_Node* copy = reinterpret_cast<Alloc_Node*>(ptr);
        copy->next = top;
        top = copy;
    }
};

template<typename T>
class FastAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template<typename U>
    struct rebind {
        using other = FastAllocator<U>;
    };
    std::vector<char*> allocatedMemory;
    FastAllocator() {}

    FastAllocator(const FastAllocator<T>& another_alloc) {
        int cnt = 0;
        while (another_alloc.allocatedMemory.size() > this->allocatedMemory.size()) {
            this->allocatedMemory.push_back(another_alloc.allocatedMemory[cnt]);
            ++cnt;
        }
    }

    template<typename U>
    FastAllocator(const FastAllocator<U>& another_alloc) {
        int cnt = 0;
        while (another_alloc.allocatedMemory.size() > this->allocatedMemory.size()) {
            this->allocatedMemory.push_back(another_alloc.allocatedMemory[cnt]);
            ++cnt;
        }
    }

    ~FastAllocator() {}

    pointer allocate(size_t n = 1){
        pointer ptr;
        if (n == 1) {
            ptr = reinterpret_cast<pointer>(FixedAllocator<sizeof(T)>::instance().allocate());
        } else if (n < 4) {
            ptr = reinterpret_cast<pointer>(FixedAllocator<sizeof(T) * 4>::instance().allocate());
        } else if (n < 16) {
            ptr = reinterpret_cast<pointer>(FixedAllocator<sizeof(T) * 16>::instance().allocate());
        } else if (n < 64) {
            ptr = reinterpret_cast<pointer>(FixedAllocator<sizeof(T) * 64>::instance().allocate());
        } else {
            char* new_ptr = new char[n * sizeof(T)];
            allocatedMemory.push_back(new_ptr);
            ptr = reinterpret_cast<pointer>(new_ptr);
        }
        return ptr;
    }

    void deallocate(pointer block, size_t n = 1){
        void* new_block = reinterpret_cast<void*>(block);
        if (n == 1) {
            FixedAllocator<sizeof(T)>::instance().deallocate(new_block);
        } else if (n < 4) {
            FixedAllocator<sizeof(T) * 4>::instance().deallocate(new_block);
        } else if (n < 16) {
            FixedAllocator<sizeof(T) * 16>::instance().deallocate(new_block);
        } else if (n < 64) {
            FixedAllocator<sizeof(T) * 64>::instance().deallocate(new_block);
        } else {
            for (auto& i : allocatedMemory) {
                if (reinterpret_cast<void*>(i) == block) {
                    std::swap(i, allocatedMemory[allocatedMemory.size() - 1]);
                }
            }
            allocatedMemory.pop_back();
            delete[] block;
        }
    }
};

template <typename T, class Allocator = std::allocator<T>>
class List{
private:
    class Node{
    public:
        T value;
        Node* next = nullptr;
        Node* prev = nullptr;

        Node() {}

        Node(const T& curVal, Node* curNext, Node* curPrev) : value(curVal), next(curNext), prev(curPrev){}

        Node(T&& curVal, Node* curNext, Node* curPrev) : value(std::move(curVal)), next(curNext), prev(curPrev){}

        Node(const Node* anotherNode)  {
            value = anotherNode->value;
            next = anotherNode->next;
            prev = anotherNode->prev;
        }

        Node(Node&& anotherNode) : value(std::move(anotherNode->value)),
            next(std::move(anotherNode->next)),
            prev(std::move(anotherNode->prev)) {
            anotherNode->next = nullptr;
            anotherNode->prev = nullptr;
        }

        bool operator==(const Node* eq) const {
            if (value == eq->value && next == eq->next && prev == eq->prev) {
                return true;
            }
            return false;
        }

        bool operator!=(const Node* eq) const {
            return *this == eq;
        }
    };
    Node* ptrOnHead = nullptr;
    size_t sizeOfList = 0;
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using move_reference = T&&;

    using myAllocator = typename Allocator::template rebind<Node>::other;
    using allocTraits = std::allocator_traits<myAllocator>;
    myAllocator* allocator = new myAllocator;
    Allocator commonAllocator;
    private:
    void destroyList(){
        Node* deletedNode = ptrOnHead;
        Node* tempNode = deletedNode;
        for (size_t i = 0; i < sizeOfList; ++i) {
            tempNode = deletedNode;
            allocTraits::destroy(*allocator, deletedNode);
            allocator->deallocate(deletedNode, 1);
            deletedNode = tempNode->next;
        }
    }

public:
    List(size_t count, const T& value, const Allocator& alloc = Allocator()) {
        commonAllocator = alloc;
        Node* ptr = allocator->allocate(1);
        allocTraits::construct(*allocator, ptr);
        ptrOnHead = ptr;
        Node* nextPtr = ptrOnHead;
        Node* prevPtr = ptrOnHead;
        for (size_t i = 1; i <= count; ++i) {
            Node* allocatedMemory = allocator->allocate(1);
            allocTraits::construct(*allocator, allocatedMemory, value, nextPtr, prevPtr);
            prevPtr->next = allocatedMemory;
            nextPtr->prev = allocatedMemory;
            prevPtr = allocatedMemory;
        }
        sizeOfList = count;
    }

    List(size_t count) {
        commonAllocator = Allocator();
        ptrOnHead = allocator->allocate(1);
        ptrOnHead->prev = ptrOnHead->next = ptrOnHead;
        Node* nextPtr = ptrOnHead;
        Node* prevPtr = ptrOnHead;
        for (size_t i = 0; i < count; ++i) {
            Node* allocatedMemory = allocator->allocate(1);
            allocTraits::construct(*allocator, allocatedMemory, ptrOnHead->value, nextPtr, prevPtr);
            prevPtr->next = allocatedMemory;
            nextPtr->prev = allocatedMemory;
            prevPtr = allocatedMemory;
        }
        sizeOfList = count;
    }

    explicit List(const Allocator& alloc = Allocator()) {
        commonAllocator = alloc;
        ptrOnHead  = allocator->allocate(1);
        ptrOnHead->prev = ptrOnHead;
        ptrOnHead->next = ptrOnHead;
    }

    List<T, Allocator>(const List<T, Allocator>& anotherList) {
        *allocator = allocTraits::select_on_container_copy_construction(*anotherList.allocator);
        commonAllocator = std::allocator_traits<Allocator>::select_on_container_copy_construction(anotherList.commonAllocator);
        ptrOnHead = allocator->allocate(1);
        ptrOnHead->prev = ptrOnHead->next = ptrOnHead;
        Node* nextPtr = ptrOnHead;
        Node* prevPtr = ptrOnHead;
        Node* tempPtr = anotherList.getPtrOnHead()->next;
        for (size_t i = 1; i <= anotherList.size(); ++i) {
            Node* allocatedMemory = allocator->allocate(1);
            allocTraits::construct(*allocator, allocatedMemory, tempPtr->value, nextPtr, prevPtr);
            prevPtr->next = allocatedMemory;
            nextPtr->prev = allocatedMemory;
            prevPtr = allocatedMemory;
            tempPtr = tempPtr->next;
        }
        sizeOfList = anotherList.size();
    }

    List<T, Allocator>& operator=(const List<T, Allocator>& anotherList) {
        if (this == &anotherList) {
            return *this;
        }
        if (allocTraits::propagate_on_container_copy_assignment::value) {
            allocator = anotherList.allocator;
            commonAllocator = anotherList.commonAllocator;
        }
        destroyList();
        ptrOnHead = allocator->allocate(1);
        ptrOnHead->prev = ptrOnHead->next = ptrOnHead;
        Node* nextPtr = ptrOnHead;
        Node* prevPtr = ptrOnHead;
        Node* tempPtr = anotherList.getPtrOnHead()->next;
        for (size_t i = 1; i <= anotherList.size(); ++i) {
            Node* allocatedMemory = allocator->allocate(1);
            allocTraits::construct(*allocator, allocatedMemory, tempPtr->value, nextPtr, prevPtr);
            prevPtr->next = allocatedMemory;
            nextPtr->prev = allocatedMemory;
            prevPtr = allocatedMemory;
            tempPtr = tempPtr->next;
        }
        sizeOfList = anotherList.size();
        return *this;
    }

    List(List&& anotherList) {
        *allocator = allocTraits::select_on_container_move_construction(*anotherList.allocator);
        commonAllocator = std::allocator_traits<Allocator>::select_on_container_move_construction(anotherList.commonAllocator);
        ptrOnHead = std::move(anotherList.ptrOnHead);
        ptrOnHead->prev = ptrOnHead->next = ptrOnHead;
        Node* nextPtr = ptrOnHead;
        Node* prevPtr = ptrOnHead;
        Node* tempPtr = ptrOnHead()->next;
        for (size_t i = 1; i <= anotherList.size(); ++i) {
            Node* allocatedMemory = allocator->allocate(1);
            allocTraits::construct(*allocator, allocatedMemory, std::move(tempPtr->value), nextPtr, prevPtr);
            prevPtr->next = allocatedMemory;
            nextPtr->prev = allocatedMemory;
            prevPtr = allocatedMemory;
            tempPtr = tempPtr->next;
        }
        sizeOfList = anotherList.size();
        anotherList.size() = 0;
    }

    List<T, Allocator>& operator=(List<T, Allocator>&& anotherList) {
        if (allocTraits::propagate_on_container_move_assignment::value) {
            allocator = anotherList.allocator;
            commonAllocator = anotherList.commonAllocator;
        }
        destroyList();
        ptrOnHead = std::move(anotherList.ptrOnHead);
        ptrOnHead->prev = ptrOnHead->next = ptrOnHead;
        Node* nextPtr = ptrOnHead;
        Node* prevPtr = ptrOnHead;
        Node* tempPtr = ptrOnHead()->next;
        for (size_t i = 1; i <= anotherList.size(); ++i) {
            Node* allocatedMemory = allocator->allocate(1);
            allocTraits::construct(*allocator, allocatedMemory, std::move(tempPtr->value), nextPtr, prevPtr);
            prevPtr->next = allocatedMemory;
            nextPtr->prev = allocatedMemory;
            prevPtr = allocatedMemory;
            tempPtr = tempPtr->next;
        }
        anotherList.ptrOnHead = nullptr;
        sizeOfList = anotherList.size();
        anotherList.size() = 0;
        return *this;
    }
    ~List() {
        destroyList();
    }

    Allocator get_allocator() {
        return commonAllocator;
    }

    Node* getPtrOnHead() const {
        return ptrOnHead;
    }


    size_t size() const {
        return sizeOfList;
    }

    //iterator part
    template<bool Const, bool Reversed>
    class Iterator;
    typedef Iterator<false, false> iterator;
    typedef Iterator<true, false> const_iterator;
    typedef Iterator<false, true> reverse_iterator;
    typedef Iterator<true, true> const_reverse_iterator;

    template<bool Const, bool Reversed>

    class Iterator{
    public:
        using reference = typename std::conditional_t<Const, const T&, T&>;
        using pointer = typename std::conditional_t<Const, const T*, T*>;
        typedef typename std::conditional_t<Const, typename std::conditional_t<Reversed, const_reverse_iterator,
            const_iterator>, typename std::conditional_t< Reversed, reverse_iterator, iterator>> self_type;
        typedef T value_type;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T difference_type;
    private:
        Node* node = nullptr;
    public:

        Iterator() = default;

        Iterator(const Iterator<false, Reversed>& a){
            this->node = a.getNode();
        }

        Iterator& operator=(const Iterator& a){
            assert(a.node != nullptr);
            this->node = a.getNode();
            return *this;
        }

        Iterator(Node* newNode) {
            node = newNode;
        }

        Node* getNode() const{
            return this->node;
        }

        Iterator<Const, Reversed>& operator++() {
            if (Reversed) {
                node = node->prev;
            } else {
                node = node->next;
            }
            return *this;
        }

        Iterator<Const, Reversed>& operator--() { //prefix
            if (Reversed) {
                node = node->next;
            } else {
                node = node->prev;
            }
            return *this;
        }

        Iterator<Const, Reversed> operator--(int) {
            Iterator<false, Reversed> copy(*this);
            --*this;
            return copy;
        }

        Iterator<Const, Reversed> operator++(int) {
            Iterator<false, Reversed> copy(*this);
            ++*this;
            return copy;
        }

        Iterator<false, false> base() {
            Iterator<false, false> iter(node->next);
            return iter;
        }

        template<bool AnotherConst>
        bool operator==(const Iterator<AnotherConst, Reversed>& another) const {
            assert(node != nullptr);
            return this->node == another.node;
        }

        template<bool AnotherConst>
        bool operator!=(const Iterator<AnotherConst, Reversed>& another) const {
            assert(node != nullptr);
            return this->node != another.node;
        }

        template< bool _Const = Const>
        reference operator*() const{
            assert(node != nullptr);
            return this->node->value;
        }

        template< bool _Const = Const>
        reference operator*() {
            assert(node != nullptr);
            return this->node->value;
        }

        template< bool _Const = Const>
        pointer operator->() const{
            assert(node != nullptr);
            return &this->node->value;
        }

        template< bool _Const = Const>
        pointer operator->(){
            assert(node != nullptr);
            return &this->node->value;
        }

        int operator-(const Iterator<Const, Reversed>& anotherIter) const {
            assert(anotherIter.node != nullptr);
            auto t = anotherIter;
            int difference = 0;
            if (*this < anotherIter) {
                while(t != *this) {
                    --t;
                    ++difference;
                }
            } else {
                while(t != *this) {
                    ++t;
                    ++difference;
                }
            }
            return difference;
        }
    };

    iterator begin() {
        Iterator<false, false> copyIter(ptrOnHead->next);
        return copyIter;
    }

    const_iterator begin() const {
        return const_iterator(ptrOnHead->next);
    }

    const_iterator cbegin() const {
        return const_iterator(ptrOnHead->next);
    }

    iterator end() {
        Iterator<false, false> copyOfEnd(ptrOnHead);
        return copyOfEnd;
    }

    const_iterator end() const {
        Iterator<true, false> copyOfEnd(ptrOnHead);
        return copyOfEnd;
    }

    const_iterator cend() const {
        Iterator<false, false> copyOfEnd(ptrOnHead);
        return copyOfEnd;
    }

    const_reverse_iterator crbegin() const {
        Iterator<true, true> iter(ptrOnHead->next);
        return iter;
    }

    const_reverse_iterator crend() const {
        Iterator<true, true> iter(ptrOnHead);
        return iter;
    }

    reverse_iterator rend(){
        Iterator<false, true> iter(ptrOnHead);
        return iter;
    }

    reverse_iterator rbegin(){
        return Iterator<false, true>(ptrOnHead->prev);
    }

    const_reverse_iterator rend() const {
        Iterator<true, true> iter(ptrOnHead);
        return iter;
    }

    const_reverse_iterator rbegin() const{
        Iterator<true, true> iter(ptrOnHead->prev);
        return iter;
    }
    //end of iterator's part

    void push_back(const_reference val) {
        Node* newNode = allocator->allocate(1);
        allocTraits::construct(*allocator, newNode, val, ptrOnHead, ptrOnHead->prev);
        ptrOnHead->prev->next = newNode;
        ptrOnHead->prev = newNode;
        ++sizeOfList;
    }

    void push_back(move_reference val) {
        Node* newNode = allocator->allocate(1);
        allocTraits::construct(*allocator, newNode, std::move(val), ptrOnHead, ptrOnHead->prev);
        ptrOnHead->prev->next = newNode;
        ptrOnHead->prev = newNode;
        ++sizeOfList;
    }

    void push_front(const_reference val) {
        Node* newNode = allocator->allocate(1);
        allocTraits::construct(*allocator, newNode, val, ptrOnHead->next, ptrOnHead);
        ptrOnHead->next->prev= newNode;
        ptrOnHead->next = newNode;
        ++sizeOfList;
    }

    void push_front(move_reference val) {
        Node* newNode = allocator->allocate(1);
        allocTraits::construct(*allocator, newNode, std::move(val), ptrOnHead->next, ptrOnHead);
        ptrOnHead->next->prev= newNode;
        ptrOnHead->next = newNode;
        ++sizeOfList;
    }

    void pop_front() {
        assert(sizeOfList != 0);
        Node* deleted = ptrOnHead->next;
        Node* tempNode = ptrOnHead->next->next;
        ptrOnHead->next->next->prev = ptrOnHead;
        ptrOnHead->next = tempNode;
        allocTraits::destroy(*allocator, deleted);
        allocator->deallocate(deleted, 1);
        --sizeOfList;
    }

    void pop_back() {
        assert(sizeOfList != 0);
        Node* deleted = ptrOnHead->prev;
        Node* tempNode = ptrOnHead->prev->prev;
        ptrOnHead->prev->prev->next = ptrOnHead;
        ptrOnHead->prev = tempNode;
        allocTraits::destroy(*allocator, deleted);
        allocator->deallocate(deleted, 1);
        --sizeOfList;
    }

    template<bool Const, bool Reversed>
    void insert(const Iterator<Const, Reversed>& iterator, const T& val) {
        Node* newNode = allocator->allocate(1);
        allocTraits::construct(*allocator, newNode, val, iterator.getNode(), iterator.getNode()->prev);
        iterator.getNode()->prev->next= newNode;
        iterator.getNode()->prev = newNode;
        ++sizeOfList;
    }

    template<bool Const, bool Reversed>
    void insert(const Iterator<Const, Reversed>& iterator, T&& val) {
        Node* newNode = allocator->allocate(1);
        allocTraits::construct(*allocator, newNode, std::move(val), iterator.getNode(), iterator.getNode()->prev);
        iterator.getNode()->prev->next= newNode;
        iterator.getNode()->prev = newNode;
        ++sizeOfList;
    }

    template<bool Const, bool Reversed>
    void erase(const Iterator<Const, Reversed>& iterator) {
        assert(sizeOfList != 0);
        Node* tempNode = iterator.getNode()->prev;
        iterator.getNode()->prev->next = iterator.getNode()->next;
        iterator.getNode()->next->prev = tempNode;
        allocTraits::destroy(*allocator, iterator.getNode());
        allocator->deallocate(iterator.getNode(), 1);
        --sizeOfList;
    }
};