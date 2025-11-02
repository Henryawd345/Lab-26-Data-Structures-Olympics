#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

const int runs = 15; // number of  simulations to average
const int events = 4; // {Read, Sort, Insert, Delete}
const int structs = 3; // {Vector, List, Set}
const int accuIndex = runs; // totals slice at the end of the first dimension

// Named instead of using number for clarity
enum Event {ReadEvent = 0, SortEvent = 1, InsertEvent = 2, DeleteEvent = 3};
enum DS {Vec = 0, List = 1, Set = 2};

// file loader 
vector<string> read_lines(const string &path){
    ifstream in(path);
    vector<string> lines;
    string s;
    lines.reserve(20000);  
    while (getline(in, s)) lines.push_back(s);
    return lines;
}

// one full trial
void oneSimulation(const vector<string>& lines, long long out[events][structs]){
    long long vec_read_us, lst_read_us, set_read_us;
    long long vec_sort_us, lst_sort_us, set_sort_us = 0; // set already ordered
    long long vec_ins_us,  lst_ins_us,  set_ins_us;
    long long vec_del_us,  lst_del_us,  set_del_us;
    //us = microsecond
    //Read section
    {
        auto start = high_resolution_clock::now();
        vector<string> vec; vec.reserve(lines.size());
        for (const auto& x : lines) vec.push_back(x);
        auto end = high_resolution_clock::now();
        vec_read_us = duration_cast<microseconds>(end - start).count();
    }
    { 
        auto start = high_resolution_clock::now();
        list<string> lst;
        for (const auto& x : lines) lst.push_back(x);
        auto end = high_resolution_clock::now();
        lst_read_us = duration_cast<microseconds>(end - start).count();
    }
    { 
        auto start = high_resolution_clock::now();
        set<string> st(lines.begin(), lines.end());
        auto end = high_resolution_clock::now();
        set_read_us = duration_cast<microseconds>(end - start).count();
    }

    //sort section
    { 
        vector<string> vec = lines;
        auto start = high_resolution_clock::now();
        sort(vec.begin(), vec.end());
        auto end = high_resolution_clock::now();
        vec_sort_us = duration_cast<microseconds>(end - start).count();
    }
    {
        list<string> lst(lines.begin(), lines.end());
        auto start = high_resolution_clock::now();
        lst.sort();
        auto end = high_resolution_clock::now();
        lst_sort_us = duration_cast<microseconds>(end - start).count();
    }
    // set are already sorted
    // Insert section
     {
        vector<string> vec = lines;
        auto start = high_resolution_clock::now();
        vec.insert(vec.begin() + static_cast<long long>(vec.size()/2), "TESTCODE");
        auto end = high_resolution_clock::now();
        vec_ins_us = duration_cast<microseconds>(end - start).count();
    }
    {
        list<string> lst(lines.begin(), lines.end());
        auto start = high_resolution_clock::now();
        auto it = lst.begin(); 
        advance(it, static_cast<long long>(lst.size()/2));
        lst.insert(it, "TESTCODE");
        auto end = high_resolution_clock::now();
        lst_ins_us = duration_cast<microseconds>(end - start).count();
    }
    {
        set<string> st(lines.begin(), lines.end());
        auto start = high_resolution_clock::now();
        st.insert("TESTCODE");
        auto end = high_resolution_clock::now();
        set_ins_us = duration_cast<microseconds>(end - start).count();
    }
    // Delete section
     {
        vector<string> vec = lines;
        auto start = high_resolution_clock::now();
        vec.erase(vec.begin() + static_cast<long long>(vec.size()/2));
        auto end = high_resolution_clock::now();
        vec_del_us = duration_cast<microseconds>(end - start).count();
    }
    {
        list<string> lst(lines.begin(), lines.end());
        auto start = high_resolution_clock::now();
        auto it = lst.begin();
        advance(it, static_cast<long long>(lst.size()/2));
        lst.erase(it);
        auto end = high_resolution_clock::now();
        lst_del_us = duration_cast<microseconds>(end - start).count();
    }
    {
        set<string> st(lines.begin(), lines.end());
        auto it = st.begin(); advance(it, static_cast<long long>(st.size()/2));
        auto start = high_resolution_clock::now();
        st.erase(it);
        auto end = high_resolution_clock::now();
        set_del_us = duration_cast<microseconds>(end - start).count();
    }

    // Write this run’s 12 numbers
    out[ReadEvent][Vec]   = vec_read_us;
    out[ReadEvent][List]   = lst_read_us;
    out[ReadEvent][Set]   = set_read_us;

    out[SortEvent][Vec]   = vec_sort_us;
    out[SortEvent][List]   = lst_sort_us;
    out[SortEvent][Set]   = set_sort_us; // 0

    out[InsertEvent][Vec] = vec_ins_us;
    out[InsertEvent][List] = lst_ins_us;
    out[InsertEvent][Set] = set_ins_us;

    out[DeleteEvent][Vec] = vec_del_us;
    out[DeleteEvent][List] = lst_del_us;
    out[DeleteEvent][Set] = set_del_us;

}

int main() {
    vector<string> lines = read_lines("codes.txt");
    if (lines.empty()) {
      cerr << "ERROR: couldn't read codes.txt\n";
      return 1;
    }

    static long long results[runs + 1][events][structs] = {0};

    for (int i = 0; i < runs; i++){
        long long one[events][structs] = {0};
        oneSimulation(lines, one);

        for (int e = 0; e < events; ++e) {
            for (int d = 0; d < structs; ++d) {
                results[i][e][d] += one[e][d];
                results[accuIndex][e][d] += one[e][d];
            }
        }
    }

    long long avg[events][structs] = {0};
    for (int e = 0; e < events; ++e)
        for (int d = 0; d < structs; ++d)
            avg[e][d] = results[accuIndex][e][d] / runs;

    cout << "Number of simulations: " << runs << "\n";

    cout << left << setw(16) << " Operation"
         << setw(16) << "Vector(us)"
         << setw(16) << "List(us)"
         << setw(16) << "Set(us)" << "\n";

    cout << left << setw(16) << "      Read"
         << setw(16) << avg[ReadEvent][Vec]
         << setw(16) << avg[ReadEvent][List]
         << setw(16) << avg[ReadEvent][Set] << "\n";

    cout << left << setw(16) << "      Sort"
         << setw(16) << avg[SortEvent][Vec]
         << setw(16) << avg[SortEvent][List]
         << setw(16) << avg[SortEvent][Set];
    cout << "\n";

    cout << left << setw(16) << "      Insert"
         << setw(16) << avg[InsertEvent][Vec]
         << setw(16) << avg[InsertEvent][List]
         << setw(16) << avg[InsertEvent][Set] << "\n";

    cout << left << setw(16) << "      Delete"
         << setw(16) << avg[DeleteEvent][Vec]
         << setw(16) << avg[DeleteEvent][List]
         << setw(16) << avg[DeleteEvent][Set] << "\n";

    return 0;
}

/* syntax examples:
auto start = high_resolution_clock::now()
auto end = high_resolution_clock::now()
auto duration = duration_cast<milliseconds>(end - start)
duration.count() references elapsed milliseconds
*/

// writing this so I can commit 