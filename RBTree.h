#pragma once
#include <cstddef>
#include <string>
#include "ArraySeq.h"
#include <sstream>

template <typename T, class Comparator = std::less<T>>
class RBTree{
private:
    const static bool BLACK_;
    const static bool RED_;
    static const size_t LEFT_;
    static const size_t RIGHT_ ;
private:
    struct Node_{
        const T data;
        bool colour;
        Node_* kids[2];
        Node_* parent;
        Node_* next;
        Node_* prev;
    };
private:
    Node_* root_;
    size_t size_;
    Comparator cmp_;
private:
    bool GetColour(const Node_* node) const;
    Node_* LeftTurn(Node_* node);
    Node_* RightTurn(Node_* node);
    void Connect(Node_* child, Node_* parent) const;
    Node_* FindNode(const T& value) const;
    Node_* Copy(Node_* node) const;
    void Delete(Node_* node);
    void InOrder(std::string& result, char item1, char item2, char item3, Node_* node, char first, char second, char third) const;
private:
    static std::string to_string(const T& data) {
        std::ostringstream oss;
        oss << data;
        return oss.str();
    }
public:
    class Iterator{
        friend class RBTree;
    private:
        Node_* ptr_;
    public:
        Iterator(Node_* node);
        Node_* GetPtr(){return ptr_;}
        const T& operator*() const;
        const T* operator->() const;
        Iterator& operator++();
        Iterator operator++(int);
        Iterator& operator--();
        Iterator operator--(int);
        bool operator==(Iterator other) const;
        bool operator!=(Iterator other) const;
    };
public:
    RBTree(Comparator cmp = Comparator());
    RBTree(const RBTree& other);
    ~RBTree();
    bool operator==(const RBTree& other) const;
    bool operator!=(const RBTree& other) const;
    RBTree& operator=(const RBTree& other);
    void Insert(const T& value);
    size_t Size() const;
    Iterator begin() const;
    Iterator end() const;
    Iterator Find(const T& value) const;
    void Stitch();
    ArraySeq<T> GoForward() const;
    std::string Read(const std::string& template_str) const;
    RBTree<T, Comparator> GetSubTree(const T& key) const;
    size_t Count(Node_* node) const;
    bool IsSubTree(const RBTree<T, Comparator>& other) const;
    bool IsSubTree(Node_* node, Node_* other) const;
    template<class Unary>
    RBTree<T, Comparator> Map(const Unary& unary) const;
    template<class Predicate>
    RBTree<T, Comparator> Where(const Predicate& predicate) const;
    template<class Binary, class U>
    U Reduce(const Binary& binary,U u) const;
};

template <typename T, class Comparator>
const T& RBTree<T, Comparator>::Iterator::operator*()const{
    return ptr_->data;
}

template <typename T, class Comparator>
const bool RBTree<T, Comparator>::BLACK_ = true;

template <typename T, class Comparator>
const bool RBTree<T, Comparator>::RED_ = false;

template <typename T, class Comparator>
const size_t RBTree<T, Comparator>::LEFT_ = 0;

template <typename T, class Comparator>
const size_t RBTree<T, Comparator>::RIGHT_ = 1;

template <typename T, class Comparator>
bool RBTree<T, Comparator>::GetColour(const Node_* node)const{
    if(node == nullptr){
        return BLACK_;
    }
    return node->colour;
}

template <typename T, class Comparator>
void RBTree<T, Comparator>::Connect(Node_* child, Node_* parent) const{
    if(child != nullptr){
        child->parent = parent;
    }
}

template <typename T, class Comparator>
typename RBTree<T, Comparator>::Node_* RBTree<T, Comparator>::LeftTurn(Node_* node){
    Node_* right = node->kids[RIGHT_];
    node->kids[RIGHT_] = right->kids[LEFT_];
    right->kids[LEFT_] = node;
    Connect(node->kids[RIGHT_], node);
    Connect(right->kids[LEFT_], right);
    return right;
}

template <typename T, class Comparator>
typename RBTree<T, Comparator>::Node_* RBTree<T, Comparator>::RightTurn(Node_* node){
    Node_* left = node->kids[LEFT_];
    node->kids[LEFT_] = left->kids[RIGHT_];
    left->kids[RIGHT_] = node;
    Connect(node->kids[LEFT_], node);
    Connect(left->kids[RIGHT_], left);
    return left;
}

