#ifndef SJTU_LIST_HPP
#define SJTU_LIST_HPP

#include "exceptions.hpp"
#include "algorithm.hpp"

#include <climits>
#include <cstddef>

namespace sjtu {
/**
 * a data container like std::list
 * allocate random memory addresses for data and they are doubly-linked in a list.
 */
template<typename T>
class list {
protected:
    class node {
    public:
        node *prev;
        node *next;
        T *data; // nullptr for sentinel

        node(): prev(this), next(this), data(nullptr) {}
        explicit node(const T &value): prev(nullptr), next(nullptr) {
            data = new T(value);
        }
        ~node() {
            if (data) {
                delete data;
                data = nullptr;
            }
        }

    };

protected:
    /**
     * add data members for linked list as protected members
     */
    node *head; // sentinel node
    size_t n;

    /**
     * insert node cur before node pos
     * return the inserted node cur
     */
    node *insert(node *pos, node *cur) {
        // assume pos and cur are not null, and cur is detached
        cur->next = pos;
        cur->prev = pos->prev;
        pos->prev->next = cur;
        pos->prev = cur;
        return cur;
    }
    /**
     * remove node pos from list (no need to delete the node)
     * return the removed node pos
     */
    node *erase(node *pos) {
        pos->prev->next = pos->next;
        pos->next->prev = pos->prev;
        pos->prev = pos->next = nullptr;
        return pos;
    }

    // helper for mergesort on linked list without copying/moving elements
    static node *merge_linear(node *a, node *b) {
        node dummy; // sentinel (data nullptr)
        node *tail = &dummy;
        // make sure dummy is in a detached state
        dummy.prev = dummy.next = nullptr;
        while (a && b) {
            if (*(a->data) < *(b->data)) {
                tail->next = a;
                a = a->next;
            } else {
                tail->next = b;
                b = b->next;
            }
            tail = tail->next;
        }
        tail->next = a ? a : b;
        return dummy.next;
    }

    static node *sort_linear(node *start) {
        if (!start || !start->next) return start;
        // split list into halves
        node *slow = start, *fast = start->next;
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
        }
        node *mid = slow->next;
        slow->next = nullptr;
        node *left = sort_linear(start);
        node *right = sort_linear(mid);
        return merge_linear(left, right);
    }

public:
    class const_iterator;
    class iterator {
    private:
        node *ptr;
        const list<T> *owner;

