#ifndef LINERARISER_H
#define LINERARISER_H
//Included("lineariser");
/*
C3 Linearitsation: https://en.wikipedia.org/wiki/C3_linearization
Naive method for obtaining attribute:
	Look through the attrs for each of the supers for each class (recursive)
However, experimenting with examples shows that there may be contradictions created
*/

#include <iostream>
#include <forward_list>
#include <unordered_map>
#include <algorithm>
#include <utility>

using std::clog;
using ClassT = InternalObject*;

template <typename T> using fwd = std::forward_list<T>;
template <typename T> using fwd_it = typename fwd<T>::iterator;
template <typename T> using fwd_c_it = typename fwd<T>::const_iterator;

enum class ReturnValues { found, not_yet_found, invalid };

template <typename T>
std::ostream& operator <<(std::ostream& stream, fwd<T> linked_list) {
    stream << "{";
    for (fwd_it<T> it = linked_list.begin(); it != linked_list.end(); ++it) {
        if (it != linked_list.begin())
            stream << ", ";
        stream << *it;
    }
    stream << "}";
    return stream;
}

template <typename K, typename V>
std::ostream& operator <<(std::ostream& stream, std::unordered_map<K, V> map) {
    stream << "{";
    for (auto it = map.begin(); it != map.end(); ++it) { //typename std::unordered_map<K, V>::iterator
        if (it != map.begin())
            stream << ", ";
        stream << it->first << ": " << it->second;
    }
    stream << "}";
    return stream;
}

std::unordered_map<ClassT, fwd<ClassT>> memoised;
/* = {
    {'H', {'H', 'O'}},
    {'E', {'E', 'H', 'O'}},
    {'F', {'F', 'H', 'O'}},
    {'G', {'G', 'H', 'O'}},
    {'C', {'C', 'E', 'F', 'G', 'H', 'O'}}
};
*/

//template <typename T>
bool tail_with_element(const fwd<ClassT>& list, ClassT element /* e.g. 'A' */) {
    clog << "Checking if " << list << " contains " << element << "\n";
    if (list.empty()) {
        std::cerr << "How could this happen?\n";
        exit(0);
    }
    if (++list.begin() == list.end()) { // e.g. ['A'], it is a head and tail so the tail contains the element
        if (list.front() == element)
            return true;
        return false; // Element is not the same as what's looked for, e.g. ['B']
    }

    /*
    for (fwd_c_it<ClassT> it = list.begin(); ++it != list.end();) { // pre-increment so first value is ignored
        clog << "\tChecking " << *it << " against " << element << "\n";
        if (*it == element) // Checks each element starting from the second
            return false; // Found element in tail of list
    }
    */

    for (fwd_c_it<ClassT> it = list.begin(); ++it != list.end();) { // for-each loop, looking at each element and, if 
        clog << "\tChecking " << *it << " against " << element << "\n"; 
        //if (el == element)
        if (*it == element)
            return true;
    }
    //clog << "";
    return false; // Did not find element
}

bool all_tails_without_element(const fwd<fwd<ClassT>> list_of_lists, ClassT element) {
    /*
    If all elements in every list's tail does not contain element,
    a viable candidate is found for the next MRO value
    */
    for (const fwd<ClassT>& list : list_of_lists) {
        if (tail_with_element(list, element)) // If the element appears in the list's tail, not viable
            return false;
    }

    return true;
}

/*
bool tails_without_element(const fwd<fwd<ClassT>>& list, ClassT element) {
    for (int)
}
*/

void mutating_remove_if(fwd<fwd<ClassT>>& list_of_lists, ClassT element) {
    clog << "Removing element " << element << " from " << list_of_lists << "\n";
    for (fwd_it<fwd<ClassT>> ll_it = list_of_lists.begin(); ll_it != list_of_lists.end(); ++ll_it) {
        ll_it->remove(element); // Linear search
        /*
        for (fwd_it<ClassT> l_it = ll_it->begin(); l_it != ll_it->end(); ++l_it) {
            //clog << "HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
            if (*l_it == element) {
                ll_it->erase_after(l_it);
                break; // Each element only appears once
            }
        }
        */
    }
}

template <typename T>
void append_to_list(fwd<T>& list, fwd_it<T>& iterator, T element, bool increment_bool = true) {
    clog << "Appending\n";
    /*
    if (list.empty())
        list.push_front(element);
    else {
        list.insert_after(iterator, element);
        //++iterator;
    }
    */
    list.insert_after(iterator, element);
    ++iterator;
}

