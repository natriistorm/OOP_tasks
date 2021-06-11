#include <vector>
#include <iterator>
#include <cassert>
#include <iostream>


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
        while (!current) {
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
            top = create_new_pool();
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

        Node(T&& curVal, Node* curNext, Node* curPrev): value(std::move(curVal)), next(curNext), prev(curPrev){}

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

    List() : List(Allocator()){}

    explicit List(const Allocator& alloc) {
        commonAllocator = alloc;
        ptrOnHead  = allocator->allocate(1);
        ptrOnHead->prev = ptrOnHead;
        ptrOnHead->next = ptrOnHead;
        sizeOfList = 0;
    }

    List(List<T, Allocator>&& anotherList) noexcept {
        ptrOnHead = std::move(anotherList.ptrOnHead);
        sizeOfList = std::move(anotherList.sizeOfList);
        allocator = std::move(anotherList.allocator);
        anotherList.ptrOnHead = nullptr;
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


    List<T, Allocator>& operator=(List<T, Allocator>&& anotherList)  noexcept {
        if (allocTraits::propagate_on_container_move_assignment::value) {
            allocator = anotherList.allocator;
            commonAllocator = anotherList.commonAllocator;
        }
        ptrOnHead = std::move(anotherList.ptrOnHead);
        ptrOnHead->prev = ptrOnHead->next = ptrOnHead;
        Node* nextPtr = ptrOnHead;
        Node* prevPtr = ptrOnHead;
        Node* tempPtr = ptrOnHead->next;
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
        return *this;
    }
    ~List() {}

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
        --sizeOfList;
    }

    void pop_back() {
        assert(sizeOfList != 0);
        Node* deleted = ptrOnHead->prev;
        Node* tempNode = ptrOnHead->prev->prev;
        ptrOnHead->prev->prev->next = ptrOnHead;
        ptrOnHead->prev = tempNode;
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
        if(sizeOfList == 0)
            return;
        Node* tempNode = iterator.getNode()->prev;
        iterator.getNode()->prev->next = iterator.getNode()->next;
        iterator.getNode()->next->prev = tempNode;
        --sizeOfList;
    }
};

template <typename Key, typename Value, class Hash = std::hash<Key>, class Equal = std::equal_to<Key>,
    class constAlloc =  std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