template <typename T, class Comparator>
void RBTree<T, Comparator>::Insert(const T& value) {
    if (root_ == nullptr) {
        root_ = new Node_{value, BLACK_, {nullptr, nullptr}, nullptr, nullptr, nullptr};
        size_++;
        return;
    }
    Node_* current = root_;
    Node_* parent = nullptr;
    size_t side;

    while (true) {
        parent = current;
        if (cmp_(current->data, value)) {
            side = RIGHT_;
            current = current->kids[RIGHT_];
        } else if (cmp_(value, current->data)) {
            side = LEFT_;
            current = current->kids[LEFT_];
        } else {
            return;
        }

        if (current == nullptr) {
            break;
        }
    }
    current = new Node_{value, RED_, {nullptr, nullptr}, parent, nullptr, nullptr};
    parent->kids[side] = current;
    size_++;
    while (current->parent != nullptr && current->parent->colour == RED_) {
        Node_* parent = current->parent;
        Node_* grandparent = parent->parent;
        size_t parent_side = (grandparent->kids[LEFT_] == parent) ? LEFT_ : RIGHT_;
        Node_* uncle = grandparent->kids[1 - parent_side];

        if (uncle != nullptr && uncle->colour == RED_) {
            parent->colour = BLACK_;
            uncle->colour = BLACK_;
            grandparent->colour = RED_;
            current = grandparent;
        } else {
            if (current == parent->kids[1 - parent_side]) {
                if (parent_side == LEFT_) {
                    grandparent->kids[LEFT_] = LeftTurn(parent);
                } else {
                    grandparent->kids[RIGHT_] = RightTurn(parent);
                }
                current = parent;
                parent = current->parent;
            }
            Node_* grand_grandparent = grandparent->parent;
            parent->colour = BLACK_;
            grandparent->colour = RED_;
            if(grand_grandparent != nullptr){
                size_t grand_grandparent_side = (grand_grandparent->kids[LEFT_] == grandparent) ? LEFT_ : RIGHT_;
                if (parent_side == LEFT_) {
                    grandparent = RightTurn(grandparent);
                } else {
                    grandparent = LeftTurn(grandparent);
                }
                grandparent->parent = grand_grandparent;
                grand_grandparent->kids[grand_grandparent_side] = grandparent;
            }else{
                if (parent_side == LEFT_) {
                    grandparent = RightTurn(grandparent);
                } else {
                    grandparent = LeftTurn(grandparent);
                }
                grandparent->parent = grand_grandparent;
                root_ = grandparent;
                break;
            }
            break;
        }
    }

    root_->colour = BLACK_;
}

template <typename T, class Comparator>
RBTree<T, Comparator>::RBTree(Comparator cmp) : root_(nullptr), size_(0), cmp_(cmp) {}

template <typename T, class Comparator>
typename RBTree<T, Comparator>::Node_* RBTree<T, Comparator>::FindNode(const T& value) const {
    Node_* current_node = root_;
    while(current_node != nullptr) {
        if(cmp_(value, current_node->data)) {
            current_node = current_node->kids[LEFT_];
        } else if(cmp_(current_node->data, value)) {
            current_node = current_node->kids[RIGHT_];
        } else {
            return current_node;
        }
    }
    return nullptr;
}

template <typename T, class Comparator>
typename RBTree<T, Comparator>::Node_* RBTree<T, Comparator>::Copy(Node_* node) const{
    if(node == nullptr){
        return nullptr;
    }
    Node_* new_node = new Node_{node->data,node->colour, {nullptr, nullptr}, nullptr};
    new_node->kids[LEFT_] = Copy(node->kids[LEFT_]);
    Connect(new_node->kids[LEFT_], new_node);
    new_node->kids[RIGHT_] = Copy(node->kids[RIGHT_]);
    Connect(new_node->kids[RIGHT_], new_node);
    return new_node;
}

template <typename T, class Comparator>
RBTree<T, Comparator>::RBTree(const RBTree& other) : root_(Copy(other.root_)), size_(other.size_), cmp_(other.cmp_) {
    Stitch();
}
template <typename T, class Comparator>
void RBTree<T, Comparator>::Delete(Node_* node){
    if(node == nullptr){
        return;
    }
    Delete(node->kids[LEFT_]);
    Delete(node->kids[RIGHT_]);
    delete node;
}

template <typename T, class Comparator>
RBTree<T, Comparator>::~RBTree(){
    Delete(root_);
}

template <typename T, class Comparator>
RBTree<T, Comparator>& RBTree<T, Comparator>::operator=(const RBTree& other) {
    if (this == &other) {
        return *this;
    }
    Delete(root_);
    root_ = Copy(other.root_);
    size_ = other.size_;
    cmp_ = other.cmp_;
    Stitch();
    return *this;
}

