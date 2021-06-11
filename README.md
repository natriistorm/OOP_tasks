# Implementation of various STL structures

Добавлены задачи String, BigInteger + Rational,Deque, List и Unordered_Map

String - аналог STL std::string

Biginteger + Rational - поддержка длинной арифметики и работаа с рациональным числами в представлении m / n, m и n - целочисленные

Deque - аналог std::deque с поддержкой итераторов

List - помимо самого std::list реализованы два вид аллокаторов : FixedAllocator и FastAllocator. Первый позволяет выделять ячейки динамической памяти 
фикисированного размера (pool-allocator), второй, реализованный над Fixed решает, выделять ли новый участок памяти или взять уже выделенный. По сравнению со стандартными аллокаторми std::list получен прирост в скорости работы 


C std::list
Tests log: Results with std::allocator: 290 194 212 210 221  ms, results with FastAllocator: 68 67 68 67 67  ms


С собственным List
Tests log: Results with std::allocator: 182 180 177 180 201  ms, results with FastAllocator: 59 58 58 57 61  ms 

UnorderedMap - аналог std::unordered_map, реализован на основе собственного List, с поддержкой собственных аллокаторов