public:
    using Alloc = typename constAlloc::template rebind<std::pair<Key,Value>>::other;
    using NodeType = std::pair<Key, Value>;
    using Chain = List<NodeType, Alloc>;
    using key_type = Key;
    Hash hash;
    Equal equal;
    //////////////////////ITERATOR////////////////////////////////
    template<bool Const>
    class MyIterator;
    using iterator = MyIterator<false>;
    using const_iterator = MyIterator<true>;

    template<bool Const>
    class MyIterator {
    private:
        std::vector<Chain>* ptrOnInterfaceVector;
        size_t positionX;
        size_t positionY;
    public:
        using reference = typename std::conditional_t<Const, const NodeType&, NodeType&>;
        using pointer = typename std::conditional_t<Const, const NodeType*, NodeType*>;
        typedef reference value_type;
        typedef std::forward_iterator_tag iterator_category;
        typedef size_t difference_type;
        typedef typename std::conditional_t<Const, const_iterator, iterator> self_type;
        MyIterator() {
            positionX = positionY = 0;
        }

        MyIterator<Const>(const MyIterator<false>& copy) {
            ptrOnInterfaceVector = copy.getPtrOnVector();
            positionX = copy.getPositionX();
            positionY = copy.getPositionY();
        }

        MyIterator(const MyIterator<true>& copy) {
            ptrOnInterfaceVector = copy.getPtrOnVector();
            positionX = copy.getPositionX();
            positionY = copy.getPositionY();
        }

        MyIterator(MyIterator<false>&& copy) {
            ptrOnInterfaceVector = std::move(copy.getPtrOnVector());
            positionX = std::move(copy.getPositionX());
            positionY = std::move(copy.getPositionY());
            copy.getPtrNull();
        }

        MyIterator& operator= (const MyIterator<Const>& copy) {
            ptrOnInterfaceVector = copy.getPtrOnVector();
            positionX = copy.getPositionX();
            positionY = copy.getPositionY();
            return *this;
        }

        MyIterator& operator= (MyIterator<Const>&& copy) {
            ptrOnInterfaceVector = std::move(copy.getPtrOnVector());
            positionX = std::move(copy.getPositionX());
            positionY = std::move(copy.getPositionY());
            copy.getPtrNull();
            return *this;
        }

        MyIterator& operator++() {
            if (positionX == (*ptrOnInterfaceVector)[positionY].size() - 1
                                                                or (*ptrOnInterfaceVector)[positionY].size() == 0) {
                do {
                    ++positionY;
                } while (positionY < (*ptrOnInterfaceVector).size() && (*ptrOnInterfaceVector)[positionY].size() == 0);
                positionX = 0;
            } else {
                ++positionX;
            }
            return *this;
        }
        MyIterator& operator--() {
            if (positionX != 0) {
                --positionX;
            } else {
                do {
                    --positionY;
                } while (positionY < (*ptrOnInterfaceVector).size() && (*ptrOnInterfaceVector)[positionY].size() == 0);
                positionX = (*ptrOnInterfaceVector)[positionY].size() - 1;
            }
            return *this;
        }

        MyIterator operator++(int) {
            MyIterator<Const> copy(*this);
            ++*this;
            return copy;
        }


        template<bool AnotherConst>
        bool operator==(const MyIterator<AnotherConst>& another) const {
            return positionX == another.getPositionX() && positionY == another.getPositionY();
        }

        template<bool AnotherConst>
        bool operator!=(const MyIterator<AnotherConst>& another) const {
            return !(*this == another);
        }


        template<bool _Const = Const>
        reference operator*() const {
            auto it = (*ptrOnInterfaceVector)[positionY].begin();
            size_t cnt = 0;
            while (cnt != positionX) {
                ++it;
                ++cnt;
            }
            return (*it);
        }

        template< bool _Const = Const >
        reference operator*() {
            auto it = (*ptrOnInterfaceVector)[positionY].begin();
            size_t cnt = 0;
            while (cnt != positionX) {
                ++it;
                ++cnt;
            }
            return (*it);

        }

        template< bool _Const = Const >
        pointer operator->() const {
            auto it = (*ptrOnInterfaceVector)[positionY].begin();
            size_t cnt = 0;
            while (cnt != positionX) {
                ++it;
                ++cnt;
            }
            return (&*it);

        }
        template< bool _Const = Const >
        pointer operator->(){
            auto it = (*ptrOnInterfaceVector)[positionY].begin();
            size_t cnt = 0;
            while (cnt != positionX) {
                ++it;
                ++cnt;
            }
            return (&*it);
        }


        std::vector<Chain>* getPtrOnVector() const{
            return ptrOnInterfaceVector;
        };
        size_t getPositionX() const{
            return positionX;
        };
        size_t getPositionY() const{
            return positionY;
        };

        void getPtrNull() {
            ptrOnInterfaceVector = nullptr;
        }
        void create(int x, int y, std::vector<Chain>* ptrVector) {
            positionX = x;
            positionY = y;
            ptrOnInterfaceVector = ptrVector;
        }

    };
private:
    std::vector<Chain> InterfaceVector;
    size_t sz = 0;
    float maxLoadFactor;
    iterator beginUnorderedMap;
    iterator endUnorderedMap;
    Alloc allocator;