template <typename T, class Comparator>
RBTree<T, Comparator>::Iterator::Iterator(Node_* node): ptr_(node){}

template <typename T, class Comparator>
const T* RBTree<T, Comparator>::Iterator::operator->() const{
    return &(ptr_->data);
}

template <typename T, class Comparator>
size_t RBTree<T, Comparator>::Size() const{
    return size_;
}

template <typename T, class Comparator>
typename RBTree<T, Comparator>::Iterator& RBTree<T, Comparator>::Iterator::operator++(){
    if(ptr_->kids[RIGHT_] != nullptr){
        ptr_ = ptr_->kids[RIGHT_];
        while(ptr_->kids[LEFT_] != nullptr){
            ptr_ = ptr_->kids[LEFT_];
        }
        return *this;
    }
    while(ptr_->parent != nullptr && ptr_ == ptr_->parent->kids[RIGHT_]){
        ptr_ = ptr_->parent;
    }
    ptr_ = ptr_->parent;
    return *this;
}
    

template <typename T, class Comparator>
typename RBTree<T, Comparator>::Iterator RBTree<T, Comparator>::Iterator::operator++(int){
    auto copy = *this;
    ++(*this);
    return copy;
}

template <typename T, class Comparator>
typename RBTree<T, Comparator>::Iterator& RBTree<T, Comparator>::Iterator::operator--(){
    if(ptr_->kids[LEFT_] != nullptr){
        ptr_ = ptr_->kids[LEFT_];
        while(ptr_->kids[RIGHT_] != nullptr){
            ptr_ = ptr_->kids[RIGHT_];
        }
        return *this;
    }
    while(ptr_->parent != nullptr && ptr_ == ptr_->parent->kids[LEFT_]){
        ptr_ = ptr_->parent;
    }
    ptr_ = ptr_->parent;
    return *this;
}
    
template <typename T, class Comparator>
bool RBTree<T, Comparator>::Iterator::operator==(Iterator other) const{
    return ptr_ == other.ptr_;
}

template <typename T, class Comparator>
bool RBTree<T, Comparator>::Iterator::operator!=(Iterator other) const{
    return ptr_ != other.ptr_;
}

template <typename T, class Comparator>
typename RBTree<T, Comparator>::Iterator RBTree<T, Comparator>::begin() const{
    if(root_ == nullptr){
        return Iterator(nullptr);
    }
    Node_* current_node = root_;
    while(current_node->kids[LEFT_] != nullptr){
        current_node = current_node->kids[LEFT_];
    }
    return Iterator(current_node);
}

template <typename T, class Comparator>
typename RBTree<T, Comparator>::Iterator RBTree<T, Comparator>::end() const{
    return Iterator(nullptr);
}

template <typename T, class Comparator>
typename RBTree<T, Comparator>::Iterator RBTree<T, Comparator>::Find(const T& value) const{
    return Iterator(FindNode(value));
}

template <typename T, class Comparator>
bool RBTree<T, Comparator>::operator==(const RBTree& other) const{
    if(size_ != other.size_){
        return false;
    }
    for(const auto& elem : *this){
        if(other.Find(elem) == other.end()){
            return false;
        }
    }
    return true;
}

template <typename T, class Comparator>
bool RBTree<T, Comparator>::operator!=(const RBTree& other) const{
    return !(*this == other);
}

template <typename T, class Comparator>
void RBTree<T, Comparator>::Stitch() {
    auto iter = begin();
    if (iter == end()) return;
    auto prev = iter;
    ++iter;
    while (iter != end()) {
        prev.ptr_->next = iter.ptr_;
        iter.ptr_->prev = prev.ptr_;
        prev = iter;
        ++iter;
    }
}

template <typename T, class Comparator>
ArraySeq<T> RBTree<T, Comparator>::GoForward()const{
    auto iter = begin();
    ArraySeq<T> data;
    while(iter != end()){
        data.push_back(*iter);
        ++iter;
    }
    return data;
}