ReturnValues mutating_merge(fwd<ClassT>& accumulated_mro, fwd_it<ClassT>& accumulated_mro_end_it, fwd<fwd<ClassT>>& to_merge) {
    clog << "\n\n" << accumulated_mro;
    //if (!accumulated_mro.empty())
    //    clog << "Current iter value: " << *accumulated_mro_end_it << "\n";
    ClassT candidate;
    //for (std::pair<ClassT, const fwd<ClassT>> el: heterarchy)
    //std::optional<ClassT> consistent_element;
    bool consistently_one = true;
    ClassT consistent_element = 0;
    size_t length_counter;

    clog << "to_merge: " << to_merge << "\n";
    for (const fwd<ClassT>& list : to_merge) {
        if (consistently_one && ++list.begin() != list.end())
            consistently_one = false;
        if (all_tails_without_element(to_merge, list.front())) {
            // Then this is a viable candidate
            clog << list.front() << " is viable!\n";
            candidate = list.front();
            goto found_viable_class;
        }
    }

    /*
    for (fwd_c_it<fwd<ClassT>> mro_it = to_merge.cbegin(); mro_it != to_merge.cend(); ++mro_it) {
        // *mro_it is each sublist and its head is that needs to be compared to each tail of each sub-list
        clog << "mro_it " << *mro_it << "\n";
        length_counter = 0;
        fwd_c_it<ClassT> it = mro_it->cbegin();
        while (it != mro_it->cend()) {
            // *it is each sub-list (still including the head, which is ignored in tail_without_element)
            clog << "it, mro_it " << *it << ", " << *mro_it << "\n";
            if (tail_without_element(*mro_it, *it)) { // Then this is the first viable candidate and is selected
                candidate = *it;
                goto endloop;
            }

            ++it, ++length_counter;
        }
        clog << "length of " << *mro_it << " is " << length_counter << "\n";
        if (length_counter != 1) {
            consistently_one = false;
        }
        else if (consistently_one) {
            clog << "Still consistent!\n";
            if (consistent_element == 0) { // If not already assigned, assign it to this (should happen for first element only)
                clog << "Char empty, *it=" << mro_it->front() << "\n";
                consistent_element = mro_it->front();
            }
            else { // If it has a character
                clog << "Char: '" << consistent_element << "'\n";
                if (mro_it->front() != consistent_element)
                    consistently_one = false;
            }
        }
        clog << "Here!\n";
    }
    */
    
    /*
    If this code has been reached, then no value has been found
    Either there is an error and a consistent MRO or all lists are a single element
    */

    clog << "Current MRO: " << accumulated_mro << "\n";

    if (consistently_one) {
        clog << "Consistently " << to_merge.front().front() << "\n";
        //accumulated_mro.insert_after(accumulated_mro_end_it, consistent_element);
        append_to_list(accumulated_mro, accumulated_mro_end_it, to_merge.front().front());
        return ReturnValues::found;
    } else {
        clog << "Inconsistent!\n";
        return ReturnValues::invalid;
    }

    found_viable_class:
    clog << "candidate for " << to_merge << " = '" << candidate << "', consistently one element: " << consistently_one << "\n";
    //std::cin.get();
    mutating_remove_if(to_merge, candidate);
    //to_merge.remove(candidate);
    append_to_list(accumulated_mro, accumulated_mro_end_it, candidate);
    clog << "so far " << accumulated_mro << " ";
    clog << to_merge << "\n";

    return ReturnValues::not_yet_found;
}

fwd<ClassT> mro_of(ClassT class_object, std::unordered_map<ClassT, fwd<ClassT>>& heterarchy) {
    fwd<ClassT> output;
    clog << "In mro_of\n";
    std::unordered_map<ClassT, fwd<ClassT>>::iterator cached;
    if (heterarchy.end() != (cached = memoised.find(class_object))) {
        clog << class_object << " cached already: " << cached->second << "\n";
        output = cached->second; // If class MRO cache found, get the value of this.
    } else {
        /*
        First find class which is in at least one head but not in any tails
        Add this head to the front and repeat the merging process with the remaining lists
        If there are no heads which are candidates, then there has been an error and a consistent MRO cannot be found
        */
        clog << "Super classes of " << class_object << ": " << heterarchy[class_object] << "\n";
        if (heterarchy[class_object].empty()) {
            output.push_front('O');
            return output;
        }
        fwd<fwd<ClassT>> to_merge{{}};
        /*
        to_merge needs to contain the mros for each class in heterarchy[class_object]
        Insert an empty linked list and keep track of iterator

        We guarantee at least one superclass

        {'C', {'E', 'F', 'G'}}
        */

        fwd<ClassT> accumulated_mro{}; // The result

        fwd_it<fwd<ClassT>>
            merge_it = to_merge.begin();
        fwd_it<ClassT>
            heterarchy_it = heterarchy[class_object].begin(),
            accumulated_mro_end_it = accumulated_mro.before_begin();
        // None are initially valid since

        *merge_it = mro_of(heterarchy[class_object].front(), heterarchy);
        //clog << to_merge << "\n";
        clog << "--------\n";
        while (++heterarchy_it != heterarchy[class_object].end()) {
            to_merge.insert_after(merge_it, fwd<ClassT>{});
            *++merge_it = mro_of(*heterarchy_it, heterarchy);
        }
        //merge_it->pop_front(); // Removes name of class from presented MRO

        clog << to_merge << "\n";

        //clog << *accumulated_mro_end_it << "\n";

        ReturnValues mro_result = ReturnValues::not_yet_found;

        //mutating_merge(accumulated_mro, accumulated_mro_end_it, to_merge);

        clog << accumulated_mro << " " << to_merge << "\n";

        //mutating_merge(accumulated_mro, accumulated_mro_end_it, to_merge);

        clog << accumulated_mro << " " << to_merge << "\n"; //<< " " << (mro_result == ReturnValues::not_yet_found)

        while ((mro_result = mutating_merge(accumulated_mro, accumulated_mro_end_it, to_merge)) == ReturnValues::not_yet_found);

        if (mro_result == ReturnValues::invalid)
            exit(0);

        accumulated_mro.push_front(class_object);

        memoised[class_object] = accumulated_mro;
        output = std::move(accumulated_mro);
        
    }
}
#endif