public:
    using allocTraits = std::allocator_traits<Alloc>;

    UnorderedMap() {
        beginUnorderedMap.create(0,0, &InterfaceVector);
        sz = 0;
        maxLoadFactor = 15;
        for(int i = 0; i < 12; ++i) {
            List<NodeType, Alloc> y(allocator);
            InterfaceVector.push_back(std::move(y));
        }
        endUnorderedMap.create(0, InterfaceVector.size(), &InterfaceVector);

    }
    explicit UnorderedMap(size_t size, const Hash& hash = Hash(),
                          const Equal& equal = Equal(), const Alloc& alloc = Alloc())
                                            : InterfaceVector(std::vector<Chain>(size, alloc)), sz(size)  {
            maxLoadFactor = 15;
            beginUnorderedMap(0,0, &InterfaceVector);
            endUnorderedMap.create(0, InterfaceVector.size(), &InterfaceVector);
            allocator = alloc;

    }

    UnorderedMap(size_t bucket_count, const Alloc& alloc) : UnorderedMap(bucket_count, Hash(), Equal(), alloc) {};

    UnorderedMap(const UnorderedMap& another) {
        allocator = another.get_allocator();
        std::vector<List<NodeType, Alloc>> new_vector;
        new_vector.resize(another.InterfaceVector.size());
        InterfaceVector = new_vector;
        for (size_t i = 0; i < InterfaceVector.size(); ++i) {
            InterfaceVector[i] = another.InterfaceVector[i];
        }
        maxLoadFactor = another.max_load_factor();
        beginUnorderedMap = another.beginUnorderedMap;
        endUnorderedMap = another.endUnorderedMap;
        sz = another.sz;
    }

    UnorderedMap(UnorderedMap&& another) {
        allocator = another.get_allocator();
        InterfaceVector = std::move(another.InterfaceVector);
        maxLoadFactor = std::move(another.max_load_factor());
        sz = std::move(another.sz);
        beginUnorderedMap = std::move(another.beginUnorderedMap);
        endUnorderedMap = std::move(another.endUnorderedMap);
    }

    UnorderedMap& operator=(const UnorderedMap& another) {
        InterfaceVector.clear();
        std::vector<Chain> new_vector;
        allocator = another.get_allocator();
        new_vector.reserve(another.size());
        InterfaceVector = new_vector;
        for (size_t i = 0; i < InterfaceVector.size(); ++i) {
            InterfaceVector[i] =another.InterfaceVector[i];
        }
        beginUnorderedMap = another.beginUnorderedMap;
        endUnorderedMap = another.endUnorderedMap;
        maxLoadFactor = another.max_load_factor();
        sz = another.size();
    }

    UnorderedMap& operator=(UnorderedMap&& another) {
        InterfaceVector.clear();
        allocator = another.get_allocator();
        InterfaceVector = std::move(another.InterfaceVector);
        maxLoadFactor = std::move(another.maxLoadFactor);
        sz = std::move(another.sz);
        beginUnorderedMap = std::move(another.beginUnorderedMap);
        endUnorderedMap = std::move(another.endUnorderedMap);
        return *this;
    }

    ~UnorderedMap() {
        sz = 0;
        maxLoadFactor = 0;
    }

    size_t size() {
        return sz;
    }

    iterator begin() {
        if (InterfaceVector[beginUnorderedMap.getPositionY()].size() != 0) {
            return beginUnorderedMap;
        }
        ++beginUnorderedMap;
        return beginUnorderedMap;
    }

    const_iterator begin() const {
        if (InterfaceVector[beginUnorderedMap.getPositionY()].size() != 0) {
            return const_iterator(beginUnorderedMap);
        }
        iterator temp = beginUnorderedMap;
        ++temp;
        return const_iterator(temp);
    }

    const_iterator cbegin() const {
        return const_iterator(begin());
    }

    iterator end() {
        return endUnorderedMap;
    }

    const_iterator end() const {
        return const_iterator(endUnorderedMap);
    }

    const_iterator cend() const {
        return const_iterator(endUnorderedMap);
    }

    void checkBeginUnorderedMap(size_t currentHash) {
        if (currentHash < beginUnorderedMap.getPositionY()) {
            beginUnorderedMap.create(0, currentHash, &InterfaceVector);
        }
    }

    std::pair<iterator, bool> insert(const NodeType& value) {
        bool can_be_inserted = true;
        iterator iteratorOnCurElem;
        size_t currentHash = hash(value.first) % InterfaceVector.size();
        checkBeginUnorderedMap(currentHash);
        iteratorOnCurElem = find(value.first);
        if (iteratorOnCurElem != endUnorderedMap) {
            can_be_inserted = false;
        }
        if (can_be_inserted) {
            iteratorOnCurElem = check_and_add(value);
        }
        return std::make_pair(iteratorOnCurElem, can_be_inserted);
    }

    std::pair<iterator, bool> insert(NodeType&& value) {
        iterator iteratorOnCurElem;
        bool can_be_inserted = true;
        size_t currentHash = hash(value.first) % InterfaceVector.size();
        typename List<NodeType,Alloc>::iterator it = InterfaceVector[currentHash].begin();
        checkBeginUnorderedMap(currentHash);
        int steps = 0;
        for (; it != InterfaceVector[currentHash].end(); ++it, ++steps) {
            if (equal(it->first, value.first)) {
                iteratorOnCurElem.create(steps, currentHash, &InterfaceVector);
                can_be_inserted = false;
            }
        }
        if (can_be_inserted) {
            if (load_factor() > maxLoadFactor) {
                reserve(2 * InterfaceVector.size(), false);
            }
            currentHash = hash(value.first) % InterfaceVector.size();
            iteratorOnCurElem.create(InterfaceVector[currentHash].size(), currentHash, &InterfaceVector);
            InterfaceVector[currentHash].insert(it, std::move(value));
            ++sz;
        }
        return std::make_pair(iteratorOnCurElem, can_be_inserted);
    }

    template<class InputIt>
    void insert(InputIt first, InputIt last) {
        while (first != last) {
            this->insert(*first);
            ++first;
        }
    }

    void erase(const iterator& it) {
        typename List<NodeType,Alloc>::iterator beginListIterator = InterfaceVector[it.getPositionY()].begin();
        while (!equal(beginListIterator->first, it->first)) {
            ++beginListIterator;
        }
        InterfaceVector[it.getPositionY()].erase(beginListIterator);
        --sz;
    }

    template<class InputIt>
    void erase(const InputIt& first, const InputIt& last) {
        iterator end(last);
        --end;
        while (end != first) {
            this->erase(end);
            --end;
        }
        this->erase(first);
        if (last == endUnorderedMap) {
            endUnorderedMap.create(endUnorderedMap.getPositionX(), first.getPositionY(), &InterfaceVector);
        }
    }

    template<typename ... Args>
    std::pair<iterator,bool> emplace(Args&&... args){
        NodeType* node = allocator.allocate(1);
        allocTraits::construct(allocator, node, std::forward<Args>(args)...);
        return this->insert(std::move(*node));

    }

    iterator check_and_add(const NodeType& node){
        if (load_factor() > maxLoadFactor) {
            reserve(2 * InterfaceVector.size(), false);
        }
        size_t currentHash = hash(node.first) % InterfaceVector.size();
        if (currentHash < beginUnorderedMap.getPositionY()) {
            beginUnorderedMap.create(0, currentHash, &InterfaceVector);
        }
        InterfaceVector[currentHash].push_back(node);
        ++sz;
        iterator addedElem;
        addedElem.create(InterfaceVector[currentHash].size() - 1, currentHash, &InterfaceVector);
        return addedElem;
    }

    Value& at (const key_type& key) {
        int currentHash = hash(key) % InterfaceVector.size();
        for (auto& i : InterfaceVector[currentHash])
            if (equal(i.first, key)) {
                return i.second;
            }
        throw std::out_of_range("Out of range");
    }

    const Value& at (const key_type& key) const {
        int currentHash = hash(key) % InterfaceVector.size();
        for (auto& i : InterfaceVector[currentHash])
            if (equal(i.first, key)) {
                return i.second;
            }
        throw std::out_of_range("Out of range");
    }

    Value& operator[](const key_type& key) {
        try{
            return at(key);
        } catch(...) {
            return (*check_and_add(std::move(NodeType{key, Value()}))).second;
        }
    }

    Value& operator[](key_type&& key) {
        try{
            return at(key);
        } catch(...) {
            return (*check_and_add(std::forward<NodeType>(NodeType{key, Value()}))).second;
        }
    }

    iterator find(const key_type& key) {
        int currentHash = hash(key) % InterfaceVector.size();
        typename List<NodeType,Alloc>::iterator iter = InterfaceVector[currentHash].begin();
        int steps = 0;
        for (; iter != InterfaceVector[currentHash].end(); ++iter, ++steps) {
            if (equal(key, iter->first)) {
                iterator it;
                it.create(steps, currentHash, &InterfaceVector);
                return it;
            }
        }
        return this->end();
    }

    const_iterator find(const key_type& key) const {
        int currentHash = hash(key) % InterfaceVector.size();;
        const_iterator iter = InterfaceVector[currentHash].begin();
        for (; iter != InterfaceVector[currentHash].end(); ++iter) {
            if (equal(key, iter->first)) {
                return iter;
            }
        }
        return this->cend();
    }

    float load_factor() const {
        if (InterfaceVector.size() == 0) {
            return 0;
        }
        return sz / InterfaceVector.size();
    }

    float max_load_factor() const {
        return maxLoadFactor;
    }

    void max_load_factor(float ml) {
        maxLoadFactor = ml;
    }

    Alloc get_allocator() const {
        return allocator;
    }

    void rehash(size_t count, bool is_forced) {
        std::vector<Chain> rehashedInteface;
        size_t newSize = 0;
        if (is_forced) {
            newSize = count / maxLoadFactor;
        } else {
            if(load_factor() > maxLoadFactor)
                newSize = count;
        }
        if (newSize < InterfaceVector.size())
            return;
        for(size_t i = 0; i < newSize; ++i) {
            rehashedInteface.push_back(std::move(List<NodeType, Alloc>(allocator)));
        }
        for (iterator i = this->begin(); i != this->end(); ++i) {
            int newHash = hash(i->first) % newSize;
            rehashedInteface[newHash].push_back(std::move(*i));
        }
        InterfaceVector = std::move(rehashedInteface);
        endUnorderedMap.create(0, newSize, &InterfaceVector);
    }

    void reserve(size_t count, bool forced_rehash = true) {
        rehash(count, forced_rehash);
    }


};