template<typename T, class Comparator>
void RBTree<T, Comparator>::InOrder(std::string& result, char item1, char item2, char item3, Node_* node, char first, char second, char third) const{
    if(node == nullptr) {
        return;
    }
    char close;
    switch(item1) {
        case '{': close = '}'; break;
        case '(': close = ')'; break;
        case '[': close = ']'; break;
        case '<': close = '>'; break;
        default:
            throw std::invalid_argument("Invalid opening character '" + std::string(1, item1) + "'");
    }
    result += item1;
    if(first == 'L'){
        InOrder(result, item1, item2, item3, node->kids[LEFT_], first, second, third);
    }else if(first == 'C'){
        result += to_string(node->data);
    }else if(first == 'R'){
        InOrder(result, item1, item2, item3, node->kids[RIGHT_], first, second, third);
    }
    if(second == 'L'){
        InOrder(result, item1, item2, item3, node->kids[LEFT_], first, second, third);
    }else if(second == 'C'){
        result += to_string(node->data);
    }else if(second == 'R'){
        InOrder(result, item1, item2, item3, node->kids[RIGHT_], first, second, third);
    }
    if(third == 'L'){
        InOrder(result, item1, item2, item3, node->kids[LEFT_], first, second, third);
    }else if(third == 'C'){
        result += to_string(node->data);
    }else if(third == 'R'){
        InOrder(result, item1, item2, item3, node->kids[RIGHT_], first, second, third);
    }
    result += close;
    if(item2 == '{'){
        close = '}';
    }else if(item2 == '('){
        close = ')';
    }else if(item2 == '['){
        close = ']';
    }else if(item2 == '<'){
        close = '>';
    }
    result += item2;
    result += std::to_string(node->data);
    result += close;
    if(item3 == '{'){
        close = '}';
    }else if(item3 == '('){
        close = ')';
    }else if(item3 == '['){
        close = ']';
    }else if(item3 == '<'){
        close = '>';
    }
    result += item3;
    InOrder(result, item1, item2, item3, node->kids[RIGHT_], first, second, third);
    result += close;
}

template<typename T, class Comparator>
std::string RBTree<T, Comparator>::Read(const std::string& template_str) const{
    char item1 = template_str[0];
    char item2 = template_str[4];
    char item3 = template_str[8];
    char first = template_str[1];
    char second = template_str[3];
    char third = template_str[7];
    std::string str;
    InOrder(str, item1, item2, item3, root_, first, second, third);
    return str;
}

template<typename T, class Comparator>
size_t RBTree<T, Comparator>::Count(Node_* node) const{
    if(node == nullptr){
        return 0;
    }
    return Count(node->kids[LEFT_]) + Count(node->kids[RIGHT_]) + 1;
}

template<typename T, class Comparator>
RBTree<T, Comparator> RBTree<T, Comparator>::GetSubTree(const T& key) const{
    Node_* sub_root = Find(key).ptr_;
    Node_* node = Copy(sub_root);
    if(GetColour(node) == RED_){
        node->colour = BLACK_;
    }
    RBTree<T, Comparator> sub_tree(cmp_);
    sub_tree.size_ = Count(node);
    sub_tree.root_ = Copy(node);
    return sub_tree;
}

template<typename T, class Comparator>
bool RBTree<T, Comparator>::IsSubTree(const RBTree<T, Comparator>& other) const{
    Node_* mini_root = FindNode(other.root_->data);
    if(mini_root == nullptr){
        return IsSubTree(mini_root, other.root_);
    }
    return IsSubTree(mini_root, other.root_);
}

template<typename T, class Comparator>
bool RBTree<T, Comparator>::IsSubTree(Node_* node, Node_* other) const{
    if(node == nullptr && other == nullptr){
        return true;
    }
    if(other == nullptr){
        return false;
    }
    if(node == nullptr){
        return false;
    }
    if(cmp_(node->data, other->data)){
        return false;
    }
    if(cmp_(other->data, node->data)){
        return false;
    }
    return IsSubTree(node->kids[LEFT_], other->kids[LEFT_]) && IsSubTree(node->kids[RIGHT_], other->kids[RIGHT_]);
}

template<typename T, class Comparator>
template<class Unary>
RBTree<T, Comparator> RBTree<T,Comparator>::Map(const Unary& unary)const{
    RBTree map_RBTree;
    auto iter = begin();
    while(iter != end()){
        map_RBTree.Insert(unary(*iter));
        ++iter;
    }
    return map_RBTree;
}

template<typename T, class Comparator>
template<class Predicate>
RBTree<T, Comparator> RBTree<T,Comparator>::Where(const Predicate& predicate)const{
    RBTree selected_RBTree;
    auto iter = begin();
    while(iter != end()){
        if(predicate(*iter)){
            selected_RBTree.Insert(*iter);
        }
        ++iter;
    }
    return selected_RBTree;
}

template<typename T, class Comparator>
template<class Binary, class U>
U RBTree<T,Comparator>::Reduce(const Binary& binary,U u)const{
    for(const auto& elem: *this){
        u = binary(elem, u);
    }
    return u;
}