    public:
        iterator(): ptr(nullptr), owner(nullptr) {}
        iterator(node *p, const list<T> *o): ptr(p), owner(o) {}
        friend class list<T>;
        friend class const_iterator;
        /**
         * iter++
         */
        iterator operator++(int) {
            iterator tmp = *this;
            // increment end() is invalid
            if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
            if (ptr == owner->head) throw invalid_iterator();
            ptr = ptr->next;
            return tmp;
        }
        /**
         * ++iter
         */
        iterator & operator++() {
            if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
            if (ptr == owner->head) throw invalid_iterator();
            ptr = ptr->next;
            return *this;
        }
        /**
         * iter--
         */
        iterator operator--(int) {
            iterator tmp = *this;
            if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
            if (ptr == owner->head) {
                // --end(): valid only if not empty
                if (owner->empty()) throw invalid_iterator();
                ptr = owner->head->prev;
            } else {
                if (ptr->prev == owner->head) throw invalid_iterator();
                ptr = ptr->prev;
            }
            return tmp;
        }
        /**
         * --iter
         */
        iterator & operator--() {
            if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
            if (ptr == owner->head) {
                if (owner->empty()) throw invalid_iterator();
                ptr = owner->head->prev;
            } else {
                if (ptr->prev == owner->head) throw invalid_iterator();
                ptr = ptr->prev;
            }
            return *this;
        }
        /**
         * TODO *it
         * remember to throw if iterator is invalid
         */
        T & operator *() const {
            if (owner == nullptr || ptr == nullptr || ptr == owner->head || ptr->data == nullptr)
                throw invalid_iterator();
            return *(ptr->data);
        }
        /**
         * TODO it->field
         * remember to throw if iterator is invalid
         */
        T * operator ->() const {
            if (owner == nullptr || ptr == nullptr || ptr == owner->head || ptr->data == nullptr)
                throw invalid_iterator();
            return ptr->data;
        }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }
        bool operator==(const const_iterator &rhs) const { return rhs == *this; }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }
        bool operator!=(const const_iterator &rhs) const { return rhs != *this; }
    };
    /**
     * TODO
     * has same function as iterator, just for a const object.
     * should be able to construct from an iterator.
     */
    class const_iterator {
    private:
        const node *ptr;
        const list<T> *owner;
    public:
        const_iterator(): ptr(nullptr), owner(nullptr) {}
        const_iterator(const node *p, const list<T> *o): ptr(p), owner(o) {}
        const_iterator(const iterator &it): ptr(it.ptr), owner(it.owner) {}
        friend class list<T>;

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
            if (ptr == owner->head) throw invalid_iterator();
            ptr = ptr->next;
            return tmp;
        }
        const_iterator & operator++() {
            if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
            if (ptr == owner->head) throw invalid_iterator();
            ptr = ptr->next;
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
            if (ptr == owner->head) {
                if (owner->empty()) throw invalid_iterator();
                ptr = owner->head->prev;
            } else {
                if (ptr->prev == owner->head) throw invalid_iterator();
                ptr = ptr->prev;
            }
            return tmp;
        }
        const_iterator & operator--() {
            if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
            if (ptr == owner->head) {
                if (owner->empty()) throw invalid_iterator();
                ptr = owner->head->prev;
            } else {
                if (ptr->prev == owner->head) throw invalid_iterator();
                ptr = ptr->prev;
            }
            return *this;
        }
        const T & operator *() const {
            if (owner == nullptr || ptr == owner->head || ptr == nullptr || ptr->data == nullptr)
                throw invalid_iterator();
            return *(ptr->data);
        }
        const T * operator ->() const {
            if (owner == nullptr || ptr == owner->head || ptr == nullptr || ptr->data == nullptr)
                throw invalid_iterator();
            return ptr->data;
        }
        bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }
        bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }
        bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }
        bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }
    };
    // Define cross-type comparisons inline in class bodies instead of out-of-class
    /**
     * TODO Constructs
     * Atleast two: default constructor, copy constructor
     */
    list() {
        head = new node();
        n = 0;
    }
    list(const list &other) {
        head = new node();
        n = 0;
        for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
            push_back(*it);
        }
    }
    /**
     * TODO Destructor
     */
    virtual ~list() {
        clear();
        delete head;
        head = nullptr;
    }
    /**
     * TODO Assignment operator
     */
    list &operator=(const list &other) {
        if (this == &other) return *this;
        clear();
        for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
            push_back(*it);
        }
        return *this;
    }
    /**
     * access the first / last element
     * throw container_is_empty when the container is empty.
     */
    const T & front() const {
        if (empty()) throw container_is_empty();
        return *(head->next->data);
    }
    const T & back() const {
        if (empty()) throw container_is_empty();
        return *(head->prev->data);
    }
    /**
     * returns an iterator to the beginning.
     */
    iterator begin() { return iterator(head->next, this); }
    const_iterator cbegin() const { return const_iterator(head->next, this); }
    /**
     * returns an iterator to the end.
     */
    iterator end() { return iterator(head, this); }
    const_iterator cend() const { return const_iterator(head, this); }
    /**
     * checks whether the container is empty.
     */
    virtual bool empty() const { return n == 0; }
    /**
     * returns the number of elements
     */
    virtual size_t size() const { return n; }

    /**
     * clears the contents
     */
    virtual void clear() {
        node *p = head->next;
        while (p != head) {
            node *nx = p->next;
            // detach and delete
            p->prev = p->next = nullptr;
            delete p;
            p = nx;
        }
        head->next = head->prev = head;
        n = 0;
    }
    /**
     * insert value before pos (pos may be the end() iterator)
     * return an iterator pointing to the inserted value
     * throw if the iterator is invalid
     */
    virtual iterator insert(iterator pos, const T &value) {
        if (pos.owner != this || pos.ptr == nullptr) throw invalid_iterator();
        node *nd = new node(value);
        insert(pos.ptr, nd);
        ++n;
        return iterator(nd, this);
    }
    /**
     * remove the element at pos (the end() iterator is invalid)
     * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
     * throw if the container is empty, the iterator is invalid
     */
    virtual iterator erase(iterator pos) {
        if (empty()) throw container_is_empty();
        if (pos.owner != this || pos.ptr == nullptr || pos.ptr == head) throw invalid_iterator();
        node *next_node = pos.ptr->next;
        node *removed = erase(pos.ptr);
        delete removed;
        --n;
        return iterator(next_node, this);
    }
    /**
     * adds an element to the end
     */
    void push_back(const T &value) { insert(end(), value); }
    /**
     * removes the last element
     * throw when the container is empty.
     */
    void pop_back() {
        if (empty()) throw container_is_empty();
        iterator it(head->prev, this);
        erase(it);
    }
    /**
     * inserts an element to the beginning.
     */
    void push_front(const T &value) { insert(begin(), value); }
    /**
     * removes the first element.
     * throw when the container is empty.
     */
    void pop_front() {
        if (empty()) throw container_is_empty();
        iterator it(head->next, this);
        erase(it);
    }
    /**
     * sort the values in ascending order with operator< of T
     */
    void sort() {
        if (n <= 1) return;
        // Break circular links to create a linear list using next pointers
        head->prev->next = nullptr;
        node *start = head->next;
        // Perform mergesort on linear list
        node *sorted = sort_linear(start);
        // Rebuild prev pointers and circular links
        head->next = sorted;
        node *p = head;
        while (sorted) {
            sorted->prev = p;
            p = sorted;
            sorted = sorted->next;
        }
        p->next = head;
        head->prev = p;
    }
    /**
     * merge two sorted lists into one (both in ascending order)
     * compare with operator< of T
     * container other becomes empty after the operation
     * for equivalent elements in the two lists, the elements from *this shall always precede the elements from other
     * the order of equivalent elements of *this and other does not change.
     * no elements are copied or moved
     */
    void merge(list &other) {
        if (&other == this) return;
        if (other.n == 0) return;
        if (n == 0) {
            // splice all nodes from other
            head->next = other.head->next;
            head->prev = other.head->prev;
            head->next->prev = head;
            head->prev->next = head;
            n = other.n;
            other.head->next = other.head->prev = other.head;
            other.n = 0;
            return;
        }
        // break both into linear lists
        head->prev->next = nullptr;
        other.head->prev->next = nullptr;
        node *a = head->next;
        node *b = other.head->next;
        node *merged = nullptr;
        node **tail_ref = &merged;
        while (a && b) {
            // ensure stability: choose from 'a' when equal
            if (*(b->data) < *(a->data)) {
                *tail_ref = b;
                b = b->next;
            } else {
                *tail_ref = a;
                a = a->next;
            }
            (*tail_ref)->prev = nullptr; // will be fixed in rebuild
            tail_ref = &((*tail_ref)->next);
        }
        *tail_ref = a ? a : b;
        // rebuild prev and circular links
        node *p = head;
        node *cur = merged;
        while (cur) {
            cur->prev = p;
            p = cur;
            cur = cur->next;
        }
        head->next = merged;
        p->next = head;
        head->prev = p;
        n += other.n;
        other.head->next = other.head->prev = other.head;
        other.n = 0;
    }
    /**
     * reverse the order of the elements
     * no elements are copied or moved
     */
    void reverse() {
        if (n <= 1) return;
        node *p = head;
        do {
            node *tmp = p->next;
            p->next = p->prev;
            p->prev = tmp;
            p = tmp; // original next
        } while (p != head);
    }
    /**
     * remove all consecutive duplicate elements from the container
     * only the first element in each group of equal elements is left
     * use operator== of T to compare the elements.
     */
    void unique() {
        if (n <= 1) return;
        node *p = head->next;
        while (p != head && p->next != head) {
            if (*(p->data) == *(p->next->data)) {
                iterator it(p->next, this);
                erase(it);
            } else {
                p = p->next;
            }
        }
    }
};

}

#endif //SJTU_LIST_HPP
